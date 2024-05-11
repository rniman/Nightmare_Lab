struct VS_STANDARD_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float3 tangentW : TANGENT;
    float3 bitangentW : BITANGENT;
    float2 uv : TEXCOORD;
};

struct PS_SHADOW_OUTPUT
{
    float depth : SV_Target0;
};

PS_SHADOW_OUTPUT PSShadow(VS_STANDARD_OUTPUT input)
{
    PS_SHADOW_OUTPUT output;
    output.depth = input.position.z;
    
    return output;
}
