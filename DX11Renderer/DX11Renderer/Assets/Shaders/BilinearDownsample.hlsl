// Downsamples by x2

Texture2D<float4> DownsampleInput : register(t3);
RWTexture2D<float4> DownsampleOutput : register(u0);

[numthreads(16, 16, 1)]
void CSMain(uint3 tId : SV_DispatchThreadID)
{
	uint2 resolutionSrc;
	uint2 resolutionDst;
	DownsampleInput.GetDimensions(resolutionSrc.x, resolutionSrc.y);
	DownsampleOutput.GetDimensions(resolutionDst.x, resolutionDst.y);
	if (tId.x >= (uint) resolutionDst.x || tId.y >= (uint) resolutionDst.y)
		return;

	uint2 srcId = tId.xy * 2u;

	// Don't allow overextending
	uint2 srcDispl = uint2(
		(uint) resolutionSrc.x > srcId.x + 1u ? 1u : 0u,
		(uint) resolutionSrc.y > srcId.y + 1u ? 1u : 0u);

	float4 d00 = DownsampleInput[srcId.xy];
	float4 d10 = DownsampleInput[srcId.xy + uint2(srcDispl.x, 0)];
	float4 d01 = DownsampleInput[srcId.xy + uint2(0, srcDispl.y)];
	float4 d11 = DownsampleInput[srcId.xy + srcDispl.xy];
	DownsampleOutput[tId.xy] = (d00 + d10 + d01 + d11) * 0.25f;

}