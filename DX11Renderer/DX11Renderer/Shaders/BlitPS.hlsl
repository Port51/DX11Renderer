Texture2D tex : register(t0);
SamplerState splr : register(s0);

float4 main(float2 uv : Texcoord) : SV_Target
{
    //return float4(0, uv.x, 0, 1);
    //return float4(0, 1, 0, 1);
    return tex.Sample(splr, uv).rgba;
}