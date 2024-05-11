#define offset 3

cbuffer cbFrameInfo : register(b5)
{
    float time : packoffset(c0.x);
    float localTime : packoffset(c0.y);
    float usePattern : packoffset(c0.z);
    
    // [0506] 플레이어 외곽선
    float gfTrackingTime : packoffset(c0.w);
}

SamplerState gssWrap : register(s0);
Texture2D AlbedoTexture : register(t0);

struct VS_BLEND_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float2 uv : TEXCOORD;
};

float4 PSBlend(VS_BLEND_OUTPUT input) : SV_Target
{
    float4 PScolor = 0.0f;
    
    float2 uv = input.uv / offset;
    
    float elapsedTime = frac(localTime / 1.0f); // 현재 시간에서 정수 부분을 제거하여 경과된 시간만 남긴다.
    int count = int(elapsedTime * offset * offset); // 1초당
    
    float t = fmod(count, offset * offset);
    
    int xoffset = fmod(t, offset);
    int yoffset = t / offset;
    
    uv.x += (1.0f / offset) * float(xoffset);
    uv.y += (1.0f / offset) * float(yoffset);
    
    PScolor += AlbedoTexture.Sample(gssWrap, uv);
    
    return PScolor;
}
