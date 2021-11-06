cbuffer ObjectCBuf
{
    float3 materialColor;
    float specularIntensity;
    float specularPower;
};

float4 main() : SV_Target
{
    return float4(materialColor.rgb, 1);
}