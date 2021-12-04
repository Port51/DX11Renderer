Texture2D<float4> tex : register(t0);
RWStructuredBuffer<float> BufferOut : register(u0);

[numthreads(64, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    BufferOut[DTid.x] = frac(DTid.x * 0.2) + tex.Load(int3(DTid.xy, 0)).r + tex2.Load(int3(DTid.xy, 0)).r;
}