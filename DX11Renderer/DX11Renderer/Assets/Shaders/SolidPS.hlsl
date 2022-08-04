#include "./CbufCommon.hlsli"

float4 main() : SV_Target
{
    return float4(materialColor.rgb, 1);
}