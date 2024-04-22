#include "Shaders.hlsl"

struct VS_BLEND_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

struct VS_BLEND_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float2 uv : TEXCOORD;
};


    VS_BLEND_OUTPUT VS_Blend(VS_BLEND_INPUT input)
{
    VS_BLEND_OUTPUT output;
    
    output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
    output.normalW = mul(input.normal, (float3x3) gmtxGameObject);
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;
    
	return output;
}

#define offset 3

float4 PS_Blend(VS_BLEND_OUTPUT input) : SV_Target
{
    float4 PScolor = 0.0f;
    
    float2 uv = input.uv / offset;
    
    float elapsedTime = frac(localTime/1.0f); // 현재 시간에서 정수 부분을 제거하여 경과된 시간만 남긴다.
    int count = int(elapsedTime * offset*offset); // 1초당
    
    float t = fmod(count, offset * offset);
    
    int xoffset = fmod(t, offset);
    int yoffset = t / offset;
    
    uv.x += (1.0f / offset) * float(xoffset);
    uv.y += (1.0f / offset) * float(yoffset);
    
    PScolor += AlbedoTexture.Sample(gssWrap, uv);
    
    return PScolor;
}
