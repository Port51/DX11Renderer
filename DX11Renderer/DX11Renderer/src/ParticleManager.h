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

	template<typename Type>
	class ConstantBuffer;
	template<typename Type>
	class StructuredBuffer;

	class ParticleManager
	{
	public:
		ParticleManager(const GraphicsDevice & gfx);
		~ParticleManager();
	public:
		void ExecuteComputePass(const GraphicsDevice& gfx, const Camera& camera, RenderState& renderState) const;
		void ExecuteRenderPass(const GraphicsDevice& gfx, const Camera& camera, RenderState& renderState) const;
		const StructuredBuffer<Particle>& GetParticleBuffer() const;
		const StructuredBuffer<ParticleInstance>& GetParticleInstanceBuffer() const;
		const StructuredBuffer<ParticleSortData>& GetParticleSortBuffer() const;
		const StructuredBuffer<ParticleSystemSettings>& GetParticleSystemBuffer() const;
		const StructuredBuffer<ParticleSystemRuntime>& GetParticleSystemRuntimeBuffer() const;
		const ArgsBuffer& GetArgsBuffer() const;
		const ConstantBuffer<ParticleManagerCB>& GetParticleManagerCB() const;
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
		std::shared_ptr<StructuredBuffer<Particle>> m_pParticleBuffer;
		std::shared_ptr<StructuredBuffer<ParticleInstance>> m_pParticleInstanceBuffer;
		std::shared_ptr<StructuredBuffer<ParticleSortData>> m_pParticleSortBuffer;
		std::shared_ptr<StructuredBuffer<ParticleSystemSettings>> m_pParticleSystemBuffer;
		std::shared_ptr<StructuredBuffer<ParticleSystemRuntime>> m_pParticleSystemRuntimeBuffer;
		std::shared_ptr<ConstantBuffer<ParticleManagerCB>> m_pParticleManagerCB;
		std::vector<std::unique_ptr<ParticleSystem>> m_pParticleSystems;
		std::unique_ptr<ParticleSystem> m_pLayeredParticleSystem;
	};

}