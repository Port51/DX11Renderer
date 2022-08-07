#include "pch.h"
#include "ParticleComputePass.h"
#include "RenderConstants.h"
#include "GraphicsDevice.h"
#include "ComputeKernel.h"
#include "ComputeShader.h"
#include "ConstantBuffer.h"
#include "StructuredBuffer.h"
#include "ParticleManager.h"
#include "ArgsBuffer.h"

namespace gfx
{

	ParticleComputePass::ParticleComputePass(const GraphicsDevice & gfx, const ParticleManager& particleManager)
		: RenderPass(RenderPassType::ParticleComputeRenderPass), m_particleManager(particleManager)
	{
		const char* computeShaderPath = "GPUParticles.hlsl";
		m_pSpawnParticlesKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, computeShaderPath, "SpawnParticles"));
		m_pUpdateParticlesKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, computeShaderPath, "UpdateParticles"));
		m_pFrustumCullKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, computeShaderPath, "FrustumCull"));
		m_pSetupInstanceBufferKernel = std::make_unique<ComputeKernel>(ComputeShader::Resolve(gfx, computeShaderPath, "SetupInstanceBuffer"));

		CreateSubPass(ParticleComputeSubpass::SpawnParticlesSubpass);
		CreateSubPass(ParticleComputeSubpass::UpdateParticlesSubpass);
		CreateSubPass(ParticleComputeSubpass::FrustumCullSubpass);
		CreateSubPass(ParticleComputeSubpass::SetupInstanceBuffer);

		SetupRenderPassDependencies(gfx);
	}

	void ParticleComputePass::SetupRenderPassDependencies(const GraphicsDevice & gfx)
	{
		this->ClearBinds()
			.CSSetSRV(RenderSlots::CS_FreeSRV + 0u, m_particleManager.GetParticleSystemBuffer().GetSRV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 0u, m_particleManager.GetParticleBuffer().GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 1u, m_particleManager.GetParticleSystemRuntimeBuffer().GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 2u, m_particleManager.GetParticleSortBuffer().GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 3u, m_particleManager.GetParticleInstanceBuffer().GetUAV())
			.CSSetUAV(RenderSlots::CS_FreeUAV + 4u, m_particleManager.GetArgsBuffer().GetUAV())
			.CSSetCB(RenderSlots::CS_FreeCB + 0u, m_particleManager.GetParticleManagerCB().GetD3DBuffer());
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
			m_pFrustumCullKernel->Dispatch(gfx, m_particleManager.GetMaxParticles(), 1u, 1u);
		}

		// CS - Sort rain particles
		{
			
		}

		// CS - Setup instance buffers w/ sorted particles
		{
			m_pSetupInstanceBufferKernel->Dispatch(gfx, m_particleManager.GetMaxParticles(), 1u, 1u);
		}

		this->UnbindSharedResources(gfx, renderState);
	}

}