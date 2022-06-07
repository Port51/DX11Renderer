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
		void Execute(const GraphicsDevice& gfx, const Camera& camera) const;
	protected:
		const size_t GetParticleSystemCount() const;
		static bool SortByDistance(const std::pair<size_t, float>& a, const std::pair<size_t, float>& b);
	protected:

		size_t maxParticles;

		std::unique_ptr<ComputeKernel> m_pSpawnParticlesKernel;
		std::unique_ptr<ComputeKernel> m_pUpdateParticlesKernel;

		std::unique_ptr<StructuredBuffer<Particle>> m_pParticleBuffer;
		std::unique_ptr<StructuredBuffer<ParticleSystemSettings>> m_pParticleSystemBuffer;
		std::vector<std::unique_ptr<ParticleSystem>> m_pParticleSystems;
		std::unique_ptr<ParticleSystem> m_pLayeredParticleSystem;
	};

}