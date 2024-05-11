struct VS_USER_INTERFACE_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_USER_INTERFACE_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_USER_INTERFACE_OUTPUT VSUserInterface(VS_USER_INTERFACE_INPUT input)
{
    VS_USER_INTERFACE_OUTPUT output;

    //float4 positionW = float4(input.position, 1.0f); -> 나중에 UI추가될때 월드 하고 곱하기
    output.position = float4(input.position.xy, 0.0f, 1.0f);
    output.uv = input.uv;
    
    return output;
}