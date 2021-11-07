#ifndef _BRDF_INCLUDED
#define _BRDF_INCLUDED

// From SIGGRAPH 2014 paper "Moving Frostbite to Physically Based Rendering"

#define PI 3.14159265359

#include "./Schlick.hlsli"
#include "./Diffuse.hlsli"

//#define PI 3.141592
#define Epsilon 0.00001

struct BRDFLighting
{
    float3 diffuseLight;
    float3 specularLight;
};

// Attenuation function f(d):
// f(d) = 2 / [ d^2 + rad^2 + d*sqrt(d^2 + rad^2) ]
// f(d) = 2 / [ d^2 + rad^2 + sqrt(d^4 + d^2 * rad^2) ] <-- optimized
// Range attenuation functions a(d):
// a(d) = saturate(1 - d^2 / range^2)
// a(d) = saturate(1 - d^4 / range^4)
// Modified attenuation function:
// v = d / range
// f(v) = 2 / ( v^2 + rad^2 + v * sqrt(v^2 + rad^2) )
// f(v) = 2 / ( v^2 + rad^2 + sqrt(v^4 + v^2rad^2) ) <-- optimized

float GetSphericalLightAttenuation(float distanceRatioSqr, float lightSphereRadiusSqr)
{
    return 2.0 / (distanceRatioSqr + lightSphereRadiusSqr + sqrt(distanceRatioSqr * distanceRatioSqr + distanceRatioSqr * lightSphereRadiusSqr));
}

// Multiply this by an attenuation that converges to 0 at infinite distance in order to limit it
float GetLightRangeAttenuation(float distanceSquare, float invRangeSqr)
{
    float falloff = distanceSquare * invRangeSqr;
    return saturate(1.0 - falloff * falloff); // square factor so final formula is f(d) = 1 - d^4 / r^4
}

float GetSphericalLightDistanceAttenuation(float3 posToLight, float lightSphereRadiusSqr, float invRangeSqr)
{
    float distanceSqr = dot(posToLight, posToLight);
    float attenuation = GetSphericalLightAttenuation(distanceSqr * invRangeSqr, lightSphereRadiusSqr);
    attenuation *= GetLightRangeAttenuation(distanceSqr, invRangeSqr);
    return attenuation;
}

// From buffer
struct LightData
{
    float4 positionRangeOrDirectionVS;
    float4 color;
};

struct Light
{
    float4 colorIntensity; // rgb, pre-exposed intensity
    float3 direction;
    float attenuation;
    float NdotL;
    bool castsShadows;
    bool contactShadows;
    uint shadowIndex;
    uint shadowLayer;
};

Light GetPointLight(LightData data, float3 positionVS, float3 normalVS)
{
    float3 posToLight = data.positionRangeOrDirectionVS.xyz - positionVS;

    Light light;
    light.colorIntensity.rgb = data.color.rgb;
    light.colorIntensity.w = 1; //computePreExposedIntensity(data.color.w, frameUniforms.exposure);
    light.direction = normalize(posToLight);
    light.attenuation = GetSphericalLightDistanceAttenuation(posToLight, pow(1.4, 2), data.positionRangeOrDirectionVS.w);
    //light.attenuation = (1.f / data.positionRangeOrDirectionVS.w) / dot(posToLight, posToLight);
    light.NdotL = saturate(dot(normalVS, light.direction));

    //uint type = floatBitsToUint(scaleOffsetShadowType.w);
    /*if (type == LIGHT_TYPE_SPOT) {
        light.attenuation *= getAngleAttenuation(-directionIES.xyz, light.l, scaleOffsetShadowType.xy);
        uint shadowBits = floatBitsToUint(scaleOffsetShadowType.z);
        light.castsShadows = bool(shadowBits & 0x1u);
        light.contactShadows = bool((shadowBits >> 1u) & 0x1u);
        light.shadowIndex = (shadowBits >> 2u) & 0xFu;
        light.shadowLayer = (shadowBits >> 6u) & 0xFu;
    }*/

    return light;
}

float SmithGGXCorrelated(float NdotL, float NdotV, float alphaG);
float SmithGSF(float NdotL, float NdotV, float roughness);
float GGX(float NdotH, float m);

