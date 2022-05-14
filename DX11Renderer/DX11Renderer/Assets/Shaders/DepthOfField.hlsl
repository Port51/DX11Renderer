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
RWTexture2D<float4> UAVTex2 : register(u2);

// Separable gaussian blur with groupshared cache
#define DiscWidth 31u
#define DiscKernelSize (DiscWidth * 2 + 1)
groupshared float4 discCache0[64 + 2 * DiscWidth];
groupshared float4 discCache1[64 + 2 * DiscWidth];

cbuffer DOF_CB : register(b4)
{
	uint _WeightOffset;
	float _VerticalPassAddFactor; // if 0, will overwrite. if 1, will add.
	float _CombineRealFactor;
	float _CombineImaginaryFactor;
	float _NearCoCScale;
	float _NearCoCBias;
	float _NearCoCIntensity;
	float _FarCoCScale;
	float _FarCoCBias;
	float _FarCoCIntensity;
	float2 _Padding;
};

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

	// Apply SCurve as mid-intensity blur can look weird
	float cocFar = SCurve(saturate(hzbFar * _FarCoCScale + _FarCoCBias)) * _FarCoCIntensity;
	float cocNear = SCurve(saturate(hzbNear * _NearCoCScale + _NearCoCBias)) * _NearCoCIntensity;
	
	// Premultiply RGB by CoC, and store CoC in alpha channel
	float3 origColor = SRVTex0[tId.xy].rgb;
	UAVTex0[tId.xy] = float4(origColor * cocFar, cocFar);
	UAVTex1[tId.xy] = float4(origColor * cocNear, cocNear);

	// Debug output (makes a single pixel white)
	//UAVTex0[tId.xy] = all(tId.xy == 200u);
	//UAVTex1[tId.xy] = all(tId.xy == 200u);
}

// Input: SRVTex0
// Outputs: UAVTex0, UAVTex1
[numthreads(64, 1, 1)]
void HorizontalFilter(uint3 gtId : SV_GroupThreadID, uint3 tId : SV_DispatchThreadID)
{
	uint2 resolution;
	SRVTex0.GetDimensions(resolution.x, resolution.y);
	uint2 clampedId = min(tId.xy, (uint2)resolution.xy);

	// Cache within group bounds
	discCache0[gtId.x + DiscWidth] = SRVTex0[clampedId.xy];

	// Extra samples for data outside group bounds
	if (gtId.x < DiscWidth)
	{
		int srcX = max(0, clampedId.x - DiscWidth);
		discCache0[gtId.x] = SRVTex0[uint2(srcX, clampedId.y)];
	}
	else if (gtId.x >= 64u - DiscWidth)
	{
		int srcX = min(resolution.x - 1u, clampedId.x + DiscWidth);
		discCache0[gtId.x + 2u * DiscWidth] = SRVTex0[uint2(srcX, clampedId.y)];
	}

	GroupMemoryBarrierWithGroupSync();

	if (tId.x >= (uint) resolution.x || tId.y >= (uint) resolution.y)
		return;

	// Sum of complex numbers is done component-wise
	// P + Q = (Pr + Qr) + (Pi + Qi)i
	float4 Sr = 0.0;
	float4 Si = 0.0;
	[unroll(DiscKernelSize)]
	for (uint i = 0; i < DiscKernelSize; ++i)
	{
		// Multiplication of real with complex number
		Sr += discCache0[i + gtId.x] * GetRealWeight(i);
		Si += discCache0[i + gtId.x] * GetImaginaryWeight(i);
	}

	UAVTex0[tId.xy] = Sr;
	UAVTex1[tId.xy] = Si;
}

// Input: UAVTex0, UAVTex1
// Outputs: UAVTex2 (combined)
[numthreads(1, 64, 1)]
void VerticalFilterAndCombine(uint3 gtId : SV_GroupThreadID, uint3 tId : SV_DispatchThreadID)
{
	uint2 resolution;
	UAVTex0.GetDimensions(resolution.x, resolution.y);
	uint2 clampedId = min(tId.xy, (uint2)resolution.xy);

	// Cache within group bounds
	discCache0[gtId.y + DiscWidth] = UAVTex0[clampedId.xy];
	discCache1[gtId.y + DiscWidth] = UAVTex1[clampedId.xy];

	// Extra samples for data outside group bounds
	if (gtId.y < DiscWidth)
	{
		int srcY = max(0, clampedId.y - DiscWidth);
		discCache0[gtId.y] = UAVTex0[uint2(clampedId.x, srcY)];
		discCache1[gtId.y] = UAVTex1[uint2(clampedId.x, srcY)];
	}
	else if (gtId.y >= 64u - DiscWidth)
	{
		int srcY = min(resolution.y - 1u, clampedId.y + DiscWidth);
		discCache0[gtId.y + 2u * DiscWidth] = UAVTex0[uint2(clampedId.x, srcY)];
		discCache1[gtId.y + 2u * DiscWidth] = UAVTex1[uint2(clampedId.x, srcY)];
	}

	GroupMemoryBarrierWithGroupSync();

	if (tId.x >= (uint) resolution.x || tId.y >= (uint) resolution.y)
		return;

	// Sum of complex numbers is done component-wise
	// P + Q = (Pr + Qr) + (Pi + Qi)i
	float4 Sr = 0.0;
	float4 Si = 0.0;
	[unroll(DiscKernelSize)]
	for (uint i = 0; i < DiscKernelSize; ++i)
	{
		// Multiplication of complex numbers
		// P * Q = PrQr - PiQi + [PrQi + PrSi]i

		float4 Pr = discCache0[i + gtId.y];
		float4 Pi = discCache1[i + gtId.y];
		float4 Qr = GetRealWeight(i);
		float4 Qi = GetImaginaryWeight(i);

		Sr += (Pr * Qr - Pi * Qi);
		Si += (Pr * Qi + Qr * Pi);
	}

	// Allows for adding or overwriting
	UAVTex2[tId.xy] = UAVTex2[tId.xy] * _VerticalPassAddFactor + (Sr * _CombineRealFactor + Si * _CombineImaginaryFactor);

	// Debug views
	//UAVTex2[tId.xy] = Sr;
	//UAVTex2[tId.xy] = Si;
}

// Blends near and far CoC with camera color
// Input: UAVTex0, SRVTex0, SRVTex1
// Outputs: UAVTex0 (combined)
[numthreads(16, 16, 1)]
void Composite(uint3 tId : SV_DispatchThreadID)
{
	uint2 resolutionDst;
	UAVTex0.GetDimensions(resolutionDst.x, resolutionDst.y);
	if (tId.x >= (uint)resolutionDst.x || tId.y >= (uint)resolutionDst.y)
		return;

	uint2 dofId = tId >> 1u;

	float4 src = UAVTex0[tId.xy];
	float4 farCoC = saturate(SRVTex0[dofId.xy]);
	float4 nearCoC = saturate(SRVTex1[dofId.xy]);

	float3 color = lerp(lerp(src, farCoC.rgb, farCoC.a), nearCoC.rgb, nearCoC.a);
	UAVTex0[tId.xy] = float4(max(color, 0.f), src.a);
}
