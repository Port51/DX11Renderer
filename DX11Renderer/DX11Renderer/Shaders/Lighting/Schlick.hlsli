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