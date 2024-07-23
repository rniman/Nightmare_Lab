#define FRAME_BUFFER_WIDTH 1600
#define FRAME_BUFFER_HEIGHT 1024

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			256

#define MAX_LIGHTS			28

//Particle
#define TP 0
#define SPARK 1
#define BUBBLE 2
#define BUBBLE_C 11
#define FOOTPRINT 3


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
    matrix gmtxInverseView : packoffset(c8);
    matrix gmtxInverseViewProjection : packoffset(c12);
    float4 gvCameraPosition : packoffset(c16); // 16바이트를 맞추기 위해 w값: 더미 추가
    float4 gvFogColor : packoffset(c17);
    float4 gvfFogInfo : packoffset(c18); // START, RANGE, Density, MOD
};

struct OPTION
{
    float alphaValue;
    float3 padding;
};

cbuffer cbGameObjectInfo : register(b1)
{
    matrix gmtxGameObject : packoffset(c0);
    MATERIAL gMaterial : packoffset(c4);
    OPTION option : packoffset(c8);
    uint gnTexturesMask : packoffset(c9);
};

cbuffer cbBoneOffsets : register(b3)
{
    float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(b4)
{
    float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};


cbuffer cbFrameInfo : register(b5)
{
    float time : packoffset(c0.x);
    float localTime : packoffset(c0.y);
    float usePattern : packoffset(c0.z);
    float gfTrackingTime : packoffset(c0.w);
    
    // Occlusion Info
    float gfScale : packoffset(c1.x);
    float gfBias : packoffset(c1.y);
    float gfIntesity : packoffset(c1.z);
}

cbuffer cbFrameworkInfo : register(b6)
{
    float gfCurrentTime : packoffset(c0.x);
    float gfElapsedTime : packoffset(c0.y);
    float gfSecondsPerFirework : packoffset(c0.z);
    int gnFlareParticlesToEmit : packoffset(c0.w);
    float3 gf3Gravity : packoffset(c1.x);
    int gnMaxFlareType2Particles : packoffset(c1.w);
};

SamplerState gssWrap : register(s0);
SamplerState gssWrapPoint : register(s3);

Texture2D AlbedoTexture : register(t0);
Texture2D SpecularTexture : register(t1);
Texture2D NormalTexture : register(t2);
Texture2D MetallicTexture : register(t3);
Texture2D EmissionTexture : register(t4);

Texture2D DFTextureTexture : register(t5);
Texture2D DFNormalTexture : register(t6);
Texture2D<float> DFzDepthTexture : register(t7);
Texture2D DFPositionTexture : register(t8);

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

struct PS_POSTPROCESSING_OUT
{
    float4 position : SV_Position;
    float2 uv : UV0;
    float4 uvs[MAX_LIGHTS] : UV1; // 최적화가 필요.. 픽셀쉐이더에서 계산중인 빛의 viewProjection을 버텍스쉐이더에서 하도록 바꾸자.
};