BRDFLighting BRDF(float3 f0, float f90, float roughness, float linearRoughness, float3 normalVS, float3 viewDirVS, float3 lightDirVS)
{
    BRDFLighting brdf;

    // This code is an example of call of previous functions
    float NdotL = saturate(dot(normalVS, lightDirVS));
    float NdotV = max(abs(dot(normalVS, viewDirVS)), 0.00001);

    // Normal way of getting LdotH and NdotH
    //float3 H = normalize(viewDirVS + lightDirVS);
    //float LdotH = saturate(dot(lightDirVS, H));
    //float NdotH = saturate(dot(normalVS, H));

    // Optimized method that doesn't calculate H:
    // Reference: https://www.gdcvault.com/play/1024063/PBR-Diffuse-Lighting-for-GGX
    // Go to ~41:00 timestamp
    float LdotV = saturate(dot(lightDirVS, viewDirVS));
    float LVSqr = LdotV * 2.0 + 2.0;
    float rcpLVSqr = rsqrt(LVSqr);
    float NdotH = (NdotL + NdotV) * rcpLVSqr;
    float LdotH = rcpLVSqr * LdotV + rcpLVSqr;

    // Specular BRDF
    float3 FF = F_Schlick(f0, f90, LdotH);
    //float3 F = F_Schlick(f0, f90, NdotV); // bad!
    //F = f0 + (1.0 - f0) * pow(1.0 - LdotH, 5.0);


    float GSF = SmithGGXCorrelated(NdotL, NdotV, roughness); // opposite side?
    //float GSF = SmithGSF(NdotL, NdotV, roughness);
    float NDF = GGX(NdotH, roughness); // looks good!
    //float Fr = NDF * FF * GSF / (PI * 4.0 * NdotL * NdotV);
    float Fr = NDF * FF * GSF / PI;

    // Diffuse BRDF
    float Fd = NormalizedDisneyDiffuse(NdotV, NdotL, LdotH, linearRoughness) / PI;
    //float Fd = DisneyDiffuse(NdotV, NdotL, LdotH, linearRoughness) / PI;

    brdf.diffuseLight = Fd * NdotL;
    brdf.specularLight = Fr * NdotL;
    //brdf.specularLight = NdotV;
#if defined(_DEBUG_VIEW_DIFFUSE)
    brdf.specularLight = 0;
#elif defined(_DEBUG_VIEW_FF)
    brdf.diffuseLight = 0;
    brdf.specularLight = FF;
#elif defined(_DEBUG_VIEW_NDF)
    brdf.diffuseLight = 0;
    brdf.specularLight = NDF;
#elif defined(_DEBUG_VIEW_GSF)
    // For the debug view of visibility, make sure to include 4.0 * NdotV * NdotL
    brdf.diffuseLight = 0;
    brdf.specularLight = GSF * 4.0 * NdotV * NdotL;
#endif
    //brdf.specularLight = SmithGGXCorrelated(NdotL, NdotV, roughness);
    //brdf.specularLight = SmithGSF(NdotL, NdotV, roughness);
    return brdf;
}

// This looks way different from uncorrelated GSF, and that's expected
float SmithGGXCorrelated(float NdotL, float NdotV, float roughness)
{
    // Original formulation of G_SmithGGX Correlated
    // lambda_v = ( -1 + sqrt ( alphaG2 * (1 - NdotL2 ) / NdotL2 + 1)) * 0.5 f;
    // lambda_l = ( -1 + sqrt ( alphaG2 * (1 - NdotV2 ) / NdotV2 + 1)) * 0.5 f;
    // G_SmithGGXCorrelated = 1 / (1 + lambda_v + lambda_l );
    // V_SmithGGXCorrelated = G_SmithGGXCorrelated / (4.0 f * NdotL * NdotV );
    // This is the optimized version
    float alphaG2 = roughness * roughness;
    float Lambda_GGXV = NdotL * sqrt((NdotV * -alphaG2 + NdotV) * NdotV + alphaG2);
    float Lambda_GGXL = NdotV * sqrt((NdotL * -alphaG2 + NdotL) * NdotL + alphaG2);

    float a2 = roughness * roughness;
    float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - a2) + a2);
    float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - a2) + a2);
    return 0.5 / (GGXV + GGXL);

    return 0.5f / (Lambda_GGXV + Lambda_GGXL);
}

float SmithGSF(float NdotL, float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8;
    float ggx1 = NdotV / (NdotV * (1 - k) + k);
    float ggx2 = NdotL / (NdotL * (1 - k) + k);
    return ggx1 * ggx2;

    return NdotV / (NdotV * (1 - roughness) + roughness);
}

float GGX(float NdotH, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;

    // Divide by PI is applied later
    float denom = (NdotH * NdotH) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (denom * denom);

    // Divide by PI is applied later
    float roughnessSqr = roughness * roughness;
    float f = (NdotH * roughnessSqr - NdotH) * NdotH + 1.0;
    return roughnessSqr / max(f * f, 0.00001);
}

#endif