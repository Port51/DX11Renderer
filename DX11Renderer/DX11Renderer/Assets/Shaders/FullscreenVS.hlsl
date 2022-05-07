struct v2f
{
    float2 uv : Texcoord;
    float4 pos : SV_Position;
};

v2f main(float2 pos : Position)
{
    v2f o;
    o.pos = float4(pos * 2.0f - 1.0f, 0.0f, 1.0f);
    o.uv = float2(pos.x, 1.0f - pos.y);
    return o;
}