
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

VS_PARTICLE_INPUT VSParticleStreamOutput(VS_PARTICLE_INPUT input)
{
    return (input);
}