#include "pch.h"
#include "ParticleManager.h"
#include "StructuredBuffer.h"
#include "ConstantBuffer.h"
#include "GraphicsDevice.h"
#include "Camera.h"
#include "ParticleSystem.h"
#include "ComputeKernel.h"
#include "ComputeShader.h"
#include "ParticleComputePass.h"
#include "RenderState.h"

namespace gfx
{
	ParticleManager::ParticleManager(const GraphicsDevice & gfx)
	{

		// Create particle systems
		m_pParticleSystems.emplace_back(std::move(std::make_unique<ParticleSystem>(128, dx::XMVectorSet(-1, 0, 0, 1))));
		m_pParticleSystems.emplace_back(std::move(std::make_unique<ParticleSystem>(128, dx::XMVectorSet(1, 0, 0, 1))));
		m_pParticleSystems.emplace_back(std::move(std::make_unique<ParticleSystem>(128, dx::XMVectorSet(0, 0, 0, 1))));
		
		// Determine size of particle buffer
		m_maxParticles = (m_pLayeredParticleSystem != nullptr) ? m_pLayeredParticleSystem->GetMaxParticleCount() : 0u;
		for (int i = 0, ct = m_pParticleSystems.size(); i < ct; ++i)
		{
			m_maxParticles += m_pParticleSystems[i]->GetMaxParticleCount();
		}

		// Raise to next power of 2
		m_maxParticles = 1u << (uint32_t)std::ceil(std::log2(m_maxParticles));

		m_pParticleBuffer = std::make_shared<StructuredBuffer<Particle>>(gfx, D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS, m_maxParticles);
		m_pParticleSystemBuffer = std::make_shared<StructuredBuffer<ParticleSystemSettings>>(gfx, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, GetParticleSystemCount());
		m_pParticleSystemRuntimeBuffer = std::make_shared<StructuredBuffer<ParticleSystemRuntime>>(gfx, D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS, GetParticleSystemCount());

		m_pParticleComputePass = std::make_unique<ParticleComputePass>(gfx, *this);
	}

	ParticleManager::~ParticleManager()
	{
	}

	void ParticleManager::ExecuteCompute(const GraphicsDevice & gfx, const Camera& camera, RenderState& renderState) const
	{
		const auto& context = gfx.GetContext();

		// Store indices of particle systems
		static std::vector<std::pair<size_t, float>> particleSystemsByDistance;
		if (particleSystemsByDistance.size() != m_pParticleSystems.size())
		{
			particleSystemsByDistance.resize(m_pParticleSystems.size());
			for (int i = 0, ct = particleSystemsByDistance.size(); i < ct; ++i)
			{
				particleSystemsByDistance[i] = std::make_pair<size_t, float>(0u, 0.f);
			}
		}

		// CPU - Sort particle systems by distance
		{
			for (int i = 0, ct = m_pParticleSystems.size(); i < ct; ++i)
			{
				particleSystemsByDistance[i].first = i;
				particleSystemsByDistance[i].second = dx::XMVectorGetX(dx::XMVector3LengthSq(dx::XMVectorSubtract(camera.GetPositionWS(), m_pParticleSystems[i]->GetPositionWS())));
			}

			std::sort(particleSystemsByDistance.begin(), particleSystemsByDistance.end(), SortByDistance);
		}

		m_pParticleComputePass->Execute(gfx, renderState);

		// Draw back rain
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-drawinstancedindirect
			// need to set D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS flag
			//context->DrawInstancedIndirect();
		}

		// Draw particle systems in order
		{
			for (int i = 0, ct = particleSystemsByDistance.size(); i < ct; ++i)
			{
				const size_t idx = particleSystemsByDistance[i].first;
				m_pParticleSystems[idx]->DrawIndirect(gfx);
			}
		}

		// Draw front rain
		{
			//context->DrawInstancedIndirect();
		}
	}

	const StructuredBuffer<Particle>& ParticleManager::GetParticleBuffer() const
	{
		return *m_pParticleBuffer;
	}

	const StructuredBuffer<ParticleSystemSettings>& ParticleManager::GetParticleSystemBuffer() const
	{
		return *m_pParticleSystemBuffer;
	}

	const StructuredBuffer<ParticleSystemRuntime>& ParticleManager::GetParticleSystemRuntimeBuffer() const
	{
		return *m_pParticleSystemRuntimeBuffer;
	}

	const size_t ParticleManager::GetMaxParticles() const
	{
		return m_maxParticles;
	}

	const size_t ParticleManager::GetParticleSystemCount() const
	{
		return m_pParticleSystems.size() + (m_pLayeredParticleSystem != nullptr) ? 1u : 0u;
	}

	bool ParticleManager::SortByDistance(const std::pair<size_t, float>& a, const std::pair<size_t, float>& b)
	{
		return a.second < b.second;
	}

}