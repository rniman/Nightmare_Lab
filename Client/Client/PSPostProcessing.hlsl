#include "Common.hlsl"
#include "Light.hlsl"

float4 PSPostProcessing(PS_POSTPROCESSING_OUT input) : SV_Target
{
    float4 cColor = DFTextureTexture.Sample(gssWrap, input.uv);
    float3 normal = DFNormalTexture.Sample(gssWrap, input.uv).rgb;
    float4 position = DFPositionTexture.Sample(gssWrap, input.uv);
    //return cColor;
    float depth = DFzDepthTexture.Sample(gssWrap, input.uv);
    //return float4(depth, depth, depth, 1.0f);
    //return ShadowMapTexture[4].Sample(gssWrap, input.uv); // 뎁스 저장되는지 확인
    
    // (0~ 1 -> -1~1)
    normal = (normal.xyz - 0.5f) * 2.0f; // 노말 렌더링으로 확인해볼수 있는 문제가 있으므로 일단 최종렌더링에서 변환작업함.
    //return float4(normal,1.0f);
    float4 light = Lighting(position, normal);
    
    float3 vCameraPosition = gvCameraPosition.xyz;
    float3 vPostionToCamera = vCameraPosition - position.xyz;
    float fDistanceToCamera = length(vPostionToCamera);
    float fFogFactor = saturate(1.0f / pow(gvfFogInfo.y + gvfFogInfo.x, pow(fDistanceToCamera * gvfFogInfo.z, 2)));
    cColor = lerp(gvFogColor, cColor * light, fFogFactor);
    
    return (cColor);
}