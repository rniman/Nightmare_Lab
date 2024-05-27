#include "Common.hlsl"

struct VS_USER_INTERFACE_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_USER_INTERFACE_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_USER_INTERFACE_OUTPUT VSUserInterface(VS_USER_INTERFACE_INPUT input)
{
    VS_USER_INTERFACE_OUTPUT output;

    float4 positionW = mul(float4(input.position, 1.0f), gmtxGameObject);
    output.position = float4(positionW.xyz, 1.0f);
    output.uv = input.uv;
    
    return output;
}