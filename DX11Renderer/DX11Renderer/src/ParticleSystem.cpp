#include "pch.h"
#include "ParticleSystem.h"
#include "GraphicsDevice.h"

namespace gfx
{
	ParticleSystem::ParticleSystem(size_t maxParticleCount, dx::XMVECTOR positionWS)
		: m_maxParticleCount(maxParticleCount), m_positionWS(positionWS)
	{
	}

	ParticleSystem::~ParticleSystem()
	{
	}

	const size_t ParticleSystem::GetMaxParticleCount() const
	{
		return m_maxParticleCount;
	}

	const dx::XMVECTOR ParticleSystem::GetPositionWS() const
	{
		return m_positionWS;
	}

	void ParticleSystem::DrawIndirect(const GraphicsDevice & gfx) const
	{
		//gfx.GetContext()->DrawInstancedIndirect();
	}
}