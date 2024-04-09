#include "Shaders.hlsl"

struct PS_SHADOW_OUTPUT{
    float depth : SV_Target0;
};

PS_SHADOW_OUTPUT PS_Shadow(VS_STANDARD_OUTPUT input)
{
    PS_SHADOW_OUTPUT output;
    output.depth = input.position.z;
    
    return output;
}
