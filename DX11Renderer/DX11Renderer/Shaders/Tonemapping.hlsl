// References:
// - https://graphics-programming.org/resources/tonemapping/index.html

#include "CbufCommon.hlsli"

Texture2D<float4> CameraColorIn : register(t3);
RWTexture2D<float4> CameraColorOut : register(u0);

cbuffer TonemappingCB : register(b4)
{
    float _WhitePoint;
    float3 padding;
};

float Luminance(float3 v)
{
    return dot(v, float3(0.2126f, 0.7152f, 0.0722f));
}

float3 ReinhardExtendedLuminance(float3 c, float maxWhiteLuminance)
{
    float lum = Luminance(c);
    float numerator = lum * (1.0f + (lum / (maxWhiteLuminance * maxWhiteLuminance)));
    float newLum = numerator / (1.0f + lum);
    return c * (newLum / max(lum, 0.001));
}

[numthreads(16, 16, 1)]
void CSMain(uint3 tId : SV_DispatchThreadID)
{
    int2 res = (int2) _ScreenParams.xy;
    if (tId.x >= (uint) res.x || tId.y >= (uint) res.y)
        return;
    
    float4 c = CameraColorIn[tId.xy];
    
    c.rgb = ReinhardExtendedLuminance(c.rgb, 0.9f);
    
    CameraColorOut[tId.xy] = saturate(c);
}