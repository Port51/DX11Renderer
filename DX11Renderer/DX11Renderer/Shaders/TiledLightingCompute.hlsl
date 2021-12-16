#include "./CbufCommon.hlsli"
#include "./Lighting/LightStructs.hlsli"
#include "./Lighting/Lights.hlsli"
#include "./Lighting/BRDF.hlsli"

// References:
// https://wickedengine.net/2018/01/10/optimizing-tile-based-light-culling/

#define MAX_TILE_LIGHTS 64
#define TILED_GROUP_SIZE 16

#define USE_FRUSTUM_INTERSECTION_TEST
#define USE_AABB_INTERSECTION_TEST

#define DEBUG_GRID
#define DEBUG_LIGHT_RANGES

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
groupshared uint minTileZ;
groupshared uint maxTileZ;

/*float ConvertZToLinearDepth(float depth)
{
    float near = camera_near;
    float far = camera_far;
    
    return (near * far) / (far - depth * (far - near));

}

float3 GetPositionVS(float2 texcoord, float depth)
{
    float4 clipSpaceLocation;
    clipSpaceLocation.xy = texcoord * 2.0f - 1.0f;
    clipSpaceLocation.y *= -1;
    clipSpaceLocation.z = depth;
    clipSpaceLocation.w = 1.0f;
    float4 homogenousLocation = mul(clipSpaceLocation, inverse_projection);
    return homogenousLocation.xyz / homogenousLocation.w;
}*/

// Z buffer to linear depth.
// Does NOT correctly handle oblique view frustums.
// Does NOT work with orthographic projection.
// zBufferParam = { (f-n)/n, 1, (f-n)/n*f, 1/f }
float LinearEyeDepth(float depth, float4 zBufferParam)
{
    return 1.0 / (zBufferParam.z * depth + zBufferParam.w);
}

float RawDepthToLinearDepth(float rawDepth)
{
    // See: https://forum.unity.com/threads/getting-scene-depth-z-buffer-of-the-orthographic-camera.601825/#post-4966334
    float persp = LinearEyeDepth(rawDepth, _ZBufferParams);
    float ortho = (_ZBufferParams.z - _ZBufferParams.y) * (1 - rawDepth) + _ZBufferParams.y;
    return lerp(persp, ortho, _OrthoParams.w);
}

bool AABBSphereIntersection(float3 spherePos, float sphereRad, float3 aabbCenter, float3 aabbExtents)
{
    float3 displ = max(0, abs(aabbCenter - spherePos) - aabbExtents);
    float sdfSqr = dot(displ, displ);
    return sdfSqr <= sphereRad * sphereRad;
}

