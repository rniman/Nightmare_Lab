#include "Common.hlsl"

struct VS_USER_INTERFACE_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 PSUserInterface(VS_USER_INTERFACE_OUTPUT input) : SV_TARGET
{
    float4 cAlbedoColor = gMaterial.m_cAlbedo;;
    
    if(gnTexturesMask & MATERIAL_ALBEDO_MAP)
        cAlbedoColor = AlbedoTexture.Sample(gssWrap, input.uv);
    
    return float4(cAlbedoColor);
}
