#include "./CbufCommon.hlsli"
#include "./Lighting/LightStructs.hlsli"

#define MAX_TILE_LIGHTS 256
#define TILED_GROUP_SIZE 16

StructuredBuffer<StructuredLight> lights : register(t0);
Texture2D<float4> NormalRoughRT : register(t1);
//Texture2D<float4> GBuffer1RT : register(t2);
Texture2D<float> DepthRT : register(t2);
StructuredBuffer<float> debug : register(t7);
RWTexture2D<float4> SpecularLightingOut : register(u0);
RWTexture2D<float4> DiffuseLightingOut : register(u1);
RWTexture2D<float4> DebugOut : register(u2);

groupshared uint tileLightCount;
groupshared uint tileLightIndices[MAX_TILE_LIGHTS];

[numthreads(16, 16, 1)]
void CSMain(uint3 gId : SV_GroupID, uint gIndex : SV_GroupIndex, uint3 groupThreadId : SV_GroupThreadID, uint3 tId : SV_DispatchThreadID)
{
    //
    // Cull lights
    //
    
    if (gIndex == 0)
    {
        tileLightCount = 0;
    }
    GroupMemoryBarrierWithGroupSync();
    
    // todo: use gId to setup frustums
    
    //for (uint i = gIndex * TILED_GROUP_SIZE; i < gIndex * TILED_GROUP_SIZE + TILED_GROUP_SIZE; ++i)
    for (uint i = gIndex; i < _VisibleLightCount; i += TILED_GROUP_SIZE * TILED_GROUP_SIZE)
    //for (uint i = 0; i < 3; ++i)
    {
        StructuredLight light = lights[i];
        
        bool isVisible = true;

        [branch]
        if (isVisible)
        {
            uint idx;
            InterlockedAdd(tileLightCount, 1u, idx);
            tileLightIndices[idx] = i;
        }
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    //
    // Apply lighting
    //
    
    float2 uv = tId.xy / _ScreenParams.xy;
    
    //float depth = depthTx.Load(int3(dispatchThreadId.xy, 0));
    //float view_depth = ConvertZToLinearDepth(depth);
    
    //float3 positionVS = GetPositionVS(uv, depth);
    //float3 viewDirVS = normalize(positionVS);
    float4 normalRough = NormalRoughRT[tId.xy];
    //float4 gbuff1 = GBuffer1RT[tId.xy];
    float depth = DepthRT.Load(int3(tId.xy, 0));
    float3 normalVS = normalRough.xyz * 2.0 - 1.0;
    
    float vv = 0;
    for (uint i = 0; i < tileLightCount; ++i)
    {
        StructuredLight light = lights[tileLightIndices[i]];
        //StructuredLight light = lights[0];
        /*
	    // fragment to light vector data
	    const float distToL = length(vToL);
	    const float3 dirToL = vToL / distToL;
	    // attenuation
        const float att = SCurve(1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL)));
	    // diffuse intensity
	    const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, i.normalVS));
	    // reflected light vector
        const float3 w = i.normalVS * dot(vToL, i.normalVS);
	    const float3 r = w * 2.0f - vToL;
	    // calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
	    const float3 specular = att * (diffuseColor * diffuseIntensity) * specularIntensity * pow(max(0.0f, dot(normalize(-r), normalize(i.positionVS))), specularPower);
	    // final color
	    //return float4(saturate((diffuse + ambient + specular) * materialColor), 1.0f);
        */
        vv += light.positionVS_Range.x;
    }
    //ColorOut[tId.xy] = 1.f;
    //ColorOut[tId.xy] = NormalRoughRT[tId.xy].y;
    
    // todo: iterate through light list of tile and execute BRDF
    
    SpecularLightingOut[tId.xy] = normalRough.y * 0.01 + debug[0] * 0.01 + vv * 0.01;
    DiffuseLightingOut[tId.xy] = 0.1f * 0;
    DebugOut[tId.xy] = depth;

}