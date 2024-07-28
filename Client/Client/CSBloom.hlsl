#include "Common.hlsl"
RWTexture2D<float4> gtxtRWOutput : register(u0);

//groupshared float4 sharedMemory[32 * 32]; // 예시: 32x32 쓰레드 그룹 크기

[numthreads(32, 32, 1)]
void CSBloom(uint3 n3DispatchThreadID : SV_DispatchThreadID,
                    uint3 groupThreadID : SV_GroupThreadID,
                    uint3 groupID : SV_GroupID /*uint3 n3DispatchThreadID : SV_DispatchThreadID*/)
{
    //int flattenedIndex = groupThreadID.x + 32 * groupThreadID.y;
    //sharedMemory[flattenedIndex] = DFTextureEmissive[n3DispatchThreadID.xy];
    //GroupMemoryBarrierWithGroupSync();
    
    float4 finalColor = DFLightTexture[n3DispatchThreadID.xy];
    float4 positionW = DFPositionTexture[n3DispatchThreadID.xy];  
    
    float3 vCameraPosition = gvCameraPosition.xyz;
    float3 vPostionToCamera = vCameraPosition - positionW.xyz;
    float fDistanceToCamera = length(vPostionToCamera);
    float fFogFactor = saturate(1.0f / pow(gvfFogInfo.y + gvfFogInfo.x, pow(fDistanceToCamera * gvfFogInfo.z, 2))) * gvfFogInfo.w;
    finalColor = lerp(gvFogColor, finalColor, fFogFactor);
    
    // 블러링을 적용하여 블룸 효과를 생성   
    float4 blurredColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    int radius = lerp(15, 1, saturate(fDistanceToCamera / 35.0f)); // 블러링 반경
    int sampleCount = 0;
    for(int x = -radius; x <= radius; x++)
    {
        for(int y = -radius; y <= radius; y++)
        {
            int2 samplePos = int2(n3DispatchThreadID.xy) + int2(x, y);
            // 텍스처 경계를 넘지 않도록 클램프
            samplePos = clamp(samplePos, int2(0, 0), int2(FRAME_BUFFER_WIDTH - 1, FRAME_BUFFER_HEIGHT - 1));
            blurredColor += DFTextureEmissive[samplePos];
            
            //int sharedIndex = clamp(groupThreadID.x + x, 0, 31) + 32 * clamp(groupThreadID.y + y, 0, 31);
            //blurredColor += sharedMemory[sharedIndex];
            sampleCount++;
        }
    }
    
    if(sampleCount != 0)
        blurredColor /= sampleCount;
    if (length(blurredColor.xyz) < 1.f)
    {
        finalColor *= (1.0f - length(blurredColor.xyz));
        finalColor += blurredColor;
    }
    else
    {
        finalColor *= blurredColor*2.0f;
    }
    
    gtxtRWOutput[n3DispatchThreadID.xy] = finalColor;
}