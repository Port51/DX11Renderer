// References:
// Concepts         - https://www.youtube.com/watch?v=21gfE-zUym8
// More concepts    - https://www.youtube.com/watch?v=OqRMNrvu6TE
// HS indices       - https://www.raywenderlich.com/books/metal-by-tutorials/v2.0/chapters/11-tessellation-terrains

#include "./Common.hlsli"
#include "./CbufCommon.hlsli"
#include "./GerstnerWaves.hlsli"

// Input patch size must match IASetPrimitiveTopology()
#define INPUT_PATCH_SIZE 4
#define OUTPUT_PATCH_SIZE 4

struct attrib
{
    float3 pos : Position;
    float3 n : Normal;
    float3 t : Tangent;
    float2 uv0 : Texcoord0;
};

struct ControlPt
{
    float3 positionWS       : POSITION;
    float tessFactor        : TEXCOORD0;
    float2 uv               : TEXCOORD1;
};

cbuffer PerObjectTransformCB : register(b3)
{
    matrix model;
    matrix modelView;
    matrix modelViewProj;
};

float GetDistanceFactor(float3 positionWS)
{
    float3 camOffsetWS = _CameraPositionWS.xyz - positionWS.xyz;
    float camDist = length(camOffsetWS);
    float factor = 1.f / camDist;

    const float farLod = saturate((camDist - 35.0f) / 50.0f);

    // Extra factor to use fewer verts at a distance
    //factor *= lerp(1.0f, 0.75f, farLod);

    // Lower tessellation at grazing angles, if at a certain distance
    float anisoFactor = (1.f - abs(camOffsetWS.y) / camDist) * farLod;
    factor *= lerp(1.f, 0.1f, saturate(anisoFactor * 1.5f));

    return saturate(factor);
}

ControlPt WaterVS(attrib i)
{
    ControlPt o;

    float3 positionWS = mul(model, float4(i.pos.xyz, 1.0f)).xyz;
    o.positionWS = positionWS;
    o.uv = i.uv0;

    const float tessMax = 551.f;
    o.tessFactor = GetDistanceFactor(positionWS) * tessMax + 1.f;

    return o;
}

struct ContantData
{
    float Edges[4]             : SV_TessFactor;
    float Inside[2]            : SV_InsideTessFactor;
};

// Called x1 time per patch
ContantData WaterConstantHS(InputPatch<ControlPt, INPUT_PATCH_SIZE> pt, uint PatchID : SV_PrimitiveID)
{
    ContantData o;

    // Outer tessellation must match edges of other patches
    // To do that, average tessFactor from the 2 points making up each edge
    // Here's a reference for how to match indices of control points with edges:
    // https://www.raywenderlich.com/books/metal-by-tutorials/v2.0/chapters/11-tessellation-terrains
    o.Edges[0] = (pt[3].tessFactor + pt[0].tessFactor) * 0.5f;
    o.Edges[1] = (pt[0].tessFactor + pt[1].tessFactor) * 0.5f;
    o.Edges[2] = (pt[1].tessFactor + pt[2].tessFactor) * 0.5f;
    o.Edges[3] = (pt[2].tessFactor + pt[3].tessFactor) * 0.5f;

    // Inner depends on center of patch, so use average of all control points
    o.Inside[0] = o.Inside[1] = (pt[0].tessFactor + pt[1].tessFactor + pt[2].tessFactor + pt[3].tessFactor) * 0.25f;

    return o;
}

struct h2d
{
    float3 positionWS           : TEXCOORD0;
    float2 uv                   : TEXCOORD1;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(OUTPUT_PATCH_SIZE)]
[patchconstantfunc("WaterConstantHS")]
h2d WaterHS(InputPatch<ControlPt, INPUT_PATCH_SIZE> p,
    uint i : SV_OutputControlPointID,
    uint PatchID : SV_PrimitiveID)
{
    h2d o;
    o.positionWS = p[i].positionWS;
    o.uv = p[i].uv;
    return o;
}

struct d2f
{
    float4 pos : SV_Position;
    float3 normalVS : Normal;
    float3 positionVS : TEXCOORD0;
    float3 positionWS : TEXCOORD1;
    float3 normalWS : TEXCOORD2;
    float3 tangentVS : TEXCOORD3;
    float2 uv : TEXCOORD4;
    float4 vertColor : TEXCOORD5;
    float4 positionNDC : TEXCOORD6;
    float4 rng : TEXCOORD7;
};

