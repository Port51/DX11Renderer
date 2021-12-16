#ifndef _LIGHTS_INCLUDED
#define _LIGHTS_INCLUDED

// Reference:
// https://imdoingitwrong.wordpress.com/2011/02/10/improved-light-attenuation/
inline float GetSphericalLightAttenuation(float lightDist, float rcpLightSphereRad, float lightRange)
{
    float distRatio = lightDist / lightRange;
    float dPrime = lightDist / max(0.0001, distRatio * -distRatio + 1.0);
    return pow(rcp(dPrime * rcpLightSphereRad + 1), 2);
}

#endif