#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			256

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

cbuffer cbBoneOffsets : register(b3)
{
    float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(b4)
{
    float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

struct VS_OUT_LINE_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    int4 indices : BONEINDEX;
    float4 weights : BONEWEIGHT;
};

struct VS_STANDARD_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float3 tangentW : TANGENT;
    float3 bitangentW : BITANGENT;
    float2 uv : TEXCOORD;
};

VS_STANDARD_OUTPUT VSOutLineMask(VS_OUT_LINE_INPUT input)
{
    VS_STANDARD_OUTPUT output;
    
    float4x4 mtxVertexToBoneWorld = (float4x4) 0.0f;
    for(int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
    {
        mtxVertexToBoneWorld += input.weights[i] * mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
    }
    output.positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
    output.normalW = mul(input.normal, (float3x3) mtxVertexToBoneWorld).xyz;
    output.tangentW = mul(input.tangent, (float3x3) mtxVertexToBoneWorld).xyz;
    output.bitangentW = mul(input.bitangent, (float3x3) mtxVertexToBoneWorld).xyz;
    
    //output.positionW -= output.normalW * 0.005f;

    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    
    output.uv = input.uv;

    return (output);
}
