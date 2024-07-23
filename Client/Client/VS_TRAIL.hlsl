#include "Common.hlsl"

struct VS_TRAIL_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float startTime : STARTTIME;
};

struct VS_TRAIL_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float startTime : STARTTIME;
};


VS_TRAIL_OUTPUT VS_TRAIL(VS_TRAIL_INPUT input)
{
    VS_TRAIL_OUTPUT output;
    
    float3 positionW = input.position;
    output.position = mul(mul(float4(positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;
    output.startTime = input.startTime;
    
    return (output);
}

