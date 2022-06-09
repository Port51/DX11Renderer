#pragma once
#include "CommonHeader.h"
#include "ParticleCommon.h"

namespace gfx
{
	class GraphicsDevice;
	class Material;
	class Topology;
	class RenderState;

	class ParticleSystem
	{
	public:
		ParticleSystem(size_t maxParticleCount, dx::XMVECTOR positionWS);
		~ParticleSystem();
	public:
		const size_t GetMaxParticleCount() const;
		const dx::XMVECTOR GetPositionWS() const;
		void DrawIndirect(const GraphicsDevice& gfx, RenderState& renderState, ID3D11Buffer* pArgsBuffer, UINT byteOffset) const;
		ParticleSystemSettings GetParticleSystemSettings(const size_t bufferOffset = 0u) const;
	protected:
		size_t m_maxParticleCount;
		dx::XMVECTOR m_positionWS;
		std::shared_ptr<Material> m_pMaterial;
		std::shared_ptr<Topology> m_pTopology;
	};

}