#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

#define FRAME_BUFFER_WIDTH 1600
#define FRAME_BUFFER_HEIGHT 1024

struct MATERIAL
{
    float4 m_cAmbient;
    float4 m_cAlbedo;
    float4 m_cSpecular; //a = power
    float4 m_cEmissive;
};

cbuffer cbCameraInfo : register(b0)
{
    matrix gmtxView : packoffset(c0);
    matrix gmtxProjection : packoffset(c4);
    matrix gmtxInverseViewProjection : packoffset(c8);
    float4 gvCameraPosition : packoffset(c12); // 16바이트를 맞추기 위해 w값: 더미 추가
    float4 gvFogColor : packoffset(c13);
    float4 gvfFogInfo : packoffset(c14); // START, RANGE, Density, MOD
};

cbuffer cbGameObjectInfo : register(b1)
{
    matrix gmtxGameObject : packoffset(c0);
    MATERIAL gMaterial : packoffset(c4);
    uint gnTexturesMask : packoffset(c8);
};

#include "light.hlsl"

SamplerState gssWrap : register(s0);
SamplerState gssWrapPoint : register(s3);

Texture2D AlbedoTexture : register(t0);
Texture2D SpecularTexture : register(t1);
Texture2D NormalTexture : register(t2);
Texture2D MetallicTexture : register(t3);
Texture2D EmissionTexture : register(t4);

Texture2D<float> DFzDepthTexture : register(t7);

struct VS_STANDARD_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float3 tangentW : TANGENT;
    float3 bitangentW : BITANGENT;
    float2 uv : TEXCOORD;
};

float4 PSTransparent(VS_STANDARD_OUTPUT input) : SV_Target
{
    float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
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
    
    float4 cColor = (cAlbedoColor * 0.7f) + (cSpecularColor * 0.2f) + (cMetallicColor * 0.05f) + (cEmissionColor * 0.05f);
    
    float3 vCameraPosition = gvCameraPosition.xyz;
    float3 vPostionToCamera = vCameraPosition - input.positionW;
    float fDistanceToCamera = length(vPostionToCamera);
    
    float fFogFactor = saturate(1.0f / pow(gvfFogInfo.y + gvfFogInfo.x, pow(fDistanceToCamera * gvfFogInfo.z, 2)));
    cColor = lerp(gvFogColor, cColor, fFogFactor);
    
    float2 uvP = input.position.xy / float2(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
    float depth = DFzDepthTexture.Sample(gssWrap, uvP);
    clip(depth.x - input.position.z);
    
    
    float4 light = Lighting(input.positionW, input.normalW);
    
    cColor = (light * cColor);
    cColor.w *= 0.5f; // 알파값 조절 
    return cColor;
}
