struct VS_STANDARD_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float3 tangentW : TANGENT;
    float3 bitangentW : BITANGENT;
    float2 uv : TEXCOORD;
};

float4 PSOutLine(VS_STANDARD_OUTPUT input) : SV_TARGET
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
