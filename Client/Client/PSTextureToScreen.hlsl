Texture2D DFTextureBlur : register(t14);

SamplerState gssWrap : register(s0);

struct VS_TEXTURED_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 PSTextureToScreen(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
    float4 cColor = DFTextureBlur.Sample(gssWrap, input.uv);
    return cColor;
}