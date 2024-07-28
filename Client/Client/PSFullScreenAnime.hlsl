#include "Common.hlsl"

float4 PSFullScreenAnime(PS_POSTPROCESSING_OUT input) : SV_Target
{
    float2 uv = input.uv - float2(0.5f,0.5f);
    float len = length(uv);
    
    float power = (frac(gfCurrentTime) * 4.f) + 3.f;
    float p_result = pow(len, power);
    
    float4 cColor = float4(1.f, 0.f, 0.f, p_result);
    
    return cColor;
}
