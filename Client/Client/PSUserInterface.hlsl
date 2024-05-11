SamplerState gssWrap : register(s0);
Texture2D AlbedoTexture : register(t0);

struct VS_USER_INTERFACE_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 PSUserInterface(VS_USER_INTERFACE_OUTPUT input) : SV_TARGET
{
    float4 cAlbedoColor = float4(1.0f, 1.0f, 0.0f, 1.0f);
    
    cAlbedoColor = AlbedoTexture.Sample(gssWrap, input.uv);
    cAlbedoColor.b = 1.0f;
    return float4(cAlbedoColor);
}
