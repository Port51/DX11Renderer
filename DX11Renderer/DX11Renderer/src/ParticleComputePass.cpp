#include "pch.h"
#include "ParticleComputePass.h"
#include "RenderConstants.h"
#include "GraphicsDevice.h"
#include "ComputeKernel.h"
#include "ComputeShader.h"
#include "ConstantBuffer.h"
#include "StructuredBuffer.h"
#include "ParticleManager.h"

namespace gfx
{

	ParticleComputePass::ParticleComputePass(const GraphicsDevice & gfx, const ParticleManager& particleManager)
		: RenderPass(RenderPassType::ParticleComputeRenderPass), m_particleManager(particleManager)
	{
		const char* computeShaderPath = "Assets\\Shaders\\GPUParticles.hlsl";
		m_pSpawnParticlesKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, computeShaderPath, "SpawnParticles"));
		m_pUpdateParticlesKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, computeShaderPath, "UpdateParticles"));

		CreateSubPass(ParticleComputeSubpass::SpawnParticlesSubpass);
		CreateSubPass(ParticleComputeSubpass::UpdateParticlesSubpass);

		SetupRenderPassDependencies(gfx);
	}

	void ParticleComputePass::SetupRenderPassDependencies(const GraphicsDevice & gfx)
	{
		this->ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, m_particleManager.GetParticleSystemBuffer().GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_particleManager.GetParticleBuffer().GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, m_particleManager.GetParticleSystemRuntimeBuffer().GetUAV());
	}

	void ParticleComputePass::Execute(const GraphicsDevice & gfx, RenderState & renderState) const
	{
		this->BindSharedResources(gfx, renderState);

		// CS - Spawn particles
		{
			m_pSpawnParticlesKernel->Dispatch(gfx, m_particleManager.GetParticleSystemCount(), 1u, 1u);
		}

		// CS - Update particles
		{
			m_pUpdateParticlesKernel->Dispatch(gfx, m_particleManager.GetMaxParticles(), 1u, 1u);
		}

		// CS - Cull particles
		{

		}

		// CS - Sort rain particles
		{

		}

		this->UnbindSharedResources(gfx, renderState);
	}

}