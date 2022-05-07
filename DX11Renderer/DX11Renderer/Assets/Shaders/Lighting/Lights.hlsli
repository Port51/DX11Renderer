#ifndef _LIGHTS_INCLUDED
#define _LIGHTS_INCLUDED

// Reference:
// https://imdoingitwrong.wordpress.com/2011/02/10/improved-light-attenuation/
// This version takes that formula, and modifies the inputs so they are easier to work with
// Mainly, the modifications allow working with MaxRange, Sharpness, and Contrast
// And the overall shape scales with MaxRange
inline float GetSphericalLightAttenuation(float lightDist, float rcpLightSphereRad, float lightRange)
{
    // Formulas:
    // d = light distance
    // M = max light distance
    // Q = falloff sharpness
    // R = light radius = sqrt(M) / [Q * (1 - 0.25 * shape)]
    
    // d0 = d^2 / [1 - (d^2 / M^2)]
    // I = 1 / (d0 / R + 1)^2
    
    float dp = lightDist / lightRange;
    float d0 = lightDist * lightDist / (dp * -dp + 1.0);
    return step(lightDist, lightRange) / pow(d0 * rcpLightSphereRad + 1.0, 2);
}

#endif