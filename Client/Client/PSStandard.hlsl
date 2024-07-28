#include "Common.hlsl"

struct PS_MULTIPLE_RENDER_TARGETS_OUTPUT
{
    float4 cTexture : SV_TARGET0;
    float4 cNormal : SV_TARGET1;
    float zDepth : SV_TARGET2;
    float4 position : SV_TARGET3;
    float4 cEmissive : SV_TARGET4;
    float4 cLight : SV_TARGET5;
};

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSStandard(VS_STANDARD_OUTPUT input)
{
    PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

    float4 cAlbedoColor = gMaterial.m_cAlbedo;
    float4 cSpecularColor = gMaterial.m_cSpecular;
    float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cEmissionColor = gMaterial.m_cEmissive;
    
    input.normalW = normalize(input.normalW);
    
    if(gnTexturesMask & MATERIAL_ALBEDO_MAP)
        cAlbedoColor = AlbedoTexture.Sample(gssWrap, input.uv);
    if(gnTexturesMask & MATERIAL_SPECULAR_MAP)
        cSpecularColor = SpecularTexture.Sample(gssWrap, input.uv);
    if (gnTexturesMask & MATERIAL_NORMAL_MAP)
    {
        cNormalColor = NormalTexture.Sample(gssWrap, input.uv); 
        cNormalColor = (cNormalColor * 2.0f) - 1.0f;
        input.normalW = (cNormalColor.x * input.tangentW) + (cNormalColor.y * input.bitangentW) + (cNormalColor.z * input.normalW);
        input.normalW = normalize(input.normalW);
    }
    if(gnTexturesMask & MATERIAL_METALLIC_MAP)
        cMetallicColor = MetallicTexture.Sample(gssWrap, input.uv);
    if (gnTexturesMask & MATERIAL_EMISSION_MAP)
    {
        cEmissionColor = EmissionTexture.Sample(gssWrap, input.uv) * (cEmissionColor * 2.0f);
    }
    
    float4 cColor = (cAlbedoColor * 0.795f) + (cSpecularColor * 0.2f) + (cMetallicColor * 0.05f)/* + (cEmissionColor)*/;
    
    if(usePattern > 0.0f)
    {
        float2 patternUV;
        patternUV.x = input.uv.x;
        patternUV.y = input.uv.y + frac(time);
    
        float4 patternColor = PatternTexture.Sample(gssWrap, patternUV);
        
        {
            cColor *= patternColor;
        }
        
        cEmissionColor += (patternColor * 0.5f);
    }
    
    output.cTexture = cColor;
    output.cNormal = float4(input.normalW.xyz * 0.5f + 0.5f, 1.0f);
    output.zDepth = input.position.z;
    output.position = float4(input.positionW, 1.0f);
    output.cEmissive = cEmissionColor;

    return output;
}