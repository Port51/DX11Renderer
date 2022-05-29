// References:
//  - http://john-chapman-graphics.blogspot.com/2013/01/ssao-tutorial.html
//  - https://learnopengl.com/Advanced-Lighting/SSAO

#include "./Common.hlsli"
#include "./HiZCommon.hlsli"

cbuffer SSAO_CB : register(b4)
{
float _RadiusVS;
float _BiasVS;
float2 _Padding;
};

Texture2D<float4> NormalRoughReflectivityRT : register(t3);
Texture2D<float2> HZB : register(t4);
StructuredBuffer<float4> SampleOffsets : register(t5);
Texture2D<float4> NoiseTex : register(t6);
Texture2D<float> OcclusionIn : register(t7);
StructuredBuffer<float> BlurWeights : register(t8);
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

// Depth-aware gaussian blur
// todo: move to shared gaussian shader

#define BlurWidth 15u
#define BlurKernelSize (BlurWidth * 2 + 1)
groupshared float2 blurCache[64 + 2 * BlurWidth];

// Return unweighted sample with depth
float2 GetOcclusionDepthSample(uint2 sampleId)
{
	float sampleOcclusion = OcclusionIn[sampleId.xy].r;
	float linearSampleDepth = HZB_LINEAR(HZB[sampleId.xy].x, _ZBufferParams);

	return float2(sampleOcclusion, linearSampleDepth);
}

[numthreads(64, 1, 1)]
void HorizontalBlurPass(uint3 gtId : SV_GroupThreadID, uint3 tId : SV_DispatchThreadID)
{
	uint2 resolution;
	OcclusionIn.GetDimensions(resolution.x, resolution.y);
	uint2 clampedId = min(tId.xy, (uint2)resolution.xy);

	// Cache within group bounds
	blurCache[gtId.x + BlurWidth] = GetOcclusionDepthSample(clampedId.xy);

	// Extra samples for data outside group bounds
	if (gtId.x < BlurWidth)
	{
		int srcX = max(0, clampedId.x - BlurWidth);
		blurCache[gtId.x] = GetOcclusionDepthSample(uint2(srcX, clampedId.y));
	}
	else if (gtId.x >= 64u - BlurWidth)
	{
		int srcX = min(resolution.x - 1u, clampedId.x + BlurWidth);
		blurCache[gtId.x + 2u * BlurWidth] = GetOcclusionDepthSample(uint2(srcX, clampedId.y));
	}

	GroupMemoryBarrierWithGroupSync();

	if (tId.x >= (uint) resolution.x || tId.y >= (uint) resolution.y)
		return;

	const float linearDepth = HZB_LINEAR(HZB[tId.xy].x, _ZBufferParams);

	// Apply blur
	float occlusion = 0.0;
	[unroll(BlurKernelSize)]
	for (uint i = 0; i < BlurKernelSize; ++i) // 0-31
	{
		float sampleOcclusion = blurCache[i + gtId.x].x;
		float sampleDepth = blurCache[i + gtId.x].y;

		float depthOffset = abs(sampleDepth - linearDepth);
		float rangeOffset = SCurve(saturate(1.f - depthOffset / _RadiusVS));

		occlusion += sampleOcclusion * BlurWeights[i] * rangeOffset;
	}

	OcclusionOut[tId.xy] = occlusion;
}

[numthreads(1, 64, 1)]
void VerticalBlurPass(uint3 gtId : SV_GroupThreadID, uint3 tId : SV_DispatchThreadID)
{
	uint2 resolution;
	OcclusionIn.GetDimensions(resolution.x, resolution.y);
	uint2 clampedId = min(tId.xy, (uint2)resolution.xy);

	// Cache within group bounds
	blurCache[gtId.y + BlurWidth] = GetOcclusionDepthSample(clampedId.xy);

	// Extra samples for data outside group bounds
	if (gtId.y < BlurWidth)
	{
		int srcY = max(0, clampedId.y - BlurWidth);
		blurCache[gtId.y] = GetOcclusionDepthSample(uint2(clampedId.x, srcY));
	}
	else if (gtId.y >= 64u - BlurWidth)
	{
		int srcY = min(resolution.y - 1u, clampedId.y + BlurWidth);
		blurCache[gtId.y + 2u * BlurWidth] = GetOcclusionDepthSample(uint2(clampedId.x, srcY));
	}

	GroupMemoryBarrierWithGroupSync();

	if (tId.x >= (uint) resolution.x || tId.y >= (uint) resolution.y)
		return;

	const float linearDepth = HZB_LINEAR(HZB[tId.xy].x, _ZBufferParams);

	// Apply blur
	float occlusion = 0.0;
	[unroll(BlurKernelSize)]
	for (uint i = 0; i < BlurKernelSize; ++i)
	{
		float sampleOcclusion = blurCache[i + gtId.y].x;
		float sampleDepth = blurCache[i + gtId.y].y;

		float depthOffset = abs(sampleDepth - linearDepth);
		float rangeOffset = SCurve(saturate(1 - depthOffset / _RadiusVS));
		occlusion += sampleOcclusion * BlurWeights[i] * rangeOffset;
	}

	OcclusionOut[tId.xy] = occlusion;
}