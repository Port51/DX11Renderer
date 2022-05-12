Texture2D<float4> InputTexture : register(t3);
StructuredBuffer<float> BlurWeights : register(t4);
RWTexture2D<float4> OutputTexture : register(u0);

float BloomLuminance(float3 color)
{
	// Use ITU BT.601 formula:
	// Y = 0.299 R + 0.587 G + 0.114 B
	return dot(color, float3(0.299f, 0.587f, 0.114f));
}

[numthreads(16, 16, 1)]
void Prefilter(uint3 tId : SV_DispatchThreadID)
{
	uint2 resolutionSrc;
	uint2 resolutionDst;
	InputTexture.GetDimensions(resolutionSrc.x, resolutionSrc.y);
	OutputTexture.GetDimensions(resolutionDst.x, resolutionDst.y);
	if (tId.x >= (uint) resolutionDst.x || tId.y >= (uint) resolutionDst.y)
		return;

	float lum = BloomLuminance(InputTexture[tId.xy].rgb);
	float bloom = saturate(lum * 1.1f - 0.1f); // todo: add settings
	OutputTexture[tId.xy] = float4(InputTexture[tId.xy].rgb * bloom, bloom);
}

// Separable gaussian blur with groupshared cache
#define BlurWidth 15u
#define BlurKernelSize (BlurWidth * 2 + 1)
groupshared float4 blurCache[64 + 2 * BlurWidth];

[numthreads(64, 1, 1)]
void HorizontalGaussian(uint3 gtId : SV_GroupThreadID, uint3 tId : SV_DispatchThreadID)
{
	uint2 resolution;
	InputTexture.GetDimensions(resolution.x, resolution.y);
	uint2 clampedId = min(tId.xy, (uint2)resolution.xy);

	// Cache within group bounds
	blurCache[gtId.x + BlurWidth] = InputTexture[clampedId.xy];

	// Extra samples for data outside group bounds
	if (gtId.x < BlurWidth)
	{
		int srcX = max(0, clampedId.x - BlurWidth);
		blurCache[gtId.x] = InputTexture[uint2(srcX, clampedId.y)];
	}
	else if (gtId.x >= 64u - BlurWidth)
	{
		int srcX = min(resolution.x - 1u, clampedId.x + BlurWidth);
		blurCache[gtId.x + 2u * BlurWidth] = InputTexture[uint2(srcX, clampedId.y)];
	}

	GroupMemoryBarrierWithGroupSync();

	if (tId.x >= (uint) resolution.x || tId.y >= (uint) resolution.y)
		return;

	// Apply blur
	float4 color = 0.0;
	[unroll(BlurKernelSize)]
	for (uint i = 0; i < BlurKernelSize; ++i) // 0-31
	{
		color += blurCache[i + gtId.x] * BlurWeights[i];
	}

	OutputTexture[tId.xy] = color;
}

[numthreads(1, 64, 1)]
void VerticalGaussian(uint3 gtId : SV_GroupThreadID, uint3 tId : SV_DispatchThreadID)
{
	uint2 resolution;
	InputTexture.GetDimensions(resolution.x, resolution.y);
	uint2 clampedId = min(tId.xy, (uint2)resolution.xy);

	// Cache within group bounds
	blurCache[gtId.y + BlurWidth] = InputTexture[clampedId.xy];

	// Extra samples for data outside group bounds
	if (gtId.y < BlurWidth)
	{
		int srcY = max(0, clampedId.y - BlurWidth);
		blurCache[gtId.y] = InputTexture[uint2(clampedId.x, srcY)];
	}
	else if (gtId.y >= 64u - BlurWidth)
	{
		int srcY = min(resolution.y - 1u, clampedId.y + BlurWidth);
		blurCache[gtId.y + 2u * BlurWidth] = InputTexture[uint2(clampedId.x, srcY)];
	}

	GroupMemoryBarrierWithGroupSync();

	if (tId.x >= (uint) resolution.x || tId.y >= (uint) resolution.y)
		return;

	// Apply blur
	float4 color = 0.0;
	[unroll(BlurKernelSize)]
	for (uint i = 0; i < BlurKernelSize; ++i)
	{
		color += blurCache[i + gtId.y] * BlurWeights[i];
	}

	OutputTexture[tId.xy] = color;
}

[numthreads(16, 16, 1)]
void Combine(uint3 tId : SV_DispatchThreadID)
{
	uint2 resolutionDst;
	OutputTexture.GetDimensions(resolutionDst.x, resolutionDst.y);
	if (tId.x >= (uint)resolutionDst.x || tId.y >= (uint)resolutionDst.y)
		return;

	uint2 srcId = tId >> 1u;

	// todo: add more settings here
	//float bloom = InputTexture[srcId.xy].a;
	//bloom = pow(bloom, 1.f) * 2.f;

	float4 bloomRGBA = float4(InputTexture[srcId.xy].rgb, 0.f);
	//OutputTexture[tId.xy] += bloomRGBA;
}
