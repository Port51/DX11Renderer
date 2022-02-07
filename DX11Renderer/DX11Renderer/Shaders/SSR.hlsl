// References:
//  - https://www.jpgrenier.org/ssr.html
//  - https://sakibsaikia.github.io/graphics/2016/12/26/Screen-Space-Reflection-in-Killing-Floor-2.html

#include "./Common.hlsli"

#define MAX_MIP                 7u
#define MAX_TRACE_ITERATIONS    20u

#define DEBUG_VIEW_SHOW_TRACE
#define DEBUG_TRACE_START_X     450u
#define DEBUG_TRACE_START_Y     480u

Texture2D<float4> NormalRoughReflectivityRT : register(t2);
Texture2D<float4> CameraColorIn : register(t3);
Texture2D<float> DepthRT : register(t4);
Texture2D<float2> HiZBuffer : register(t5);

RWTexture2D<float4> CameraColorOut : register(u0);
globallycoherent RWStructuredBuffer<uint> DebugData : register(u1);

groupshared uint2 debugTraceId;

cbuffer SSR_CB : register(b4)
{
    uint _DebugViewStep;
    uint3 padding;
};

float3 GetReflectionDirSS(float3 positionVS, float3 viewDirVS, float2 uv, float rawDepth, float3 normalVS)
{
    const float3 positionSS = float3(uv.xy, rawDepth);
    const float3 reflectVS = reflect(viewDirVS, normalVS);
    
    const float Offset = 0.5f;
    const float3 offsetPositionVS = positionVS + reflectVS * Offset;
    float4 offsetPositionSS = mul(_ProjMatrix, float4(offsetPositionVS, 0.f));
    offsetPositionSS.xyz /= offsetPositionSS.w;
    offsetPositionSS.xy = (offsetPositionSS.xy * float2(0.5f, -0.5f) + 0.5f) * _ScreenParams.xy;

    return (offsetPositionSS.xyz - positionSS);
}

[numthreads(16, 16, 1)]
void CSMain(uint3 tId : SV_DispatchThreadID)
{
    //if (tId.x >= (uint) _ScreenParams.x || tId.y >= (uint) _ScreenParams.y)
    //    return;
    
    //
    // Setup debug views
    //
    const bool isDebugTrace = (tId.x == DEBUG_TRACE_START_X && tId.y == DEBUG_TRACE_START_Y);
    
    //
    // Get surface info
    //
    const float rawDepth = DepthRT.Load(int3(tId.xy, 0));
    const float isGeometry = rawDepth < 1.f;
    const float linearDepth = RawDepthToLinearDepth(rawDepth);
    
    //const float linearDepth = HiZBuffer.Load(int3(tId.xy, 0)).r * _ProjectionParams.z;
    const float2 screenUV = tId.xy * _ScreenParams.zw;
    const float2 positionNDC = screenUV * 2.f - 1.f;
    
    const float3 frustumPlaneVS = float3(positionNDC * _FrustumCornerDataVS.xy, 1.f);
    const float3 positionVS = frustumPlaneVS * linearDepth;
    const float3 viewDirVS = normalize(positionVS);
    
    const float4 gbufferTex = NormalRoughReflectivityRT[tId.xy];
    const float3 normalVS = GetNormalVSFromGBuffer(gbufferTex);
    
    const float linearRoughness = gbufferTex.z;
    const float roughness = linearRoughness * linearRoughness;
    
    //
    // Calculate reflection
    //
    const float3 reflectDirSS = GetReflectionDirSS(positionVS, viewDirVS, tId.xy, rawDepth, normalVS);
    const float3 invDir = 1.f / reflectDirSS;
    const float2 halfSignDir = sign(reflectDirSS.xy) * 0.5f;
    
    float3 uv = float3(tId.xy + 0.5f, rawDepth - 0.01f); // start from center of pixel, with small offset
    float4 reflectColor = 0.f;
    uint iter = 0u;
    uint mip = 3u;
    
    [branch]
    if (reflectDirSS.z > 0.f)
    {
        while (iter < MAX_TRACE_ITERATIONS)
        {
            uint2 ptId = (uint2)floor(uv.xy);
            uint2 tileId = ptId >> mip;
            uint tilePixelSize = 1u << mip;
        
#if defined(DEBUG_VIEW_SHOW_TRACE)
            if (isDebugTrace && iter == _DebugViewStep)
            {
                DebugData[0] = ptId.x;
                DebugData[1] = ptId.y;
                DebugData[2] = mip;
            }
#endif
        
            // Setup planes
            float2 planesUV = ((tileId + 0.5f) * tilePixelSize + tilePixelSize * halfSignDir);
        
            // Calculate nearest intersection
            float2 intersectionSolutions = (planesUV - uv.xy) * invDir.xy;
            float intersectDist = min(intersectionSolutions.x, intersectionSolutions.y);
            float3 intersectUV;
        
            float tileIntersectDist;
            float2 tileIntersectOffset;
            if (intersectionSolutions.x < intersectionSolutions.y)
            {
                tileIntersectDist = intersectionSolutions.x;
                tileIntersectOffset = float2(halfSignDir.x, 0.f); // 1/2 pixel offset
            }
            else
            {
                tileIntersectDist = intersectionSolutions.y;
                tileIntersectOffset = float2(0.f, halfSignDir.y); // 1/2 pixel offset
            }
            
            // Calculate depth plane intersection
            float mipDepthPlane = HiZBuffer.Load(int3(tileId.xy, mip)).x;
            float mipDepthDispl = (mipDepthPlane - uv.z);
            float depthPlaneSolution = mipDepthDispl * invDir.z;
        
            // Test depth ranges
            if (depthPlaneSolution < tileIntersectDist)
            {
                if (mip == 0u)
                {
                    reflectColor.rgb = CameraColorIn[ptId].rgb;
                    reflectColor.a = 1.f;
                    break;
                }
                else
                {
                    --mip;
                    uv = reflectDirSS * depthPlaneSolution + uv;
                }
            }
            else
            {
                // Move to tile intersection and increase mip
                uv = reflectDirSS * tileIntersectDist + uv + float3(tileIntersectOffset.xy, 0.f);
                mip = min(mip + 1u, MAX_MIP);
            }
        
            ++iter;
        }
    }
    
    // todo: use fallback when reflectColor.a = 0
    
    float4 colorIn = CameraColorIn[tId.xy];
    CameraColorOut[tId.xy] = float4(colorIn.rgb + reflectColor.rgb * reflectColor.a, colorIn.a);
    
    //
    // Debug views!
    //    
#if defined(DEBUG_VIEW_SHOW_TRACE)
    // Show trace of current point
    
    AllMemoryBarrierWithGroupSync();
    
    float3 debugColor = saturate(CameraColorOut[tId.xy].rgb);
    
    if (isDebugTrace)
    {
        // Show start point
        debugColor = float3(1, 1, 0);
    }
    
    uint2 mipMod = tId.xy % (1u << DebugData[2]);
    if (tId.x == DebugData[0] && tId.y == DebugData[1])
    {
        debugColor = float3(0, 1, 0);
    }
    else if (tId.x == DebugData[0] || tId.y == DebugData[1])
    {
        debugColor = lerp(debugColor.rgb, float3(1, 0, 0), 0.8f);
    }
    else if (DebugData[2] > 0u && any(mipMod == 0u))
    {
        // Tiles
        debugColor = lerp(debugColor, float3(0, 1, 0), 0.15f);

    }
    
    CameraColorOut[tId.xy] = float4(debugColor.rgb, colorIn.a);
#endif
    
}