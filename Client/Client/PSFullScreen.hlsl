#include "Common.hlsl"

float4 PSFullScreen(PS_POSTPROCESSING_OUT input) : SV_Target
{
    float4 cColor = AlbedoTexture.Sample(gssWrap, input.uv);
    cColor *= 2.5f;
    cColor.a = option.alphaValue;
    
    return cColor;
}
