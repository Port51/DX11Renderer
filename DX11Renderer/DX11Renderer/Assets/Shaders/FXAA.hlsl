// Algorithm notes:
// FXAA reduces contrast in areas with different luminance
// Should be applied after tonemapping and color grading
// Previous pass should have stored luminance in alpha channel

// References:
//  - https://iryoku.com/aacourse/downloads/09-FXAA-3.11-in-15-Slides.pdf
//  - http://blog.simonrodriguez.fr/articles/2016/07/implementing_fxaa.html

//#define PREVIOUS_PASS_STORED_ALPHA

// Debug views:
#define DEBUG_VIEW_CONTRAST

#include "./Common.hlsli"

#if defined(PREVIOUS_PASS_STORED_ALPHA)
    // Previous pass helpfully stored luminance into alpha channel
    #define SAMPLE_LUM(c) c.a
#else
    // If there was no previous pass, estimate luminance by green channel
    #define SAMPLE_LUM(c) c.g
#endif

#define CLAMP_UV_FLT(uv) clamp(uv, (float2)0.f, (float2)_ScreenParams.xy)
#define CLAMP_UV_INT(uv) clamp(uv, (int2)0, (int2)_ScreenParams.xy)

Texture2D<float4> CameraColorIn : register(t3);
RWTexture2D<float4> CameraColorOut : register(u0);

cbuffer FXAA_CB : register(b4)
{
    float _MinThreshold;
    float _MaxThreshold;
    float _EdgeSharpness;
    float padding;
};

[numthreads(16, 16, 1)]
void CSMain(uint3 tId : SV_DispatchThreadID)
{
    int2 res = (int2)_ScreenParams.xy;
    if (tId.x >= (uint)res.x || tId.y >= (uint)res.y)
        return;
    
    int2 id = (int2)tId.xy;
    int2 idDec = max((int2)0, id - 1);
    int2 idInc = min(res - (int2) 1, id + 1);
    const float2 uv = (float2) id * _ScreenParams.zw;
    
    float lCenter = SAMPLE_LUM(CameraColorIn[id]);
    
    // Sample diagonal neighbors in CCW order
    float lur = SAMPLE_LUM(CameraColorIn[uint2(idInc.x, idInc.y)]) + 1.f / 384.f; // to avoid problems with isolated, bright pixels
    float lul = SAMPLE_LUM(CameraColorIn[uint2(idDec.x, idInc.y)]);
    float ldl = SAMPLE_LUM(CameraColorIn[uint2(idDec.x, idDec.y)]);
    float ldr = SAMPLE_LUM(CameraColorIn[uint2(idInc.x, idDec.y)]);
    
    float lMaxRight = max(lur, ldr);
    float lMinRight = min(lur, ldr);
    float lMaxLeft = max(lul, ldl);
    float lMinLeft = min(lul, ldl);
    
    float lMax = max(lMaxRight, lMaxLeft);
    float lMin = max(lMinRight, lMinLeft);
    
    float lMaxScaled = lMax * _MaxThreshold;
    float lMaxScaledClamped = max(lMaxScaled, _MinThreshold);
    
    float lMaxM = max(lMax, lCenter);
    float lMinM = min(lMin, lCenter);
    float lRange = lMaxM - lMinM;
    
    float gradient0 = ldl - lur;
    float gradient1 = ldr - lul;
    
    [branch]
    if (lRange < lMaxScaledClamped)
    {
        // Early out
        CameraColorOut[id] = CameraColorIn.SampleLevel(BilinearWrapSampler, uv, 0.f);
        return;
    }
    
    float2 dir = float2(
        gradient0 + gradient1,
        gradient0 - gradient1
    );
    float2 dir1 = normalize(dir);
    float4 sN1 = CameraColorIn.SampleLevel(BilinearWrapSampler, uv - dir1 * _ScreenParams.zw, 0.f);
    float4 sP1 = CameraColorIn.SampleLevel(BilinearWrapSampler, uv + dir1 * _ScreenParams.zw, 0.f);
    
    const float _EdgeSharpness = 0.5f;
    float dirAbsMinTimesC = min(abs(dir1.x), abs(dir1.y)) * _EdgeSharpness;
    float2 dir2 = clamp(dir1.xy / dirAbsMinTimesC, -2.f, 2.f);
    
    float4 sN2 = CameraColorIn.SampleLevel(BilinearWrapSampler, uv - dir2 * _ScreenParams.zw, 0.f);
    float4 sP2 = CameraColorIn.SampleLevel(BilinearWrapSampler, uv + dir2 * _ScreenParams.zw, 0.f);
    
    float4 s1 = sN1 + sP1;
    float4 s2 = ((sN2 + sP2) * 0.25f) + (s1 * 0.25f);
    bool twoTap = (SAMPLE_LUM(s2) < lMin) || (SAMPLE_LUM(s2) > lMax);
    if (twoTap)
    {
        s2.rgb = s1.rgb * 0.5f;
    }
    CameraColorOut[id] = s2;
}