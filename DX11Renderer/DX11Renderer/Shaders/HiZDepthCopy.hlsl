// Copies mip0 from DepthStencil texture
#include "./Common.hlsli"

cbuffer HiZCreationCB : register(b4)
{
    uint2 resolutionSrc;
    uint2 resolutionDst;
};

Texture2D<float> CameraDepthStencil : register(t3);
RWTexture2D<float2> HiZOutput : register(u0);

[numthreads(16, 16, 1)]
void CSMain(uint3 tId : SV_DispatchThreadID)
{
    if (tId.x >= (uint) resolutionDst.x || tId.y >= (uint) resolutionDst.y)
        return;

    //float depth01 = Linear01Depth(CameraDepthStencil[tId.xy].r, _ZBufferParams);
    float depth01 = CameraDepthStencil[tId.xy].r;
    HiZOutput[tId.xy] = (float2)depth01; // store as both min and max
}