#include "Common.hlsl"
#include "Light.hlsl"

float doAmbientOcclusion(float2 tcoord, float2 uv, float3 p, float3 cnorm)
{
    //float scale = 2.0f;
    //float bias = 0.01f;
    //float intesity = 5.0f;
    
    // ���ø� ��ġ ���
    float3 samplePos = mul(DFPositionTexture.Sample(gssWrap, tcoord + uv), gmtxView).xyz;
    float3 diff = samplePos - p;
    float3 v = normalize(diff);
    float d = length(diff) * gfScale;
    
    // Ambient Occlusion �� ��ȯ
    // ���� ���� cnorm�� ���� ���� v�� ���� ������ ���̾�� �� ���� ����Ͽ� occlusion ���� ���
    // �Ÿ� d�� ���� 1 / (1 + d)�� ���踦 �����ϰ� ���� �� intesity�� ���Ͽ� ���� occlusion ���� ��ȯ
    return max(0.0, dot(cnorm, v) - gfBias) * (1.0 / (1.0 + d)) * gfIntesity;
}

// ���ø� ���� �迭
const float2 vec[16] =
{
    float2(1, 0), float2(-1, 0), float2(0, 1), float2(0, -1),
    float2(0.707, 0.707), float2(-0.707, -0.707), float2(-0.707, 0.707), float2(0.707, -0.707),
    normalize(float2(0.25, 0.75)), normalize(float2(0.25, -0.75)), normalize(float2(-0.25, -0.75)), normalize(float2(-0.25, 0.75)),
    normalize(float2(0.75, 0.25)), normalize(float2(0.75, -0.25)), normalize(float2(-0.75, -0.25)), normalize(float2(-0.75, 0.25))
};

float4 PSPostProcessingWithSSAO(PS_POSTPROCESSING_OUT input) : SV_Target
{
    float4 cColor = DFTextureTexture.Sample(gssWrap, input.uv);
   
    float4 positionW = DFPositionTexture.Sample(gssWrap, input.uv);
    float4 positionV = mul(positionW, gmtxView);    // �� ���� ��ġ
    
    float3 normal = DFNormalTexture.Sample(gssWrap, input.uv).xyz;
    normal = normalize((normal * 2.0f) - 1.0f);

    float3x3 viewMatrixRotation = (float3x3) gmtxView;
    float3 normalV = normalize(mul(normal, viewMatrixRotation));    // �� ���� �븻
    
    // ������ �ؽ��� UV
    float2 vFrameBuffer = float2((float) FRAME_BUFFER_WIDTH, (float) FRAME_BUFFER_HEIGHT);
    float2 vNoiseSize = float2(64, 64);
    float2 randUV = vFrameBuffer * input.uv / vNoiseSize + float2(frac(time).xx);   // �ð� �� �޾Ƽ� �������� �ٸ���
        
    // ���� ���� ���
    float2 rand = normalize(AlbedoTexture.Sample(gssWrap, randUV).xy * 2.0f - 1.0f);
    float ssao = 0.0f;
    float rad = 0.5f / positionV.z;
    
    //**SSAO Calculation**//
    int numSamples = 16;
    for(int j = 0; j < numSamples; ++j)
    {
        float2 coord1 = reflect(vec[j], rand) * rad;
        float2 coord2 = float2(coord1.x * 0.707 - coord1.y * 0.707, coord1.x * 0.707 + coord1.y * 0.707);
        ssao += doAmbientOcclusion(input.uv, coord1 * 0.25, positionV.xyz, normalV);
        ssao += doAmbientOcclusion(input.uv, coord2 * 0.5, positionV.xyz, normalV);
        ssao += doAmbientOcclusion(input.uv, coord1 * 0.75, positionV.xyz, normalV);
        ssao += doAmbientOcclusion(input.uv, coord2, positionV.xyz, normalV);
    }
    ssao /= (float) numSamples * 4.0;
    ssao = 1.0f - ssao * 0.5f; // [0.5 ~ 1.0]
    
    // ���̿� ����� ���� ����
    float depth = DFzDepthTexture.Sample(gssWrap, input.uv);
    float4 positionP = mul(positionV, gmtxProjection);
    depth = saturate(depth / positionP.w);
    float attenuation = saturate(pow(depth, 2.f)) * 0.3f;
    ssao = saturate(ssao + attenuation);
    
    //**Light Calculation**//
    float4 light = Lighting(positionW.xyz, normal, float4(ssao, ssao, ssao, 1.0f));
    cColor = cColor * light; // SSAO�� ����Ʈ�� ���Ͽ� ���� ���� ���
    
    //**FOG Calculation**//
    float3 vCameraPosition = gvCameraPosition.xyz;
    float3 vPostionToCamera = vCameraPosition - positionW.xyz;
    float fDistanceToCamera = length(vPostionToCamera);
    float fFogFactor = saturate(1.0f / pow(gvfFogInfo.y + gvfFogInfo.x, pow(fDistanceToCamera * gvfFogInfo.z, 2))) * gvfFogInfo.w;
    cColor = lerp(gvFogColor, cColor, fFogFactor);
    
    return (cColor);
}
