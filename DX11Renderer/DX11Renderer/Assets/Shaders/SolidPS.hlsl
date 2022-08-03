
cbuffer ObjectCBuf : register(b5)
{
    float3 materialColor;
    float roughness;
    bool normalMapEnabled; // 4 bytes in HLSL, so use BOOL in C++ to match
    float specularPower;
    float reflectivity;
    float padding[1];
};

float4 main() : SV_Target
{
    return float4(materialColor.rgb, 1);
}