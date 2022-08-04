#ifndef _GERSTNER_WAVES_INCLUDED
#define _GERSTNER_WAVES_INCLUDED

float3 GerstnerWave(float2 vPos, float2 D, float W, float Q, float T, float amp)
{
	// cos(WiDi * (x, y) + Qt)
	float theta = dot(W * D, vPos) + T;
	float s, c;
	sincos(theta, s, c);

	return float3(Q * D.x * c, s, Q * D.y * c) * amp;
}

float3 GetGerstnerWaves(float3 positionWS)
{
	float wM = 0.44;
	float qM = 0.85;
	float ampM = 0.25;
	positionWS += GerstnerWave(positionWS.xz, normalize(float2(1, 0.65)), 1.0f * wM, 1.0f * qM, _Time.x * 50.0f, ampM * 0.5f); // medium waves
	positionWS += GerstnerWave(positionWS.xz, normalize(float2(0.2, -0.75)), 3.18923f * wM, 1.1389f * qM, _Time.x * 90.0f, ampM * 0.75f); // quick waves
	positionWS += GerstnerWave(positionWS.xz, normalize(float2(0.723, 0.75)), 3.18923f * wM, 1.1389f * qM, _Time.x * 90.0f, ampM * 0.85f); // quick waves
	positionWS += GerstnerWave(positionWS.xz, normalize(float2(-0.923, 0.01)), 7.18923f * wM, 0.7789f * qM, _Time.x * 115.0f, ampM * 0.551f); // very quick waves

	return positionWS;
	//return positionWS + float3(0, sin(_Time.x * 30.0 + positionWS.z) * 0.25, 0);
}

#endif