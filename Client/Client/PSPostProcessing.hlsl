#include "Common.hlsl"
#include "Light.hlsl"

float4 PSPostProcessing(PS_POSTPROCESSING_OUT input) : SV_Target
{    
    float4 cColor = DFTextureTexture.Sample(gssWrap, input.uv);
    float4 cEmissiveColor = DFTextureEmissive.Sample(gssWrap, input.uv);
        
    float4 positionW = DFPositionTexture.Sample(gssWrap, input.uv);
    
    float3 normal = DFNormalTexture.Sample(gssWrap, input.uv).xyz;
    normal = normalize((normal * 2.0f) - 1.0f);
    
    float3x3 viewMatrixRotation = (float3x3) gmtxView;
    float3 normalV = normalize(mul(normal, viewMatrixRotation)); // 뷰 공간 노말
    
    //**Light Calculation**//
    float4 light = Lighting(positionW.xyz, normal);
    cColor = cColor * light; // SSAO와 라이트를 곱하여 최종 색상 계산
    
    cColor += cEmissiveColor;
    
    //**FOG Calculation**//
    //float3 vCameraPosition = gvCameraPosition.xyz;
    //float3 vPostionToCamera = vCameraPosition - positionW.xyz;
    //float fDistanceToCamera = length(vPostionToCamera);
    //float fFogFactor = saturate(1.0f / pow(gvfFogInfo.y + gvfFogInfo.x, pow(fDistanceToCamera * gvfFogInfo.z, 2))) * gvfFogInfo.w;
    //cColor = lerp(gvFogColor, cColor, fFogFactor);
    
    return cColor;
}