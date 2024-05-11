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

struct VS_BLEND_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

struct VS_BLEND_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float2 uv : TEXCOORD;
};

VS_BLEND_OUTPUT VSBlend(VS_BLEND_INPUT input)
{
    VS_BLEND_OUTPUT output;
    
    output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
    output.normalW = mul(input.normal, (float3x3) gmtxGameObject);
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;
    
    return output;
}