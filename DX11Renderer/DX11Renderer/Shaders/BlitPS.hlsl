Texture2D tex;
SamplerState splr;

float4 main(float2 uv : Texcoord) : SV_Target
{
    return 0.5f;
    return tex.Sample(splr, uv).rgba;
}