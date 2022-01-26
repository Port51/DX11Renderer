Texture2D<float4> NormalRoughRT : register(t0);
Texture2D<float4> GBuffer1RT : register(t1);
RWTexture2D<float4> SpecularLightingOut : register(u0);
RWTexture2D<float4> DiffuseLightingOut : register(u1);

[numthreads(8, 8, 1)]
void CSMain(uint3 tId : SV_DispatchThreadID)
{
    float4 normalRough = NormalRoughRT[tId.xy];
    float4 gbuff1 = GBuffer1RT[tId.xy];
    //ColorOut[tId.xy] = 1.f;
    //ColorOut[tId.xy] = NormalRoughRT[tId.xy].y;
    
    // todo: iterate through light list of tile and execute BRDF
    
    SpecularLightingOut[tId.xy] = normalRough.y;
    DiffuseLightingOut[tId.xy] = 0.1f;
}