[numthreads(TILED_GROUP_SIZE, TILED_GROUP_SIZE, 1)]
void CSMain(uint3 gId : SV_GroupID, uint gIndex : SV_GroupIndex, uint3 groupThreadId : SV_GroupThreadID, uint3 tId : SV_DispatchThreadID)
{
    //
    // Cull lights
    //
    
    if (gIndex == 0)
    {
        tileLightCount = 0;
        minTileZ = asuint(10000.0);
        maxTileZ = asuint(0.0);
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    // todo: replace w/ Hi-Z buffer, or do depth slices here
    float rawDepth = DepthRT.Load(int3(tId.xy, 0));
    float linearDepth = RawDepthToLinearDepth(rawDepth);
    uint intDepth = asuint(linearDepth);
    InterlockedMin(minTileZ, intDepth);
    InterlockedMax(maxTileZ, intDepth);
    
    GroupMemoryBarrierWithGroupSync();
    
    const float2 tileDepthRange = float2(asfloat(minTileZ), asfloat(maxTileZ));
    
    // todo: use gId to setup frustums
    // todo: early out if only sky
    
    float debugValue = 0;
    
    // Calculate NDC for tile corners
    const float4 planeNDC = ((float4) (gId.xyxy * TILED_GROUP_SIZE + float4(TILED_GROUP_SIZE, 0, 0, TILED_GROUP_SIZE)) * _ScreenParams.zwzw) * 2.0 - 1.0;

#if defined(USE_FRUSTUM_INTERSECTION_TEST)
    // Derive frustum planes
    // #0 faces to the right, and the rest continue in CCW fashion
    float3 frustumPlanes[4];
    
    // Use cross product to turn tile view directions into plane directions
    // The cross product is done by flipping X or Y with Z
    frustumPlanes[0] = normalize(float3(1, 0, -planeNDC.x * _FrustumCornerDataVS.x));
    frustumPlanes[1] = normalize(float3(0, 1, -planeNDC.y * _FrustumCornerDataVS.y));
    frustumPlanes[2] = normalize(float3(-1, 0, planeNDC.z * _FrustumCornerDataVS.x));
    frustumPlanes[3] = normalize(float3(0, -1, planeNDC.w * _FrustumCornerDataVS.y));
#endif
    
#if defined(USE_AABB_INTERSECTION_TEST)
    // Note: Use min depth for corners in order to construct AABB that encapsulates entire frustum
    const float3 minAABB = float3(planeNDC.zy * _FrustumCornerDataVS.xy * tileDepthRange.x, tileDepthRange.x);
    const float3 maxAABB = float3(planeNDC.xw * _FrustumCornerDataVS.xy * tileDepthRange.x, tileDepthRange.y);
    const float3 aabbCenter = (minAABB + maxAABB) * 0.5;
    const float3 aabbExtents = (maxAABB - minAABB) * 0.5;
#endif
    
    for (uint i = gIndex; i < _VisibleLightCount; i += TILED_GROUP_SIZE * TILED_GROUP_SIZE)
    {
        StructuredLight light = lights[i];
        
        bool inFrustum = true;
        
#if defined(USE_FRUSTUM_INTERSECTION_TEST)        
        [unroll]
        for (uint j = 0; j < 4; ++j)
        {
            float d = dot(frustumPlanes[j], light.positionVS_range.xyz);
            inFrustum = inFrustum && (d < light.positionVS_range.w);
        }
        // Not needed due to AABB test:
        //inFrustum = inFrustum && (light.positionVS_range.z >= tileDepthRange.x - light.positionVS_range.w);
        //inFrustum = inFrustum && (light.positionVS_range.z <= tileDepthRange.y + light.positionVS_range.w);
#endif

#if defined(USE_AABB_INTERSECTION_TEST)        
        // Sphere-AABB test
        inFrustum = inFrustum && AABBSphereIntersection(light.positionVS_range.xyz, light.positionVS_range.w, aabbCenter, aabbExtents);
#endif
        
        [branch]
        if (inFrustum)
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
    float2 screenUV = tId.xy * _ScreenParams.zw;
    float2 positionNDC = screenUV * 2.0 - 1.0;
    
    float3 frustumPlaneVS = float3(positionNDC * _FrustumCornerDataVS.xy, 1);
    float3 positionVS = frustumPlaneVS * linearDepth;
    float3 viewDirVS = normalize(positionVS);
    
    float4 normalRough = NormalRoughRT[tId.xy];
    //float4 gbuff1 = GBuffer1RT[tId.xy];
    float3 normalVS = normalize(normalRough.xyz * 2.0 - 1.0);
    float linearRoughness = normalRough.w;
    float roughness = linearRoughness * linearRoughness;
    
    // f0 = fresnel reflectance at normal incidence
    // f90 = fresnel reflectance at grazing angles (usually 1)
    //float metalness = 1;
    //float3 reflectance = lerp(0.04, 1.0, metalness);
    float3 f0 = 1.0; //0.16 * reflectance * reflectance;
    
    // note: Filament uses f0 of 0.0-0.02 to indicate pre-baked specular, and turn off f90 in that case
    // For now, just set it to 1.0
    float f90 = 1.0;
    
    float3 diffuseLight = 0;
    float3 specularLight = 0;
    for (uint i = 0; i < tileLightCount; ++i)
    {
        StructuredLight light = lights[tileLightIndices[i]];
        
	    // Simple lambert
        float3 displ = light.positionVS_range.xyz - positionVS.xyz;
        float lightDist = length(displ);
        float3 lightDirVS = displ / max(lightDist, 0.0001);
        
        float3 dir = normalize(displ);
        float NdotL = dot(normalVS, dir);
        
        float lightRSqr = 1 * 1; // temporary...
        float lightAtten = GetSphericalLightAttenuation(lightDist, light.data0.y, light.positionVS_range.w);
        lightAtten *= light.color_intensity.w;
        
        float3 lightColorInput = light.color_intensity.rgb * lightAtten;
        lightColorInput = lightAtten;
        
        BRDFLighting brdf = BRDF(f0, f90, roughness, linearRoughness, normalVS, viewDirVS, lightDirVS);
        diffuseLight += brdf.diffuseLight * lightColorInput;
        specularLight += brdf.specularLight * lightColorInput;
    }
    
    diffuseLight *= (rawDepth < 1);
    specularLight *= (rawDepth < 1);
    
    // todo: iterate through light list of tile and execute BRDF
    float3 debugColor = debugValue;// * (rawDepth < 1);
    
#if defined(DEBUG_LIGHT_RANGES)
    debugColor = float3(diffuseLight.r * (rawDepth < 1), tileLightCount * 0.2, 0);
#endif
    
#if defined(DEBUG_GRID)
    if (groupThreadId.x == 0 || groupThreadId.y == 0)
    {
        debugColor = float3(0.3, 0.1, 0.8);
    }
#endif
    //debugColor = saturate(asfloat(maxTileZ) - asfloat(minTileZ));
    //debugColor = asfloat(minTileZ) / 20.0;
    
    SpecularLightingOut[tId.xy] = float4(specularLight, 1);
    DiffuseLightingOut[tId.xy] = float4(diffuseLight, 1);
    //DebugOut[tId.xy] = float4(lerp(float3(1, 1, 1), float3(1, 0, 0), tileLightCount * 0.333) * (rawDepth < 1), 1);
    DebugOut[tId.xy] = float4(debugColor, 1);
    //DebugOut[tId.xy] = float4(diffuseLight, 1);
    //DebugOut[tId.xy] = sides[0].y;
    //DebugOut[tId.xy] = positionVS.x > 0;
    //DebugOut[tId.xy] = rawDepth < 1;
    //DebugOut[tId.xy] = rawDepth;
    //DebugOut[tId.xy] = viewDirVS.y;

}