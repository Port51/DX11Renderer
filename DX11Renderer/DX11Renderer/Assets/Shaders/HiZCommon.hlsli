#ifndef HIZ_COMMON_INCLUDED
#define HIZ_COMMON_INCLUDED

// Set whether to use linear or hyperbolic depth
#define HZB_USES_LINEAR_DEPTH

#if defined(HZB_USES_LINEAR_DEPTH)
    #define HZB_LINEAR(sample, zBufferParams) Depth01ToEyeDepth(sample)
#else
    #define HZB_LINEAR(sample, zBufferParams) LinearEyeDepth(sample, zBufferParams)
    #define HZB_PARABOLIC(sample, zBufferParams) (sample * _ProjectionParams.z)
#endif

#endif