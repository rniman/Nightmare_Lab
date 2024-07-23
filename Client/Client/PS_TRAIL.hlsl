#include "Common.hlsl"

struct VS_TRAIL_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float startTime : STARTTIME;
};


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
