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
	class RenderState;

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
		void ExecuteCompute(const GraphicsDevice& gfx, const Camera& camera, RenderState& renderState) const;
		const StructuredBuffer<Particle>& GetParticleBuffer() const;
		const StructuredBuffer<ParticleSystemSettings>& GetParticleSystemBuffer() const;
		const StructuredBuffer<ParticleSystemRuntime>& GetParticleSystemRuntimeBuffer() const;
		const size_t GetMaxParticles() const;
		const size_t GetParticleSystemCount() const;
	protected:
		static bool SortByDistance(const std::pair<size_t, float>& a, const std::pair<size_t, float>& b);
	protected:

		size_t m_maxParticles;

		std::unique_ptr<ParticleComputePass> m_pParticleComputePass;

		std::shared_ptr<StructuredBuffer<Particle>> m_pParticleBuffer;
		std::shared_ptr<StructuredBuffer<ParticleSystemSettings>> m_pParticleSystemBuffer;
		std::shared_ptr<StructuredBuffer<ParticleSystemRuntime>> m_pParticleSystemRuntimeBuffer;
		std::vector<std::unique_ptr<ParticleSystem>> m_pParticleSystems;
		std::unique_ptr<ParticleSystem> m_pLayeredParticleSystem;
	};

}