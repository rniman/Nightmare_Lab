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

float4 PS_TRAIL(VS_TRAIL_OUTPUT input) : SV_Target
{
    float4 cColor = AlbedoTexture.Sample(gssWrap, input.uv);
    clip(cColor.a - 0.2f);
    float alpha = lerp(0.0f, 1.0f, clamp(input.startTime + 1.0f - gfCurrentTime, 0.0f, 1.0f));
    cColor.a *= alpha;
    cColor.r += 0.65f;
    
    //float4 cColor = float4(1.f, 0.f, 0.f, alpha);
    
    return (cColor);
}
