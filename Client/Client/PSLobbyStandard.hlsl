#include "Common.hlsl"
#include "Light.hlsl"

struct VS_SKINNED_STANDARD_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    int4 indices : BONEINDEX;
    float4 weights : BONEWEIGHT;
};

float4 PSLobbyStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
    float4 cAlbedoColor = gMaterial.m_cAlbedo;
    float4 cSpecularColor = gMaterial.m_cSpecular;
    float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cEmissionColor = gMaterial.m_cEmissive;
    
    if(gnTexturesMask & MATERIAL_ALBEDO_MAP)
        cAlbedoColor = AlbedoTexture.Sample(gssWrap, input.uv);
    if(gnTexturesMask & MATERIAL_SPECULAR_MAP)
        cSpecularColor = SpecularTexture.Sample(gssWrap, input.uv);
    if(gnTexturesMask & MATERIAL_NORMAL_MAP)
        cNormalColor = NormalTexture.Sample(gssWrap, input.uv);
    if(gnTexturesMask & MATERIAL_METALLIC_MAP)
        cMetallicColor = MetallicTexture.Sample(gssWrap, input.uv);
    if(gnTexturesMask & MATERIAL_EMISSION_MAP)
        cEmissionColor = EmissionTexture.Sample(gssWrap, input.uv);

    float4 cIllumination = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 cColor = cAlbedoColor + cSpecularColor + cEmissionColor;
    if(gnTexturesMask & MATERIAL_NORMAL_MAP)
    {
        float3 normalW = input.normalW;
        float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
        float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] ¡æ [-1, 1]
        normalW = normalize(mul(vNormal, TBN));
        cIllumination = Lighting(input.positionW, normalW);
        cColor = lerp(cColor, cIllumination, 0.5f);
    }

    return cColor;
}