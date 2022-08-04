#include "./Common.hlsli"

Texture2D<float> DepthRT : register(t3);
RWTexture2D<float4> CameraColorOut : register(u0);

[numthreads(16, 16, 1)]
void CSMain(uint3 tId : SV_DispatchThreadID)
{
    int2 res = (int2)_ScreenParams.xy;
    if (tId.x >= (uint)res.x || tId.y >= (uint)res.y)
        return;

    if (DepthRT[tId.xy] == 1.0)
    {
        const float2 screenUV = tId.xy * _ScreenParams.zw;
        const float2 positionNDC = screenUV * 2.f - 1.f;

        const float3 frustumPlaneVS = float3(positionNDC * _FrustumCornerDataVS.xy, 1.f);
        const float3 viewDirVS = normalize(frustumPlaneVS);
        const float3 viewDirWS = mul(_InvViewMatrix, float4(viewDirVS.xyz, 0.f)).xyz;

        float lv = saturate(viewDirWS.y + 0.15);
        //lv = pow(lv, 0.5);
        CameraColorOut[tId.xy] = lerp(float4(0.05, 0.03, 0.4, 1), float4(0.55, 0.75, 0.8, 1), lv) * 0.75;
    }
}