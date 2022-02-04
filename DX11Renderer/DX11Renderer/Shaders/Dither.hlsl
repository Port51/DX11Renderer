#include "CbufCommon.hlsli"

Texture2D<float4> CameraColorIn : register(t3);
Texture2D<float> DitherTex : register(t4);
RWTexture2D<float4> CameraColorOut : register(u0);

cbuffer Dither_CB : register(b4)
{
    float _ShadowDither;
    float _MidDither;
    float2 padding;
};

[numthreads(16, 16, 1)]
void CSMain(uint3 tId : SV_DispatchThreadID)
{
    int2 res = (int2) _ScreenParams.xy;
    if (tId.x >= (uint) res.x || tId.y >= (uint) res.y)
        return;
    
    float4 c = CameraColorIn[tId.xy];
    float lum = saturate(dot(float3(0.2125, 0.7154, 0.0721), c.rgb)); // maybe pre-calculate luminance in alpha?
    
    uint2 ditherRes;
    DitherTex.GetDimensions(ditherRes.x, ditherRes.y);
    
    float ditherMult = lerp(lerp(_ShadowDither, _MidDither, lum), 0.f, saturate(lum * lum * 1.2f));
    float dither = DitherTex[tId.xy % ditherRes].r * ditherMult + 1.f;
    c.rgb = saturate(c.rgb * dither);
    CameraColorOut[tId.xy] = c;
}