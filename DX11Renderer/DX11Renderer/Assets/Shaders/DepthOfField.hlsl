#include "./Common.hlsli"
#include "./HiZCommon.hlsli"

// Depth of field implementation
// References:
// - https://www.gdcvault.com/play/1025372/Circular-Separable-Convolution-Depth-of#:~:text=Overview%3A,very%20accurate%20and%20fast%20bokeh.

// Method:
// Uses separable gathers, with complex numbers
// Filter eq:	F(x) = e^(-ax^2) (cos(bx^2) + i * sin(bx^2))
// Combine eq:	C(x) = A * Freal(x) + B * Fimag(x)

// This approximates a disc, but due to being separable can be
// done in separate horizontal and vertical passes.

// Component parameters: (taken from GDC vault reference)
// C0: a = -0.886528, b = 5.268909, A = 0.411259, B = -0.548794
// C1: a = -1.960518, b = 1.558213, A = 0.513282, B =  4.561110

// Use 1 component for near, 2 components for far

Texture2D<float4> SRVTex0 : register(t3);
Texture2D<float4> SRVTex1 : register(t4);
Texture2D<float2> HiZBuffer : register(t5);
StructuredBuffer<float> DiskWeights : register(t6); // packed into (C0 real, C0 imaginary, C1 real, C1 imaginary)
RWTexture2D<float4> UAVTex0 : register(u0);
RWTexture2D<float4> UAVTex1 : register(u1);

// Separable gaussian blur with groupshared cache
#define DiscWidth 15u
#define DiscKernelSize (DiscWidth * 2 + 1)
groupshared float4 discCache[64 + 2 * DiscWidth];

cbuffer DOF_CB : register(b4)
{
	uint _WeightOffset;
	float3 padding;
};

float CalculateCoC(float linearDepth)
{
	// todo: settings!
	return (linearDepth - 18.5f) * 2.f;
}

float GetRealWeight(uint x)
{
	return DiskWeights[_WeightOffset + x];
}

float GetImaginaryWeight(uint x)
{
	return DiskWeights[_WeightOffset + x + DiscKernelSize];
}

[numthreads(16, 16, 1)]
void Prefilter(uint3 tId : SV_DispatchThreadID)
{
	uint2 resolution;
	SRVTex0.GetDimensions(resolution.x, resolution.y);
	if (tId.x >= (uint) resolution.x || tId.y >= (uint) resolution.y)
		return;

	// Mip 1 matches downrez'd texture
	float hzbFar = HiZBuffer.Load(int3(tId.xy, 1u)).r;
	hzbFar = HZB_LINEAR(hzbFar, _ZBufferParams);

	// Use larger tile sizes for near CoC to avoid edge artifacts
	float hzbNear = HiZBuffer.Load(int3(tId.xy >> 1u, 2u)).r;
	hzbNear = HZB_LINEAR(hzbNear, _ZBufferParams);

	float cocFar = saturate(CalculateCoC(hzbFar));
	float cocNear = saturate(CalculateCoC(hzbNear) * -1.f);
	
	// Premultiply RGB by CoC, and store CoC in alpha channel
	UAVTex0[tId.xy] = float4(SRVTex0[tId.xy].rgb * cocFar, cocFar);
	UAVTex1[tId.xy] = float4(SRVTex0[tId.xy].rgb * cocNear, cocNear);
}

// Input: UAVTex0 (intentional!)
// Outputs: UAVTex0, UAVTex1
[numthreads(64, 1, 1)]
void HorizontalFilter(uint3 gtId : SV_GroupThreadID, uint3 tId : SV_DispatchThreadID)
{
	uint2 resolution;
	UAVTex0.GetDimensions(resolution.x, resolution.y);
	uint2 clampedId = min(tId.xy, (uint2)resolution.xy);

	// Cache within group bounds
	discCache[gtId.x + DiscWidth] = UAVTex0[clampedId.xy];

	// Extra samples for data outside group bounds
	if (gtId.x < DiscWidth)
	{
		int srcX = max(0, clampedId.x - DiscWidth);
		discCache[gtId.x] = UAVTex0[uint2(srcX, clampedId.y)];
	}
	else if (gtId.x >= 64u - DiscWidth)
	{
		int srcX = min(resolution.x - 1u, clampedId.x + DiscWidth);
		discCache[gtId.x + 2u * DiscWidth] = UAVTex0[uint2(srcX, clampedId.y)];
	}

	GroupMemoryBarrierWithGroupSync();

	if (tId.x >= (uint) resolution.x || tId.y >= (uint) resolution.y)
		return;

	// Apply blur
	float4 real = 0.0;
	float4 imaginary = 0.0;
	[unroll(DiscKernelSize)]
	for (uint i = 0; i < DiscKernelSize; ++i)
	{
		real += discCache[i + gtId.x] * GetRealWeight(i);
		imaginary += discCache[i + gtId.x] * GetImaginaryWeight(i);
	}

	UAVTex0[tId.xy] = saturate(real);
	UAVTex1[tId.xy] = saturate(imaginary);
}

// Input: UAVTex0 (intentional!)
// Outputs: UAVTex0, UAVTex1
[numthreads(1, 64, 1)]
void VerticalFilterAndCombine(uint3 gtId : SV_GroupThreadID, uint3 tId : SV_DispatchThreadID)
{
	uint2 resolution;
	SRVTex0.GetDimensions(resolution.x, resolution.y);
	uint2 clampedId = min(tId.xy, (uint2)resolution.xy);

	// Cache within group bounds
	discCache[gtId.y + DiscWidth] = SRVTex0[clampedId.xy];

	// Extra samples for data outside group bounds
	if (gtId.y < DiscWidth)
	{
		int srcY = max(0, clampedId.y - DiscWidth);
		discCache[gtId.y] = SRVTex0[uint2(clampedId.x, srcY)];
	}
	else if (gtId.y >= 64u - DiscWidth)
	{
		int srcY = min(resolution.y - 1u, clampedId.y + DiscWidth);
		discCache[gtId.y + 2u * DiscWidth] = SRVTex0[uint2(clampedId.x, srcY)];
	}

	GroupMemoryBarrierWithGroupSync();

	if (tId.x >= (uint) resolution.x || tId.y >= (uint) resolution.y)
		return;

	// Apply blur
	float4 real = 0.0;
	float4 imaginary = 0.0;
	[unroll(DiscKernelSize)]
	for (uint i = 0; i < DiscKernelSize; ++i)
	{
		real += discCache[i + gtId.y] * GetRealWeight(i);
		imaginary += discCache[i + gtId.y] * GetImaginaryWeight(i);
	}

	UAVTex0[tId.xy] = real;
	UAVTex1[tId.xy] = imaginary;
}

[numthreads(16, 16, 1)]
void Combine(uint3 tId : SV_DispatchThreadID)
{
	uint2 resolutionDst;
	UAVTex0.GetDimensions(resolutionDst.x, resolutionDst.y);
	if (tId.x >= (uint)resolutionDst.x || tId.y >= (uint)resolutionDst.y)
		return;

	uint2 srcId = tId >> 1u;

	// SRVTex0 = far Coc
	// SRVTex1 = near Coc
}
