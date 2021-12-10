#include "./Lighting/LightStructs.hlsli"

StructuredBuffer<StructuredLight> lights : register(t0); // somehow this is a texture!?
Texture2D<float4> NormalRoughRT : register(t1);
Texture2D<float4> GBuffer1RT : register(t2);
RWTexture2D<float4> SpecularLightingOut : register(u0);
RWTexture2D<float4> DiffuseLightingOut : register(u1);

[numthreads(8, 8, 1)]
void CSMain(uint3 tId : SV_DispatchThreadID)
{
    uint numLights, lightStride;
    lights.GetDimensions(numLights, lightStride);
    
    float vv = 0;
    for (uint i = 0; i < 3; ++i)
    {
        vv += lights[i].positionVS.x;
    }
    
    float4 normalRough = NormalRoughRT[tId.xy];
    float4 gbuff1 = GBuffer1RT[tId.xy];
    //ColorOut[tId.xy] = 1.f;
    //ColorOut[tId.xy] = NormalRoughRT[tId.xy].y;
    
    // todo: iterate through light list of tile and execute BRDF
    
    SpecularLightingOut[tId.xy] = normalRough.y * 0.01 + gbuff1.y * 0.01 + vv * 0.01;
    DiffuseLightingOut[tId.xy] = 0.1f * 0;
}