#ifndef COMMON_INCLUDED
#define COMMON_INCLUDED

#include "./CbufCommon.hlsli"

// Z buffer to linear depth.
// Does NOT correctly handle oblique view frustums.
// Does NOT work with orthographic projection.
// zBufferParam = { (f-n)/n, 1, (f-n)/n*f, 1/f }
float LinearEyeDepth(float depth01, float4 zBufferParam)
{
    return 1.0 / (zBufferParam.z * depth01 + zBufferParam.w);
}

// Z buffer to linear 0..1 depth (0 at camera position, 1 at far plane).
// Does NOT work with orthographic projections.
// Does NOT correctly handle oblique view frustums.
// zBufferParam = { (f-n)/n, 1, (f-n)/n*f, 1/f }
float Linear01Depth(float depth01, float4 zBufferParam)
{
    return 1.0 / (zBufferParam.x * depth01 + zBufferParam.y);
}

float Depth01ToEyeDepth(float depth01)
{
    return depth01 * _ProjectionParams.z;
}

float RawDepthToLinearDepth(float rawDepth)
{
    // See: https://forum.unity.com/threads/getting-scene-depth-z-buffer-of-the-orthographic-camera.601825/#post-4966334
    float persp = LinearEyeDepth(rawDepth, _ZBufferParams);
    float ortho = (_ZBufferParams.z - _ZBufferParams.y) * (1 - rawDepth) + _ZBufferParams.y;
    return lerp(persp, ortho, _OrthoParams.w);
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/perspective-correct-interpolation-vertex-attributes
float GetPerspectiveCorrectDepth(float depth0, float depth1, float lerpValue)
{
    // EQ: Z = 1 / [ 1/Z0 (1 - lerp) + 1/Z1 * lerp ]
    return rcp(lerp(rcp(depth0), rcp(depth1), lerpValue));
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/perspective-correct-interpolation-vertex-attributes
float GetPerspectiveCorrectDepth_Optimized(float invDepth0, float invDepth1, float lerpValue)
{
    // EQ: Z = 1 / [ 1/Z0 (1 - lerp) + 1/Z1 * lerp ]
    return rcp(lerp(invDepth0, invDepth1, lerpValue));
}

float Luminance(float3 v)
{
    return dot(v, float3(0.2126f, 0.7152f, 0.0722f));
}

float SCurve(float x)
{
    // (3x^2 - 2x^3)
    float xSqr = x * x;
    return (-2 * x + 3) * xSqr;
}

float3 GetNormalVSFromGBuffer(float4 gbufferTex)
{
    float3 normalVS;
    normalVS.xy = gbufferTex.xy * 2.f - 1.f;
    normalVS.z = -sqrt(1.f - dot(normalVS.xy, normalVS.xy)); // positive Z points away from the camera
    return normalVS;
}

uint2 GetPixelCoordFromNDC(float3 positionNDC)
{
    return floor((positionNDC.xy * 0.5f + 0.5f) * _ScreenParams.xy);
}

#endif