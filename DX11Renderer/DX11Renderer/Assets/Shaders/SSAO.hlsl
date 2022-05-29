// References:
//  - http://john-chapman-graphics.blogspot.com/2013/01/ssao-tutorial.html
//  - https://learnopengl.com/Advanced-Lighting/SSAO

#include "./Common.hlsli"
#include "./HiZCommon.hlsli"

Texture2D<float4> NormalRoughReflectivityRT : register(t3);
Texture2D<float2> HZB : register(t4);
StructuredBuffer<float4> SampleOffsets : register(t5);
Texture2D<float4> NoiseTex : register(t6);
Texture2D<float> OcclusionIn : register(t7);
RWTexture2D<float> OcclusionOut : register(u0);
SamplerState BilinearSampler : register(s0);

float3x3 GetTBN(float3 normalVS, float3 randomVec)
{
	float3 tangentVS = normalize(randomVec - normalVS * dot(randomVec, normalVS));
	float3 bitangentVS = cross(normalVS, tangentVS);
	return float3x3(tangentVS, bitangentVS, normalVS);
}

[numthreads(16, 16, 1)]
void OcclusionPass(uint3 tId : SV_DispatchThreadID)
{
	uint2 resolution;
	NormalRoughReflectivityRT.GetDimensions(resolution.x, resolution.y);
	if (tId.x >= resolution.x || tId.y >= resolution.y)
		return;

	const float2 screenUV = tId.xy * _ScreenParams.zw;
	const float2 positionNDC = screenUV * 2.f - 1.f;
	const float linearDepth = HZB_LINEAR(HZB[tId.xy].x, _ZBufferParams);
	const float3 frustumPlaneVS = float3(positionNDC * _FrustumCornerDataVS.xy, 1.f);
	const float3 positionVS = frustumPlaneVS * linearDepth;

	const float4 gbufferTex = NormalRoughReflectivityRT[tId.xy];
	const float3 normalVS = GetNormalVSFromGBuffer(gbufferTex);

	// Construct TBN
	float3 randomVec = normalize(NoiseTex.SampleLevel(PointWrapSampler, tId.xy / 32.f, 0.f).xyz * 2.f - 1.f);
	float3x3 TBN = GetTBN(normalVS, randomVec);

	// todo: make into settings
	float _RadiusVS = 0.125;
	float _BiasVS = 0.001;

	float occlusion = 0.0;
	float debugValue = 0.0;
	float testVal = 0.0;
	const uint KernelSize = 64;
	for (uint i = 0u; i < KernelSize; ++i)
	{
		float3 sampleNormalVS = mul(SampleOffsets[i].xyz, TBN);

		// Setup sample
		float3 sampleVS = sampleNormalVS * _RadiusVS + positionVS;
		float2 sampleUV = GetScreenUVFromVS(sampleVS);

		float linearSampleDepth = HZB_LINEAR(HZB.SampleLevel(BilinearClampSampler, sampleUV.xy, 0.f).x, _ZBufferParams);
		float rangeValue = SCurve(saturate(_RadiusVS / (abs(positionVS.z - linearSampleDepth) + 0.0001f)));

		occlusion += step(linearSampleDepth, sampleVS.z - _BiasVS) * rangeValue;
		debugValue = (sampleVS.x - positionVS.x) * 10;
	}
	occlusion = SCurve(saturate(occlusion / KernelSize));

	OcclusionOut[tId.xy] = occlusion;
	//OcclusionOut[tId.xy] = debugValue;
}

[numthreads(16, 16, 1)]
void HorizontalBlurPass(uint3 tId : SV_DispatchThreadID)
{
	OcclusionOut[tId.xy] = OcclusionIn[tId.xy];
}

[numthreads(16, 16, 1)]
void VerticalBlurPass(uint3 tId : SV_DispatchThreadID)
{
	OcclusionOut[tId.xy] = OcclusionIn[tId.xy];
}