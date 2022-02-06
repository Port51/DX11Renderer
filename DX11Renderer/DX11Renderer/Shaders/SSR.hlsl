// References:
//  - https://www.jpgrenier.org/ssr.html
//  - https://sakibsaikia.github.io/graphics/2016/12/26/Screen-Space-Reflection-in-Killing-Floor-2.html

#include "./Common.hlsli"

#define MAX_MIP 7u
#define DEBUG_TRACE_START_X 450u
#define DEBUG_TRACE_START_Y 480u

Texture2D<float4> NormalRoughRT : register(t2);
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
    float3 positionSS = float3(uv.xy, rawDepth);
    float3 reflectVS = reflect(viewDirVS, normalVS);
    
    const float Offset = 0.1f;
    float3 offsetPositionVS = positionVS + reflectVS * Offset;
    float4 offsetPositionSS = mul(_ProjMatrix, float4(offsetPositionVS, 0.f));
    offsetPositionSS.xyz /= offsetPositionSS.w;
    offsetPositionSS.xy = offsetPositionSS.xy * 0.5f + 0.5f;

    return normalize(offsetPositionSS.xyz - positionSS);
}

[numthreads(16, 16, 1)]
void CSMain(uint3 tId : SV_DispatchThreadID)
{
    //if (tId.x >= (uint) _ScreenParams.x || tId.y >= (uint) _ScreenParams.y)
    //    return;
    
    //
    // Setup debug views
    //
    bool isDebugTrace = (tId.x == DEBUG_TRACE_START_X && tId.y == DEBUG_TRACE_START_Y);
    
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
    
    const float4 normalRough = NormalRoughRT[tId.xy];
    const float3 normalVS = normalize(normalRough.xyz * 2.f - 1.f);
    const float linearRoughness = normalRough.w;
    const float roughness = linearRoughness * linearRoughness;
    
    //
    // Calculate reflection
    //
    float3 reflectDirSS = GetReflectionDirSS(positionVS, viewDirVS, screenUV, rawDepth, normalVS);
    uint mip = 3u;
    
    float3 reflectColor = 0;
    float3 uv = float3(screenUV + 0.5f * _ScreenParams.zw, rawDepth - 0.01f); // start from center of pixel
    //float3 dir = float3(reflectDirVS.xy / _FrustumCornerDataVS.xy * 0.5f, reflectDirVS.z); // mix of UV space and VS
    float3 invDir = 1.f / reflectDirSS;
    float2 halfSignDir = sign(reflectDirSS.xy) * 0.5f;
    uint iter = 0u;
    
    while (iter < 10u)
    {
        float2 pixelUV = uv.xy * _ScreenParams.xy;
        uint2 ptId = (uint2)pixelUV;
        uint2 tile = ptId >> mip;
        uint tilePixelSize = 1u << mip;
        
        if (isDebugTrace && iter == _DebugViewStep)
        {
            DebugData[0] = ptId.x;
            DebugData[1] = ptId.y;
            DebugData[2] = mip;
        }
        
        // Setup planes
        float2 planesUV = ((tile + 0.5f) * tilePixelSize + tilePixelSize * halfSignDir) * _ScreenParams.zw;
        
        // Local pos in tile
        //float2 tilePos = pixelUV - tile * tilePixelSize;
        
        // Calculate nearest intersection
        float2 intersectionSolutions = (planesUV - uv.xy) * invDir.xy;
        float intersectDist = min(intersectionSolutions.x, intersectionSolutions.y);
        float3 intersectUV;
        
        float tileIntersectDist;
        float2 tileIntersectOffset;
        if (intersectionSolutions.x < intersectionSolutions.y)
        {
            tileIntersectDist = intersectionSolutions.x;
            tileIntersectOffset = float2(halfSignDir.x * _ScreenParams.z, 0.f); // 1/2 pixel offset
        }
        else
        {
            tileIntersectDist = intersectionSolutions.y;
            tileIntersectOffset = float2(0.f, halfSignDir.y * _ScreenParams.w); // 1/2 pixel offset
        }
        
        
        // Calculate depth plane intersection
        float mipDepthPlane = HiZBuffer.Load(int3(tile.xy, mip)).x;
        float mipDepthDispl = mipDepthPlane - uv.z;
        float depthPlaneSolution = mipDepthDispl / reflectDirSS.z;
        
        // Test depth ranges
        if (depthPlaneSolution < tileIntersectDist)
        {
            if (mip == 0u)
            {
                reflectColor = CameraColorIn[(uint2) ptId].rgb;
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
        
        // Choose movement
        //uv = intersectUV;
        //mip = min(mip + 1u, MAX_MIP);
        
        
        //uv.xy += _ScreenParams.zw;
        
        // Get position inside tile
        /*float2 pixelXY = uv.xy * _ScreenParams.xy;
        float2 tileSize = mip;
        uint2 tile = (uint2)floor(pixelXY) >> mip;
        
        // Find intersection pt of next tile
        float2 planes = (tile + 0.5f) * tileSize + tileSize * halfSignDir;
        float2 solutions = (planes - uv.xy) / dir.xy;
        float3 tileIntersection = uv + dir * min(solutions.x, solutions.y);
        
        // Get my depth range (current depth and intersection depth)
        float2 rayDepthRange = float2(min(uv.z, tileIntersection.z), max(uv.z, tileIntersection.z));
        
        // Get mip depth
        float2 mipDepthRange = HiZBuffer.Load(int3(tile.xy, mip)).xy;
        
        // Test depth ranges
        if (rayDepthRange.y > mipDepthRange.x && rayDepthRange.x < mipDepthRange.y)
        {
            // Intersection!
            if (mip == 0u)
            {
                reflectColor = CameraColorIn[(uint2)pixelXY].rgb;
                break;
            }
            else
            {
                --mip;
            }
        }
        else
        {
            // Move to tile intersection and increase mip
            uv = tileIntersection;
            mip = min(mip + 1u, MAX_MIP);
        }*/
        
        ++iter;
    }
    
    float4 colorIn = CameraColorIn[tId.xy];
    //reflectColor *= colorIn.rgb;
    
    //reflectColor = abs(reflectDirVS.z);
    //reflectColor = -normalVS.z;
    //reflectColor = dir.x;
    
    reflectColor = colorIn;
    //reflectColor = (float)_DebugViewStep / 50;
    
    //
    // Debug views!
    //
    if (isDebugTrace)
    {
        // Show start point
        reflectColor = float4(1, 1, 0, 0);
    }
    
    // Show current point
    //GroupMemoryBarrierWithGroupSync();
    AllMemoryBarrierWithGroupSync();
    uint2 mipMod = tId.xy % (1u << DebugData[2]);
    if (tId.x == DebugData[0] && tId.y == DebugData[1])
    {
        reflectColor = float4(0, 1, 0, 0);
    }
    else if (tId.x == DebugData[0] || tId.y == DebugData[1])
    {
        reflectColor = lerp(reflectColor, float3(1, 0, 0), 0.8f);
    }
    else if (DebugData[2] > 0u && any(mipMod == 0u))
    {
        reflectColor = lerp(reflectColor, float3(0, 1, 0), 0.1f);
    }
    
    CameraColorOut[tId.xy] = float4(reflectColor, colorIn.a);
    
}