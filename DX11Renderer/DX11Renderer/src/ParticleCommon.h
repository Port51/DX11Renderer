#pragma once
#include "CommonHeader.h"

namespace gfx
{

	struct Particle
	{
		u32 isAlive; // todo: pack more data in here?
		float lifePercent;
		float maxLife;
		float scale;
		dx::XMVECTOR velocityWS;
		dx::XMVECTOR color;
		dx::XMVECTOR positionWS;
	};

	// Used for actual rendering
	struct ParticleVertexData
	{
		dx::XMVECTOR positionVSAndLife;
		dx::XMVECTOR tint;
	};

	struct ParticleSystemSettings
	{
		dx::XMVECTOR emitPositionWS;
		float emitRateMin;
		float emitRateMax;
		float lifetimeMin;
		float lifetimeMax;
		dx::XMVECTOR velocityMinWS;
		dx::XMVECTOR velocityMaxWS;
		dx::XMVECTOR startColor0;
		dx::XMVECTOR startColor1;
	};

	struct ParticleManagerCB
	{
		u32 particleSystemCount;
	};

}