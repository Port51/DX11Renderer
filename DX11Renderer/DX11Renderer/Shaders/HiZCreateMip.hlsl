// Creates a mip from previous mip

cbuffer HiZCreationCB : register(b4)
{
    uint2 resolutionSrc;
    uint2 resolutionDst;
    float4 _ZBufferParams;
};

RWTexture2D<unorm float2> HiZInput : register(u0);
RWTexture2D<unorm float2> HiZOutput : register(u1);

[numthreads(16, 16, 1)]
void CSMain(uint3 tId : SV_DispatchThreadID)
{
    if (tId.x >= (uint) resolutionDst.x || tId.y >= (uint) resolutionDst.y)
        return;

    uint2 srcId = tId.xy * 2u;

    // Don't allow overextending
    uint2 srcDispl = uint2(
        (uint) resolutionSrc.x > srcId.x + 1u ? 1u : 0u,
        (uint) resolutionSrc.y > srcId.y + 1u ? 1u : 0u);

    float2 d00 = HiZInput[srcId.xy].rg;
    float2 d10 = HiZInput[srcId.xy + uint2(srcDispl.x, 0)].rg;
    float2 d01 = HiZInput[srcId.xy + uint2(0, srcDispl.y)].rg;
    float2 d11 = HiZInput[srcId.xy + srcDispl.xy].rg;
    HiZOutput[tId.xy] = float2(
        min(d00.x, min(d10.x, min(d01.x, d11.x))),
        max(d00.y, max(d10.y, max(d01.y, d11.y)))
    );
    
}