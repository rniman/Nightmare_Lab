#include "Common.hlsl"
RWTexture2D<float4> gtxtRWOutput : register(u0);

[numthreads(32, 32, 1)]
void CSBloomOff(uint3 n3DispatchThreadID : SV_DispatchThreadID)
{
    float4 finalColor = DFLightTexture[n3DispatchThreadID.xy];
    float4 positionW = DFPositionTexture[n3DispatchThreadID.xy];

    float3 vCameraPosition = gvCameraPosition.xyz;
    float3 vPostionToCamera = vCameraPosition - positionW.xyz;
    float fDistanceToCamera = length(vPostionToCamera);
    float fFogFactor = saturate(1.0f / pow(gvfFogInfo.y + gvfFogInfo.x, pow(fDistanceToCamera * gvfFogInfo.z, 2))) * gvfFogInfo.w;
    finalColor = lerp(gvFogColor, finalColor, fFogFactor);
    
    gtxtRWOutput[n3DispatchThreadID.xy] = finalColor;
}