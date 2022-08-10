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
	uint systemIdx; // todo: pack with isAlive
	float padding0;
	float padding1;
	float padding2;
};

struct ParticleInstanceStruct
{
	float4 positionVS;
	float4 color;
	float lifePercent;
	float scale;
	float padding0;
	float padding1;
};

struct ParticleSortData
{
	int index;
	float sortValue;
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
	uint bufferOffset;
	float padding0;
	float padding1;
};

struct ParticleSystemRuntime
{
	uint nextIdx;
	uint nextSortIdx;
	float padding0;
	float padding1;
};

cbuffer ParticleManagerCB : register(b4)
{
	uint _ParticleSystemCount;
};

StructuredBuffer<ParticleSystemSettings> ParticleSystems : register(t3);
RWStructuredBuffer<ParticleStruct> Particles : register(u0);
RWStructuredBuffer<ParticleSystemRuntime> ParticleSystemsRuntime : register(u1);
RWStructuredBuffer<ParticleSortData> ParticleSortBuffer : register(u2);
RWStructuredBuffer<ParticleInstanceStruct> ParticleInstanceBuffer : register(u3);
RWBuffer<uint> ArgsBuffer : register(u4);

// Helper macro for retrieving particles with buffer offset
#define GET_PARTICLE(idx, system) \
	Particles[idx + system.bufferOffset]

// Spawns and kills particles
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
		uint startIdx = systemRuntime.nextIdx;
		systemRuntime.nextIdx = (systemRuntime.nextIdx + 1u) % system.maxParticles;
		while (systemRuntime.nextIdx != startIdx)
		{
			if (!GET_PARTICLE(systemRuntime.nextIdx, system).isAlive)
				break;
			systemRuntime.nextIdx = (systemRuntime.nextIdx + 1u) % system.maxParticles;
		}

		// Spawn particle
		ParticleStruct newPrt;
		newPrt.isAlive = 1u;
		newPrt.systemIdx = tId.x;
		newPrt.lifePercent = 1.f;
		newPrt.maxLife = 2.f;
		newPrt.scale = 1.f;
		newPrt.velocityWS = float4(0.f, 1.f, 0.f, 1.f);
		newPrt.color = 1.f;
		newPrt.positionWS = system.emitPositionWS;
		newPrt.padding0 = 0;
		newPrt.padding1 = 0;
		newPrt.padding2 = 0;

		GET_PARTICLE(systemRuntime.nextIdx, system) = newPrt;
	}

	// Reset this every frame!
	systemRuntime.nextSortIdx = 0u;

	// Save runtime
	ParticleSystemsRuntime[tId.x] = systemRuntime;

}

// Simulates particles
[numthreads(64, 1, 1)]
void UpdateParticles(uint3 tId : SV_DispatchThreadID)
{
	ParticleStruct p = Particles[tId.x];

	// todo: just scale timestep by p.isAlive and avoid branch - need to make sure all logic can support this
	if (p.isAlive)
	{
		// Simulation
		p.positionWS.xyz += p.velocityWS.xyz * _TimeStep.z;

		// Lifetime
		p.lifePercent -= _TimeStep.z / max(0.001f, p.maxLife);
		p.isAlive = (p.lifePercent > 0.f);
		p.lifePercent *= p.isAlive;
	}

	Particles[tId.x] = p;

	// Also, reset sorted particles
	ParticleSortData nullParticleData;
	nullParticleData.index = -1; // null
	nullParticleData.sortValue = 0.f;
	ParticleSortBuffer[tId.x] = nullParticleData;
}

// Cull to camera and get distances + sort bits (for separating materials)
[numthreads(64, 1, 1)]
void FrustumCull(uint3 tId : SV_DispatchThreadID)
{
	ParticleStruct p = Particles[tId.x];
	ParticleSystemSettings system = ParticleSystems[p.systemIdx];

	// WS --> VS
	const float3 positionVS = mul(_ViewMatrix, float4(p.positionWS.xyz, 1.f));
	const float radiusVS = p.scale;

	// Cull
	const float2 intersections = float2(
		dot(abs(positionVS.xz), _FrustumPlaneDirVS.xy),
		dot(abs(positionVS.yz), _FrustumPlaneDirVS.zw));
	const bool isInside = all(intersections < 0.f) && (p.isAlive);

	// TESTING:
	/*ParticleSortData n;
	n.index = tId.x;
	n.linearDepth = p.isAlive;
	ParticleSortBuffer[tId.x] = n;*/

	// Output to render buffer
	if (isInside)
	{
		ParticleSortData newParticleData;
		newParticleData.index = (int)tId.x;
		newParticleData.sortValue = positionVS.z;

		uint insertIdx;
		InterlockedAdd(ParticleSystemsRuntime[p.systemIdx].nextSortIdx, 1u, insertIdx);
		ParticleSortBuffer[insertIdx + system.bufferOffset] = newParticleData;
	}
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

// Transfer sorted particles to instance buffer and setup indirect arguments
[numthreads(64, 1, 1)]
void SetupInstanceBuffer(uint3 tId : SV_DispatchThreadID)
{
	if (tId.x < _ParticleSystemCount)
	{
		// Setup arguments
		// Ref: https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_draw_instanced_indirect_args
		uint systemIdx = tId.x;
		uint instanceCt = ParticleSystemsRuntime[systemIdx].nextSortIdx;

		ArgsBuffer[systemIdx * 4u + 0u] = 1u;			// vert count
		ArgsBuffer[systemIdx * 4u + 1u] = instanceCt;	// instance count
		ArgsBuffer[systemIdx * 4u + 2u] = 0u;			// start vert location
		ArgsBuffer[systemIdx * 4u + 3u] = 0u;			// start instance location
	}

	// todo: is this branch needed? can we just write garbage data for dead particles and use indirect arguments to ignore them?
	int srcIdx = ParticleSortBuffer[tId.x].index;
	if (srcIdx != -1)
	{
		// Translate into particle instance
		ParticleStruct p = Particles[srcIdx];
		ParticleInstanceStruct pi;

		pi.positionVS = mul(_ViewMatrix, float4(p.positionWS.xyz, 1.f));
		pi.color = p.color;
		pi.lifePercent = p.lifePercent;
		pi.scale = p.scale;
		pi.padding0 = 0;
		pi.padding1 = 0;

		ParticleInstanceBuffer[tId.x] = pi;
	}
	
}

// After this, uses graphics pipeline for rendering the particles