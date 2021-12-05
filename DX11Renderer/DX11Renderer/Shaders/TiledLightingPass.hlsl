Texture2D<float4> NormalRoughRT : register(t0);
Texture2D<float4> GBuffer1RT : register(t1);
RWTexture2D<float4> ColorOut : register(u0);

[numthreads(8, 8, 1)]
void CSMain(uint3 tId : SV_DispatchThreadID)
{
    //ColorOut[tId.xy] = 1.f;
    ColorOut[tId.xy] = NormalRoughRT[tId.xy].y;
}