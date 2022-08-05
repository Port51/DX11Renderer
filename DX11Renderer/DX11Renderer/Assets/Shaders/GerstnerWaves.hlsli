#ifndef _GERSTNER_WAVES_INCLUDED
#define _GERSTNER_WAVES_INCLUDED

// This is used for demonstrating buoyancy with large, simple waves
//#define _BUOYANCY_DEBUG_TEST

float3 GerstnerWave(const float2 vPos, const float2 D, const float W, const float Q, const float T, const float amp)
{
	// cos(WiDi * (x, y) + Qt)
	const float theta = dot(W * D, vPos) + T;
	float s, c;
	sincos(theta, s, c);

	return float3(Q * D.x * c, s, Q * D.y * c) * amp;
}

float3 GetGerstnerWaves(float3 positionWS)
{
	// Fade out waves at horizon (looks better!)
	const float isHorizon = saturate(dot(positionWS.xz, positionWS.xz) * 0.0003);

	const float wM = 0.44f;
	const float qM = 0.85f;
	const float ampM = lerp(0.25f, 0.0f, isHorizon);

#if defined(_BUOYANCY_DEBUG_TEST)
	positionWS += GerstnerWave(positionWS.xz, float2(0.8384436f, 0.5449883f), 0.25f * wM, 1.0f * qM, _Time.y * 2.0f, ampM * 5.0f); // medium waves
#else
	positionWS += GerstnerWave(positionWS.xz, float2(0.8384436f, 0.5449883f), 1.0f * wM, 1.0f * qM, _Time.y * 2.0f, ampM * 0.5f); // medium waves
	positionWS += GerstnerWave(positionWS.xz, float2(0.2576626f, -0.9662349f), 3.18923f * wM, 1.1389f * qM, _Time.y * 3.91f, ampM * 0.75f); // quick waves
	positionWS += GerstnerWave(positionWS.xz, float2(0.6940289f, 0.7199471f), 3.18923f * wM, 1.1389f * qM, _Time.y * 4.07f, ampM * 0.85f); // quick waves
	positionWS += GerstnerWave(positionWS.xz, float2(-0.9941821f, 0.1077120f), 7.18923f * wM, 0.7789f * qM, _Time.y * 5.1f, ampM * 0.551f); // very quick waves
#endif

	return positionWS;
}

float3 GerstnerWaveTangent(const float2 vPos, const float2 D, const float W, const float Q, const float T, const float amp)
{
	// cos(WiDi * (x, y) + Qt)
	const float theta = dot(W * D, vPos) + T;
	float s, c;
	sincos(theta, s, c);

	return float3((D.x * D.x) * Q * s, D.x * c, (D.x * D.y) * Q * s) * amp;
}

float3 GerstnerWaveBitangent(const float2 vPos, const float2 D, const float W, const float Q, const float T, const float amp)
{
	// cos(WiDi * (x, y) + Qt)
	const float theta = dot(W * D, vPos) + T;
	float s, c;
	sincos(theta, s, c);

	return float3((D.y * D.x) * Q * s, D.x * c, (D.y * D.y) * Q * s) * amp;
}

float3 GetGerstnerWavesTangent(float3 positionWS)
{
	// Fade out waves at horizon (looks better!)
	const float isHorizon = saturate(dot(positionWS.xz, positionWS.xz) * 0.0003);

	const float wM = 0.44f;
	const float qM = 0.85f;
	const float ampM = lerp(0.25f, 0.0f, isHorizon) * 0.1; // lower amplitude (otherwise, need to calculate per-pixel or add tons of verts)

	// todo: optimize by precalculating sincos() for all of these?
	float3 tangent = float3(0, 0, 1);
#if defined(_BUOYANCY_DEBUG_TEST)
	tangent += GerstnerWaveTangent(positionWS.xz, float2(0.8384436f, 0.5449883f), 0.25f * wM, 1.0f * qM, _Time.y * 2.0f, ampM * 5.0f); // medium waves
#else
	tangent += GerstnerWaveTangent(positionWS.xz, float2(0.8384436f, 0.5449883f), 1.0f * wM, 1.0f * qM, _Time.y * 2.0f, ampM * 0.5f); // medium waves
	tangent += GerstnerWaveTangent(positionWS.xz, float2(0.2576626f, -0.9662349f), 3.18923f * wM, 1.1389f * qM, _Time.y * 3.91f, ampM * 0.75f); // quick waves
	tangent += GerstnerWaveTangent(positionWS.xz, float2(0.6940289f, 0.7199471f), 3.18923f * wM, 1.1389f * qM, _Time.y * 4.07f, ampM * 0.85f); // quick waves
	tangent += GerstnerWaveTangent(positionWS.xz, float2(-0.9941821f, 0.1077120f), 7.18923f * wM, 0.7789f * qM, _Time.y * 5.1f, ampM * 0.551f); // very quick waves
#endif

	return normalize(tangent);
}

float3 GetGerstnerWavesBitangent(float3 positionWS)
{
	// Fade out waves at horizon (looks better!)
	const float isHorizon = saturate(dot(positionWS.xz, positionWS.xz) * 0.0003);

	const float wM = 0.44f;
	const float qM = 0.85f;
	const float ampM = lerp(0.25f, 0.0f, isHorizon) * 0.1; // lower amplitude (otherwise, need to calculate per-pixel or add tons of verts)

	// todo: optimize by precalculating sincos() for all of these?
	float3 bitangent = float3(1, 0, 0);
#if defined(_BUOYANCY_DEBUG_TEST)
	bitangent += GerstnerWaveBitangent(positionWS.xz, float2(0.8384436f, 0.5449883f), 0.25f * wM, 1.0f * qM, _Time.y * 2.0f, ampM * 5.0f); // medium waves
#else
	bitangent += GerstnerWaveBitangent(positionWS.xz, float2(0.8384436f, 0.5449883f), 1.0f * wM, 1.0f * qM, _Time.y * 2.0f, ampM * 0.5f); // medium waves
	bitangent += GerstnerWaveBitangent(positionWS.xz, float2(0.2576626f, -0.9662349f), 3.18923f * wM, 1.1389f * qM, _Time.y * 3.91f, ampM * 0.75f); // quick waves
	bitangent += GerstnerWaveBitangent(positionWS.xz, float2(0.6940289f, 0.7199471f), 3.18923f * wM, 1.1389f * qM, _Time.y * 4.07f, ampM * 0.85f); // quick waves
	bitangent += GerstnerWaveBitangent(positionWS.xz, float2(-0.9941821f, 0.1077120f), 7.18923f * wM, 0.7789f * qM, _Time.y * 5.1f, ampM * 0.551f); // very quick waves
#endif

	return normalize(bitangent);
}

#endif