
cbuffer ObjectCBuf : register(b1)
{
    float3 materialColor;
    float specularIntensity;
    bool normalMapEnabled; // 4 bytes in HLSL, so use BOOL in C++ to match
    float specularPower;
    float padding[2];
};

float4 main() : SV_Target
{
    return float4(materialColor.rgb, 1);
}