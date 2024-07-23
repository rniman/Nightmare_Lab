#include "Common.hlsl"

struct GS_PARTICLE_DRAW_OUTPUT
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXTURE;
    uint type : PARTICLETYPE;
};

float4 PSParticleDraw(GS_PARTICLE_DRAW_OUTPUT input) : SV_TARGET
{
    float4 cColor = AlbedoTexture.Sample(gssWrap, input.uv);
    cColor *= input.color;

    return (cColor);
}
