// Copies mip0 from DepthStencil texture

cbuffer HiZCreationCB : register(b4)
{
    uint2 resolutionSrc;
    uint2 resolutionDst;
    float4 _ZBufferParams;
};

Texture2D<float> CameraDepthStencil : register(t3);
RWTexture2D<float2> HiZOutput : register(u0);

// Z buffer to linear 0..1 depth (0 at camera position, 1 at far plane).
// Does NOT work with orthographic projections.
// Does NOT correctly handle oblique view frustums.
// zBufferParam = { (f-n)/n, 1, (f-n)/n*f, 1/f }
float Linear01Depth(float depth, float4 zBufferParam)
{
    return 1.0 / (zBufferParam.x * depth + zBufferParam.y);
}

// Z buffer to linear depth.
// Does NOT correctly handle oblique view frustums.
// Does NOT work with orthographic projection.
// zBufferParam = { (f-n)/n, 1, (f-n)/n*f, 1/f }
float LinearEyeDepth(float depth, float4 zBufferParam)
{
    return 1.0 / (zBufferParam.z * depth + zBufferParam.w);
}

[numthreads(16, 16, 1)]
void CSMain(uint3 tId : SV_DispatchThreadID)
{
    if (tId.x >= (uint) resolutionDst.x || tId.y >= (uint) resolutionDst.y)
        return;

    float depth01 = Linear01Depth(CameraDepthStencil[tId.xy].r, _ZBufferParams);
    HiZOutput[tId.xy] = (float2)depth01; // store as both min and max
}