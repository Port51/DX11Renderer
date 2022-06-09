#pragma once
#include "CommonHeader.h"

namespace gfx
{

	struct alignas(16) Particle
	{
		u32 isAlive; // todo: pack more data in here?
		float lifePercent;
		float maxLife;
		float scale;
		dx::XMVECTOR velocityWS;
		dx::XMVECTOR color;
		dx::XMVECTOR positionWS;
		u32 systemIdx;
		float padding0;
		float padding1;
		float padding2;
	};

	struct alignas(16) ParticleInstance
	{
		dx::XMVECTOR positionVS;
		dx::XMVECTOR color;
		float lifePercent;
		float scale;
		float padding0;
		float padding1;
	};

	struct ParticleSortData
	{
		i32 index;
		float sortValue;
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
		u32 maxParticles;
		u32 bufferOffset;
		float padding0;
		float padding1;
	};

	struct ParticleSystemRuntime
	{
		u32 nextIdx;
		float padding0;
		float padding1;
		float padding2;
	};

	struct ParticleManagerCB
	{
		u32 particleSystemCount;
	};

}