Texture2D<float4> tex : register(t0);
RWStructuredBuffer<float> BufferOut : register(u0);
RWTexture2D<float4> RTOut : register(u1);

[numthreads(8, 8, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    BufferOut[DTid.x] = frac(DTid.x * 0.2);// + tex.Load(int3(DTid.xy, 0)).r;
    //RTOut[DTid.xy] = 1.f - RTOut[DTid.xy];
    //RTOut[DTid.xy] = float4(0.f, 1.f, 0.f, 0.f);
}