[domain("quad")]
d2f WaterDS(ContantData i, float2 dl : SV_DomainLocation, const OutputPatch<h2d, OUTPUT_PATCH_SIZE> patches)
{
    d2f o;

    // Interpolate attributes from input patches
    float3 lPos = lerp(patches[0].positionWS, patches[3].positionWS, dl.y);
    float3 rPos = lerp(patches[1].positionWS, patches[2].positionWS, dl.y);
    float3 positionWS = lerp(lPos, rPos, dl.x);

    float2 lUV = lerp(patches[0].uv, patches[3].uv, dl.y);
    float2 rUV = lerp(patches[1].uv, patches[2].uv, dl.y);
    float2 uv = lerp(lUV, rUV, dl.x);

    float3 tangentWS = GetGerstnerWavesTangent(positionWS);
    float3 bitangentWS = GetGerstnerWavesBitangent(positionWS);
    float3 normalWS = cross(tangentWS, bitangentWS);
    o.normalWS = normalWS.xyz;
    o.normalVS = mul((float3x3) _ViewMatrix, normalWS).xyz;
    o.tangentVS = mul((float3x3) _ViewMatrix, tangentWS).xyz;

    positionWS = GetGerstnerWaves(positionWS);

    o.pos = mul(_ViewProjMatrix, float4(positionWS, 1.f));
    o.positionNDC = o.pos;
    o.positionWS = positionWS;
    o.uv = uv;

    return o;
}

// Per-frame
Texture2D SpecularLightingRT : register(t5);
Texture2D DiffuseLightingRT : register(t6);

// Per-draw
Texture2D tex : register(t0);
Texture2D sdfs : register(t1);
SamplerState splr : register(s0);
SamplerState splr2 : register(s1);

float CalculateFoam(float3 positionWS, float2 uv)
{
    const float sdfScale = 3.7f;
    const float2 sdfOffset = float2(0.05f, 0.0f);
    const float4 sdfTex = sdfs.Sample(splr2, (uv - 0.5f) * sdfScale + 0.5f + sdfOffset);

    const float foamScale = 0.35;
    const float foamTex0 = tex.Sample(splr, positionWS.xz * foamScale + _Time.x * 1.13891).x;
    const float foamTex1 = tex.Sample(splr, positionWS.xz * foamScale * 0.8924 - _Time.x + float2(0.24839, 0.78214)).x;
    float foam = saturate((positionWS.y + 6.05f) * 1.1f);
    foam = saturate(foam + (sdfTex.r + sdfTex.g) * 0.75f);
    foam *= lerp(foamTex0 * foamTex1, 1.0f, foam);
    foam = SCurve(foam);

    return foam;
}

float4 WaterGBufferPS(d2f i) : SV_TARGET
{
    i.normalVS = normalize(i.normalVS);
    i.tangentVS = normalize(i.tangentVS);
    float3 bitangentVS = cross(i.tangentVS, i.normalVS);
    float3x3 tbnMatrix = float3x3(i.tangentVS.xyz, bitangentVS.xyz, i.normalVS.xyz);

    float4 diffuseTex = tex.Sample(splr, i.uv);

    // Calculate procedural normals
    {
        // todo: switch to normalmaps and whiteout blending instead?
        const float2 baseUV = (i.positionWS.xz + _Time.x * 4.0f * float2(0.09328f, -10.29303f)) * float2(0.01f, 0.1f);
        const float sampleOffset = 0.01f;
        const float n0 = tex.Sample(splr, baseUV).x;
        const float n1 = tex.Sample(splr, baseUV + float2(sampleOffset, 0.0)).x;
        const float n2 = tex.Sample(splr, baseUV + float2(0.0, sampleOffset)).x;

        const float scale = 300.0f;
        float3 n = float3((n1 - n0) / sampleOffset, (n2 - n1) / sampleOffset, scale);
        n = mul(n, tbnMatrix); // no need to transpose
        i.normalVS = normalize(n);
    }

    float foam = CalculateFoam(i.positionWS, i.uv);

    return float4(i.normalVS.xy * 0.5f + 0.5f, roughness, reflectivity * (1.0 - foam));
}

float4 WaterPS(d2f i) : SV_TARGET
{
    float3 positionNDC = i.positionNDC.xyz / i.positionNDC.w;
    float2 screenPos = positionNDC.xy * float2(0.5f, -0.5f) + 0.5f;
    float4 specularLight = SpecularLightingRT.Sample(splr, screenPos);
    float4 diffuseLight = DiffuseLightingRT.Sample(splr, screenPos);

    float3 combinedLight = specularLight.rgb + diffuseLight.rgb;
    float4 diffuseTex = 1.0;

    float foam = CalculateFoam(i.positionWS, i.uv);

    return float4(combinedLight.rgb * lerp(materialColor.rgb, 3.0, foam).rgb, 1) * diffuseTex;
}
