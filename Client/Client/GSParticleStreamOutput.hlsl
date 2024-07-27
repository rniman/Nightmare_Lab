#include "Common.hlsl"

struct VS_PARTICLE_INPUT
{
    float3 position : POSITION;
    float3 velocity : VELOCITY;
    float lifetime : LIFETIME;
    uint type : PARTICLETYPE;
    float startTime : STARTTIME;
    
    uint id : IPARTICLEID;
    float3 iStartPos : ISPOSITION;
    float iStartTime : ISTARTTIME;
};

Buffer<float4> gRandomBuffer : register(t12);

float4 RandomDirection(float fOffset)
{
    int u = uint(gfCurrentTime + fOffset + frac(gfCurrentTime) * 1000.0f) % 1024;
    return (normalize(gRandomBuffer.Load(u)));
}

void BubbleCreate(VS_PARTICLE_INPUT input, inout PointStream<VS_PARTICLE_INPUT> output, int amount)
{
    VS_PARTICLE_INPUT particle = input;
    particle.type = BUBBLE_C; // dummy type
    for (int j = 0; j < amount; j++)
    {
        float4 f4Random = RandomDirection(particle.type + particle.lifetime + length(particle.velocity) + j);
        particle.velocity = f4Random.xyz * 0.1f;
        particle.velocity.y = abs(particle.velocity.y);
        particle.lifetime = 3.0f + length(f4Random.x) * 3.f;
        particle.startTime = gfCurrentTime;
        output.Append(particle);
    }
}

[maxvertexcount(64)]
void GSParticleStreamOutput(point VS_PARTICLE_INPUT input[1], inout PointStream<VS_PARTICLE_INPUT> output)
{
    VS_PARTICLE_INPUT particle = input[0];
    particle.lifetime -= gfElapsedTime;
    
    if (particle.lifetime > 0.0f)
    {
        if (input[0].type == BUBBLE)
        {
            BubbleCreate(particle, output, 64);
        }
        else if (input[0].type == BUBBLE_C)
        {
            particle.position.xyz = (particle.velocity * frac((gfCurrentTime - particle.startTime) / 10) * 10);
		
            output.Append(particle);
        }
    }
    else
    {
        BubbleCreate(particle, output, 1);
    }
}
