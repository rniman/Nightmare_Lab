#include "Common.hlsl"

struct PS_SHADOW_OUTPUT
{
    float depth : SV_Target0;
};

PS_SHADOW_OUTPUT PSShadow(VS_STANDARD_OUTPUT input)
{
    PS_SHADOW_OUTPUT output;
    output.depth = input.position.z;
    
    return output;
}
