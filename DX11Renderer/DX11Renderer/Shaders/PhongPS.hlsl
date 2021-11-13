
#include "PhongCommon.hlsli"

cbuffer LightCBuf : register(b0)
{
	float3 lightPos;
    float lightInvRangeSqr;
	float3 lightColor;
	float lightIntensity;
};

cbuffer ObjectCBuf : register(b1)
{
	float3 materialColor;
    float roughness;
    bool normalMapEnabled; // 4 bytes in HLSL, so use BOOL in C++ to match
	float specularPower;
    float padding[2];
};

/*cbuffer CBuf : register(b2)
{
    matrix model;
    matrix modelView;
    matrix modelViewProj;
};*/

Texture2D tex : register(t0);
Texture2D nmap : register(t1);
SamplerState splr : register(s0);

#include "Lighting/BRDF.hlsli"

float SCurve(float x)
{
    // (3x^2 - 2x^3)
    return (-2 * x + 3) * x * x; // OPS: [MAD] [MUL] [MUL]
}

float4 main(v2f i) : SV_Target
{
    //return float4(lightColor, 1);
    //return abs(i.tangentVS.xyzz);
    //return frac(i.uv0.x * 10);
    
    i.normalVS = normalize(i.normalVS);
    i.tangentVS = normalize(i.tangentVS);
    float3 bitangentVS = cross(i.tangentVS, i.normalVS);
    float3x3 tbnMatrix = float3x3(i.tangentVS.xyz, bitangentVS.xyz, i.normalVS.xyz);
    
    i.uv0.y = 1 - i.uv0.y;
    float4 diffuseTex = tex.Sample(splr, i.uv0);
    
    float3 n;
    if (normalMapEnabled)
    {
        const float3 normalSample = nmap.Sample(splr, i.uv0).xyz;
        n.x = normalSample.x * 2.0f - 1.0f;
        n.y = -normalSample.y * 2.0f + 1.0f; // Convert from OpenGL to DX style
        n.z = -normalSample.z;
        //n = float3(0, 0, 1);
        n = mul(n, tbnMatrix); // no need to transpose
        //return i.tangentVS.y;
        //return n.x;
        i.normalVS = normalize(-n); // fix sampling interpolation
    }
    
    LightData lightData;
    lightData.positionRangeOrDirectionVS = float4(lightPos, lightInvRangeSqr),
    lightData.color = float4(lightColor, 1);
    
    Light light = GetPointLight(lightData, i.positionVS, i.normalVS);
    
    const float3 vToL = lightPos - i.positionVS;
    
    /*
	// fragment to light vector data
	const float distToL = length(vToL);
	const float3 dirToL = vToL / distToL;
	// attenuation
    const float att = SCurve(1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL)));
	// diffuse intensity
	const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, i.normalVS));
	// reflected light vector
    const float3 w = i.normalVS * dot(vToL, i.normalVS);
	const float3 r = w * 2.0f - vToL;
	// calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
	const float3 specular = att * (diffuseColor * diffuseIntensity) * specularIntensity * pow(max(0.0f, dot(normalize(-r), normalize(i.positionVS))), specularPower);
	// final color
	//return float4(saturate((diffuse + ambient + specular) * materialColor), 1.0f);
    */
    
    float3 normalVS = i.normalVS;
    float3 viewDirVS = normalize(i.positionVS);
    float3 f0 = 0.5;
    float f90 = 1;
    float roughnessSqr = pow(roughness, 2);
    BRDFLighting brdf = BRDF(f0, f90, roughnessSqr, roughness, normalVS, -viewDirVS, light.direction);
    
    // cheap ambient gradient
    float3 ambient = pow(i.normalVS.y * -0.5 + 0.5, 2) * 0.15 * float3(0.75, 0.95, 1.0);
    
    brdf.diffuseLight += ambient;
    
    //return diffuseTex;
    //return materialColor.rgbb;
    return float4((brdf.diffuseLight * diffuseTex.rgb + brdf.specularLight) * materialColor * lightColor * (lightIntensity * light.attenuation), 1);
    
}