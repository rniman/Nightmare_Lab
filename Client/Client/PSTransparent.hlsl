#include "Common.hlsl"
#include "light.hlsl"

float4 PSTransparent(VS_STANDARD_OUTPUT input) : SV_Target
{
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
    if (gnTexturesMask & MATERIAL_NORMAL_MAP)
    {
        cNormalColor = NormalTexture.Sample(gssWrap, input.uv);
        cNormalColor = (cNormalColor * 2.0f) - 1.0f;
        input.normalW = (cNormalColor.x * input.tangentW) + (cNormalColor.y * input.bitangentW) + (cNormalColor.z * input.normalW);
        input.normalW = normalize(input.normalW);
    }
    if(gnTexturesMask & MATERIAL_METALLIC_MAP)
        cMetallicColor = MetallicTexture.Sample(gssWrap, input.uv);
    if(gnTexturesMask & MATERIAL_EMISSION_MAP)
        cEmissionColor = EmissionTexture.Sample(gssWrap, input.uv);
    
    float4 cColor = (cAlbedoColor * 1.0f) + (cSpecularColor * 0.2f) + (cMetallicColor * 0.05f) + (cEmissionColor * 0.05f);
    
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
