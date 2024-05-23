#define MATERIAL_ALBEDO_MAP			0x01
//#define MATERIAL_SPECULAR_MAP		0x02
//#define MATERIAL_NORMAL_MAP			0x04
//#define MATERIAL_METALLIC_MAP		0x08
//#define MATERIAL_EMISSION_MAP		0x10
//#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
//#define MATERIAL_DETAIL_NORMAL_MAP	0x40

SamplerState gssWrap : register(s0);
Texture2D AlbedoTexture : register(t0);

struct MATERIAL
{
    float4 m_cAmbient;
    float4 m_cAlbedo;
    float4 m_cSpecular; //a = power
    float4 m_cEmissive;
};

cbuffer cbGameObjectInfo : register(b1)
{
    matrix gmtxGameObject : packoffset(c0);
    MATERIAL gMaterial : packoffset(c4);
    uint gnTexturesMask : packoffset(c8);
};

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
