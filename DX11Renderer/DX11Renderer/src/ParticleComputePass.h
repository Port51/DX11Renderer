#pragma once
#include "RenderPass.h"
#include "CommonHeader.h"
#include "ParticleCommon.h"

namespace gfx
{
	class GraphicsDevice;
	class Bindable;
	class ComputeKernel;
	class RenderState;
	class ParticleManager;
	class ConstantBuffer;
	class StructuredBuffer;

	class ParticleComputePass : public RenderPass
	{
	private:
		enum class ParticleComputeSubpass : u8 {
			SpawnParticlesSubpass,
			UpdateParticlesSubpass,
			FrustumCullSubpass,
			SetupInstanceBuffer,
		};
	public:
		ParticleComputePass(const GraphicsDevice& gfx, const ParticleManager& particleManager);
		void Execute(const GraphicsDevice& gfx, RenderState& renderState) const override;
	private:
		void SetupRenderPassDependencies(const GraphicsDevice& gfx);
	private:
		const ParticleManager& m_particleManager;
		std::unique_ptr<ComputeKernel> m_pSpawnParticlesKernel;
		std::unique_ptr<ComputeKernel> m_pUpdateParticlesKernel;
		std::unique_ptr<ComputeKernel> m_pFrustumCullKernel;
		std::unique_ptr<ComputeKernel> m_pSetupInstanceBufferKernel;
	};
}