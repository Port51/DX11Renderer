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

#include "./Schlick.hlsli"

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
    float lightScatter = F_Schlick_2(f0, fd90, NdotL);
    float viewScatter = F_Schlick_2(f0, fd90, NdotV);
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