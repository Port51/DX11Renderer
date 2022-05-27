// References:
//  - http://john-chapman-graphics.blogspot.com/2013/01/ssao-tutorial.html
//  - https://learnopengl.com/Advanced-Lighting/SSAO

#include "./Common.hlsli"

Texture2D<float4> NormalRoughReflectivityRT : register(t3);
Texture2D<float4> CameraColorIn : register(t4);
Texture2D<float> DepthRT : register(t5);
StructuredBuffer<float4> SampleOffsets : register(t6);
RWTexture2D<float4> CameraColorOut : register(u0);
SamplerState BilinearSampler : register(s0);

float3x3 GetTBN(float3 normalVS, float3 randomVec)
{
	float3 tangentVS = normalize(randomVec - normalVS * dot(randomVec, normalVS));
	float3 bitangentVS = cross(normalVS, tangentVS);
	return float3x3(tangentVS, bitangentVS, normalVS);
}

[numthreads(16, 16, 1)]
void CSMain(uint3 tId : SV_DispatchThreadID)
{
	/*uint2 resolution;
	NormalRoughReflectivityRT.GetDimensions(resolution.x, resolution.y);
	if (tId.x >= resolution.x || tId.y >= resolution.y)
		return;

	const float4 gbufferTex = NormalRoughReflectivityRT[tId.xy];
	const float3 normalVS = GetNormalVSFromGBuffer(gbufferTex);

	// Construct TBN
	float3x3 TBN = GetTBN(normalVS, randomVec);

	float occlusion = 0.0;
	float testVal = 0.0;
	const uint KernelSize = 64;
	for (uint i = 0u; i < KernelSize; ++i)
	{
		// NOTE: TBN direction isn't close to normalVS direction

		float3 sampleNormalVS = mul(TBN, SampleOffsets[i]);

		// Setup sample
		float3 sampleVS = sampleNormalVS * _RadiusVS + positionVS;
		float2 sampleUV = GetPositionNDCFromVS(sampleVS) * 0.5f + 0.5f;

		float rawSampleDepth = DepthRT.SampleLevel(PointClampSampler, sampleUV.xy, 0);
		float linearSampleDepth = RawDepthToLinearDepth(rawSampleDepth);

		float rangeOffset = saturate(1.0 - abs(positionVS.z - linearSampleDepth) / _RadiusVS);
		float rangeValue = SCurve(rangeOffset);
		//rangeValue = 1;
		occlusion += ((linearSampleDepth <= sampleVS.z - _BiasVS) ? 1.0 : 0.0) * rangeValue;
	}
	occlusion = saturate(occlusion / KernelSize);

	// todo: output to blur texture
	CameraColorOut[tId.xy] = occlusion;*/
}