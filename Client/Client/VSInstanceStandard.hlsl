#include "Common.hlsl"

struct VS_INSTANCE_STANDARD_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float4x4 ins_transform : INSMATRIX;
};

VS_STANDARD_OUTPUT VSInstanceStandard(VS_INSTANCE_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output;
    
    float4x4 transformMatrix = mul(input.ins_transform, gmtxGameObject);
    
    output.positionW = mul(float4(input.position, 1.0f), transformMatrix).xyz;
    output.normalW = mul(input.normal, (float3x3) transformMatrix);
    output.tangentW = mul(input.tangent, (float3x3) transformMatrix);
    output.bitangentW = mul(input.bitangent, (float3x3) transformMatrix);
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;
    
    return (output);
}