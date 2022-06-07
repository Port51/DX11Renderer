#pragma once
#include "CommonHeader.h"
#include "ParticleCommon.h"

namespace gfx
{
	class GraphicsDevice;

	class ParticleSystem
	{
	public:
		ParticleSystem(size_t maxParticleCount, dx::XMVECTOR positionWS);
		~ParticleSystem();
	public:
		const size_t GetMaxParticleCount() const;
		const dx::XMVECTOR GetPositionWS() const;
		void DrawIndirect(const GraphicsDevice& gfx) const;
	protected:
		size_t m_maxParticleCount;
		dx::XMVECTOR m_positionWS;
	};

}