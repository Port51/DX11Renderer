#ifndef _BRDF_INCLUDED
#define _BRDF_INCLUDED

// References:
// SIGGRAPH 2014 paper "Moving Frostbite to Physically Based Rendering"

#define PI 3.14159265359

#ifndef _SCHLICK_INCLUDED
#define _SCHLICK_INCLUDED

// Optimized version using f0.r
inline float F_Schlick_1D(in float3 f0, in float f90, in float u)
{
    // u = 1 at incident angle and 0 at grazing angles
    return (f90 - f0.x) * pow(1.0 - u, 5) + f0.x;
}

inline float3 F_Schlick(in float3 f0, in float f90, in float u)
{
    // u = 1 at incident angle and 0 at grazing angles
    return (f90 - f0) * pow(1.0 - u, 5) + f0;
}

#endif

#ifndef _DIFFUSE_INCLUDED
#define _DIFFUSE_INCLUDED

// # Copyright Disney Enterprises, Inc.  All rights reserved.
// #
// # Licensed under the Apache License, Version 2.0 (the "License");
// # you may not use this file except in compliance with the License
// # and the following modification to it: Section 6 Trademarks.
// # deleted and replaced with:
// #
// # 6. Trademarks. This License does not grant permission to use the
// # trade names, trademarks, service marks, or product names of the
// # Licensor and its affiliates, except as required for reproducing
// # the content of the NOTICE file.
// #
// # You may obtain a copy of the License at
// # http://www.apache.org/licenses/LICENSE-2.0

// Usage notes:
// - Modified with normalization from Frostbite
// - Added some other optimizations

float3 F_Schlick_2(in float3 f0, in float f90, in float u)
{
    return f0 + (f90 - f0) * pow(1.0 - u, 5.0);
}

// Should have sharper falloff and more lighting with low roughness

// Formula: Fd = Flambert (1 - 0.5 F_L) (1 - 0.5 F_V) + F_RetroReflection
// F_RetroReflection = (baseColor / PI) R_R (F_L + F_V + F_L * F_V * (R_R - 1))
// F_L = (1 - NdotL)^5
// F_V = (1 - NdotV)^5
// R_R = 2 * roughness * cos^2 (theta_D)
// Flambert = baseColor / PI
inline float NormalizedDisneyDiffuse(float NdotV, float NdotL, float LdotH, float linearRoughness)
{
    //float energyBias = linearRoughness * 0.5; // Optimized from lerp(0.0, 0.5, linearRoughness);
    //float energyFactor = linearRoughness * -0.33774834 + 1.0; // Optimized from lerp(1.0, 1.0 / 1.51, linearRoughness);
    linearRoughness = 0.5;
    float energyBias = lerp(0.0, 0.5, linearRoughness);
    float energyFactor = lerp(1.0, 1.0 / 1.51, linearRoughness);
    float fd90 = (LdotH * LdotH * linearRoughness) * 2.0 + energyBias;
    float3 f0 = (float3) 1.0;
    float lightScatter = F_Schlick_2(f0, fd90, NdotL).x;
    float viewScatter = F_Schlick_2(f0, fd90, NdotV).x;
    float Fd = lightScatter * viewScatter * energyFactor * NdotL;
    //return NdotL;
    //return energyBias * NdotL;
    //return energyBias * NdotL;
    //Fd = lightScatter * energyFactor;
    return Fd;
}

float SchlickFresnel(float u)
{
    float m = clamp(1.0 - u, 0.0, 1.0);
    float m2 = m * m;
    return m2 * m2 * m; // pow(m,5)
}

inline float DisneyDiffuse(float NdotV, float NdotL, float LdotH, float linearRoughness)
{
    // Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
    // and mix in diffuse retro-reflection based on roughness
    float lightScatter = SchlickFresnel(NdotL);
    float viewScatter = SchlickFresnel(NdotV);
    float fd90_minus_1 = 2.0 * LdotH * LdotH * linearRoughness - 0.5;

    float Fd = (1.0 + fd90_minus_1 * pow(1.0 - NdotL, 5.0))
        * (1.0 + fd90_minus_1 * pow(1.0 - NdotV, 5.0));
    return Fd;
}

#endif

//#define PI 3.141592
#define Epsilon 0.00001

struct BRDFLighting
{
    float3 diffuseLight;
    float3 specularLight;
};

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

    float GSF = SmithGGXCorrelated(NdotL, NdotV, roughness);
    float NDF = GGX(NdotH, roughness);
    //float Fr = NDF * FF * GSF / (PI * 4.0 * NdotL * NdotV);
    float3 Fr = NDF * FF * GSF / PI;

    // Diffuse BRDF
    float Fd = NormalizedDisneyDiffuse(NdotV, NdotL, LdotH, linearRoughness) / PI;

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