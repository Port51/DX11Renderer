// References:
//  - https://www.gdcvault.com/play/1020002/Advanced-Visual-Effects-with-DirectX

#include "./Common.hlsli"
#include "./HiZCommon.hlsli"

struct ParticleStruct
{
	uint isAlive; // todo: pack more data in here?
	float lifePercent;
	float maxLife;
	float scale;
	float4 velocityWS;
	float4 color;
	float4 positionWS;
};

struct ParticleSystemSettings
{
	float4 emitPositionWS;
	float emitRateMin;
	float emitRateMax;
	float lifetimeMin;
	float lifetimeMax;
	float4 velocityMinWS;
	float4 velocityMaxWS;
	float4 startColor0;
	float4 startColor1;
	uint maxParticles;
};

struct ParticleSystemRuntime
{
	uint nextIdx;
	float padding0;
	float padding1;
	float padding2;
};

cbuffer ParticleManagerCB : register(b4)
{
	uint _ParticleSystemCount;
};

StructuredBuffer<ParticleSystemSettings> ParticleSystems : register(t3);
RWStructuredBuffer<ParticleStruct> Particles : register(u0);
RWStructuredBuffer<ParticleSystemRuntime> ParticleSystemsRuntime : register(u1);

// Spawns and kills particles
// 1 thread per particle system (for now!)
[numthreads(64, 1, 1)]
void SpawnParticles(uint3 tId : SV_DispatchThreadID)
{
	if (tId.x > _ParticleSystemCount)
		return;

	ParticleSystemSettings system = ParticleSystems[tId.x];
	ParticleSystemRuntime systemRuntime = ParticleSystemsRuntime[tId.x];

	float emission = 1.3f;
	uint numToSpawn = floor(emission);
	float overflowEmission = frac(emission); // save for next frame

	for (uint i = 0u; i < numToSpawn; ++i)
	{
		// Find next idx
		uint startIdx = 0u;
		while (systemRuntime.nextIdx != startIdx)
		{
			if (!Particles[systemRuntime.nextIdx].isAlive)
				break;
			systemRuntime.nextIdx = (systemRuntime.nextIdx + 1u) % system.maxParticles;
		}

		// Spawn particle
		ParticleStruct newPrt;
		newPrt.isAlive = 1u;
		newPrt.lifePercent = 0.f;
		newPrt.maxLife = 2.f;
		newPrt.scale = 1.f;
		newPrt.velocityWS = float4(0.f, 1.f, 0.f, 1.f);
		newPrt.color = 1.f;
		newPrt.positionWS = system.emitPositionWS;

		Particles[systemRuntime.nextIdx] = newPrt;
	}

	// Save runtime
	ParticleSystemsRuntime[tId.x] = systemRuntime;

}

// Moves particles
[numthreads(64, 1, 1)]
void UpdateParticles(uint3 tId : SV_DispatchThreadID)
{

}

// Cull to camera and get distances + sort bits (for separating materials)
[numthreads(64, 1, 1)]
void FrustumCull(uint3 tId : SV_DispatchThreadID)
{

}

// Bin particles into tiles
[numthreads(64, 1, 1)]
void BinTiles(uint3 tId : SV_DispatchThreadID)
{
	// Test against frustum + AABB
}

// Bitonic sort
[numthreads(64, 1, 1)]
void BitonicSort(uint3 tId : SV_DispatchThreadID)
{

}

// After this, uses graphics pipeline for rendering the particles