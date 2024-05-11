#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

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

cbuffer cbFrameInfo : register(b5)
{
    float time : packoffset(c0.x);
    float localTime : packoffset(c0.y);
    float usePattern : packoffset(c0.z);
    float gfTrackingTime : packoffset(c0.w);
}

SamplerState gssWrap : register(s0);
SamplerState gssWrapPoint : register(s3);

Texture2D AlbedoTexture : register(t0);
Texture2D SpecularTexture : register(t1);
Texture2D NormalTexture : register(t2);
Texture2D MetallicTexture : register(t3);
Texture2D EmissionTexture : register(t4);

Texture2D PatternTexture : register(t9);

struct VS_STANDARD_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float3 tangentW : TANGENT;
    float3 bitangentW : BITANGENT;
    float2 uv : TEXCOORD;
};

struct PS_MULTIPLE_RENDER_TARGETS_OUTPUT
{
    float4 cTexture : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float zDepth : SV_TARGET2;
    float4 position : SV_Target3;
};

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSStandard(VS_STANDARD_OUTPUT input)
{
    PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;
    
    //float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    //float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    //float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    //float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    //float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
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
    
    float4 cColor = (cAlbedoColor * 0.7f) + (cSpecularColor * 0.2f) + (cMetallicColor * 0.05f) + (cEmissionColor * 0.5f);
    
    if(usePattern > 0.0f)
    {
        float2 patternUV;
        patternUV.x = input.uv.x;
        patternUV.y = input.uv.y + frac(time);
    
        float4 patternColor = PatternTexture.Sample(gssWrap, patternUV);
        
        //if (patternColor.x >= 0.1f && patternColor.y >= 0.1f && patternColor.z >= 0.1f)
        {
            cColor *= patternColor;
        }
    }
    
    output.cTexture = cColor;
    input.normalW = normalize(input.normalW);
    output.normal = float4(input.normalW.xyz * 0.5f + 0.5f, 1.0f);
    output.zDepth = input.position.z;
    output.position = float4(input.positionW, 1.0f);
    
    return output;
}