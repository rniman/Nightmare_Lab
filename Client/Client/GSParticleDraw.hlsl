#include "Common.hlsl"

struct VS_PARTICLE_DRAW_OUTPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
    float size : SCALE;
    uint type : PARTICLETYPE;
    uint id : PARTICLEID;
};

struct GS_PARTICLE_DRAW_OUTPUT
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXTURE;
    uint type : PARTICLETYPE;
};

static float3 gViewPosition2[4] = { float3(-1.0f, +1.0f, 0.5f), float3(+1.0f, +1.0f, 0.5f), float3(-1.0f, -1.0f, 0.5f), float3(+1.0f, -1.0f, 0.5f) };
static float2 gRectUVs[4] = { float2(0.0f, 0.0f), float2(1.0f, 0.0f), float2(0.0f, 1.0f), float2(1.0f, 1.0f) };

[maxvertexcount(4)]
void GSParticleDraw(point VS_PARTICLE_DRAW_OUTPUT input[1], inout TriangleStream<GS_PARTICLE_DRAW_OUTPUT> outputStream)
{
    GS_PARTICLE_DRAW_OUTPUT output;

    output.type = input[0].type;
    output.color = input[0].color;
    for (int i = 0; i < 4; i++)
    {
        float3 positionW = mul(gViewPosition2[i] * input[0].size, (float3x3) gmtxInverseView) + input[0].position;
        output.position = mul(mul(float4(positionW, 1.0f), gmtxView), gmtxProjection);
        output.uv = gRectUVs[i];

        outputStream.Append(output);
    }
}