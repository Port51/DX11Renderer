
#include "PhongCommon.hlsli"

cbuffer LightCBuf
{
	float3 lightPos;
	float3 ambient;
	float3 diffuseColor;
	float diffuseIntensity;
	float attConst;
	float attLin;
	float attQuad;
};

cbuffer ObjectCBuf
{
	float3 materialColor;
	float specularIntensity;
	float specularPower;
};

#include "Lighting/BRDF.hlsli"

float SCurve(float x)
{
    // (3x^2 - 2x^3)
    return (-2 * x + 3) * x * x; // OPS: [MAD] [MUL] [MUL]
}

float4 main(v2f i) : SV_Target
{
    //return n.z;
    i.normalVS = normalize(i.normalVS);
    
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
    float linearRoughness = 0.75;
    float roughness = pow(linearRoughness, 2);
    BRDFLighting brdf = BRDF(f0, f90, roughness, linearRoughness, normalVS, -viewDirVS, normalize(vToL));
    
    // cheap ambient gradient
    float3 ambient = pow(i.normalVS.y * -0.5 + 0.5, 2) * 0.15 * float3(0.5, 0.75, 1.0);
    
    return float4(brdf.diffuseLight + brdf.specularLight + ambient, 1);
    
}