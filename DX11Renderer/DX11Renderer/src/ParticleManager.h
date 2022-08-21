#pragma once
#include "CommonHeader.h"
#include "ParticleCommon.h"
#include <vector>

namespace gfx
{
	class GraphicsDevice;
	class ParticleSystem;
	class Camera;
	class ComputeKernel;
	class ParticleComputePass;
	class RenderPass;
	class RenderState;
	class ArgsBuffer;
	class ConstantBuffer;
	class StructuredBuffer;

	class ParticleManager
	{
	public:
		ParticleManager(const GraphicsDevice & gfx);
		~ParticleManager();
	public:
		void ExecuteComputePass(const GraphicsDevice& gfx, const Camera& camera, RenderState& renderState) const;
		void ExecuteRenderPass(const GraphicsDevice& gfx, const Camera& camera, RenderState& renderState) const;
		const StructuredBuffer& GetParticleBuffer() const;
		const StructuredBuffer& GetParticleInstanceBuffer() const;
		const StructuredBuffer& GetParticleSortBuffer() const;
		const StructuredBuffer& GetParticleSystemBuffer() const;
		const StructuredBuffer& GetParticleSystemRuntimeBuffer() const;
		const ArgsBuffer& GetArgsBuffer() const;
		const ConstantBuffer& GetParticleManagerCB() const;
		const size_t GetMaxParticles() const;
		const size_t GetParticleSystemCount() const;
	protected:
		static bool SortByDistance(const std::pair<size_t, float>& a, const std::pair<size_t, float>& b);
	protected:

		size_t m_maxParticles;

		std::unique_ptr<ParticleComputePass> m_pParticleComputePass;
		std::unique_ptr<RenderPass> m_pParticleRenderPass;

		std::shared_ptr<ArgsBuffer> m_pArgsBuffer;
		ID3D11Buffer* m_pArgsBufferPtr;
		std::shared_ptr<StructuredBuffer> m_pParticleBuffer;
		std::shared_ptr<StructuredBuffer> m_pParticleInstanceBuffer;
		std::shared_ptr<StructuredBuffer> m_pParticleSortBuffer;
		std::shared_ptr<StructuredBuffer> m_pParticleSystemBuffer;
		std::shared_ptr<StructuredBuffer> m_pParticleSystemRuntimeBuffer;
		std::shared_ptr<ConstantBuffer> m_pParticleManagerCB;
		std::vector<std::unique_ptr<ParticleSystem>> m_pParticleSystems;
		std::unique_ptr<ParticleSystem> m_pLayeredParticleSystem;
	};

}