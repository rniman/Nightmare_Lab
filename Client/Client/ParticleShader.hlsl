#include "Common.hlsl"

struct PARTICLE_INS_INFO
{
    float start_Time;
    float3 padding;
};

// cbuffer 4byte * 4096 max
#define PARITICLE_INSTANCE_COUNT 1024

#define TP 0
#define SPARK 1
#define BUBBLE 2
#define BUBBLE_C 11
#define FOOTPRINT 3

cbuffer cbParticleInfo : register(b3)
{
    PARTICLE_INS_INFO particle_ins_info[PARITICLE_INSTANCE_COUNT];
}

//Texture2D gtxtParticleTexture : register(t1); => Albedo
Buffer<float4> gRandomBuffer : register(t10);
Buffer<float4> gRandomSphereBuffer : register(t11);

//SamplerState gWrapSamplerState : register(s0); => gssWrap


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

float3 GetParticleColor(float fAge, float fLifetime)
{
    float3 cColor = float3(1.0f, 1.0f, 1.0f);

    if (fAge == 0.0f)
        cColor = float3(0.0f, 1.0f, 0.0f);
    else if (fLifetime == 0.0f) 
        cColor = float3(1.0f, 1.0f, 0.0f);
    else
    {
        float t = fAge / fLifetime;
        cColor = lerp(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 1.0f), t * 1.0f);
    }

    return (cColor);
}

float4 RandomDirection(float fOffset)
{
    int u = uint(gfCurrentTime + fOffset + frac(gfCurrentTime) * 1000.0f) % 1024;
    return (normalize(gRandomBuffer.Load(u)));
}

float4 RandomDirectionOnSphere(float fOffset)
{
    int u = uint(gfCurrentTime + fOffset + frac(gfCurrentTime) * 1000.0f) % 256;
    return (normalize(gRandomSphereBuffer.Load(u)));
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
            BubbleCreate(particle, output, 32);
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


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
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

static float3 g_Gravity = float3(0.f, -0.9f, 0.f);

VS_PARTICLE_DRAW_OUTPUT Velocity(VS_PARTICLE_INPUT input)
{
    float3 g_AttractPos = input.iStartPos;
    
    VS_PARTICLE_DRAW_OUTPUT output;
    output.color = float4(0.1f, 0.1f, 1.0f, 1.0f);
    VS_PARTICLE_INPUT particle = input;
    float3 newPosition = input.iStartPos;
    
    float t = gfCurrentTime - input.iStartTime;
    
    if (t > 1.5f)
    {
        float t2 = gfCurrentTime - 1.5f - input.iStartTime;
        t2 = (particle.lifetime) * frac(t2 / (particle.lifetime));
        float attractValue = frac(t2 / (particle.lifetime));
        
        float speed = lerp(3.0f, 100.0f, attractValue);
        t += lerp(0.0f,speed, attractValue);
    }
    
    if (t > particle.lifetime)
    {
        newPosition.x = 100000;
        output.position = newPosition;
        return output;
    }
    
    if (t > 0)
    {
        t = particle.lifetime * frac(t / particle.lifetime);
        float attractValue = frac(t / particle.lifetime);
        float tt = t * t;
        newPosition.xyz = newPosition.xyz + particle.velocity.xyz * t + 0.5 * g_Gravity * tt;
        newPosition.xyz = lerp(newPosition.xyz, g_AttractPos, attractValue);
        output.color.a = lerp(0.0f, 1.0f, attractValue);
    }
    
    if (length(newPosition - g_AttractPos) > 2.5f)
    {
        newPosition.x = 100000;
        output.position = newPosition;
        return output;
    }
    
    particle.position = newPosition;
    output.position = newPosition;
    return output;
}

VS_PARTICLE_DRAW_OUTPUT Explosion(VS_PARTICLE_INPUT input,float fTime)
{
    // 시간에 따라 위치 업데이트
    input.velocity = normalize(input.velocity);
    float3 newPosition = float3(0.0f, 0.0f, 0.0f) + input.velocity * 0.5f * fTime;

    // 원형 폭발 효과
    float3 radialDir = normalize(newPosition);
    float radius = length(newPosition);
    float3 explosionPos = radialDir * radius * (1.0 + fTime);
    
    VS_PARTICLE_DRAW_OUTPUT output;
    output.position = explosionPos + input.iStartPos;
    
    // 거리 값을 0에서 1 사이로 정규화
    float nor = saturate((length(explosionPos - float3(0.0f, 0.0f, 0.0f)) - 0.0f) / (3.0f - 0.0f));
    
    output.color = float4(float3(0.1f, 0.1f, 0.7f), lerp(1.0f, 0.0f, nor));
    return output;
}

VS_PARTICLE_DRAW_OUTPUT TEST1(VS_PARTICLE_INPUT input)// 원 형태로 위로 올라가는 식
{
    VS_PARTICLE_DRAW_OUTPUT output;
    
    output.color = float4(0.0f, 1.0f, 1.0f, 1.0f);
    
    output.position.x = 100.0f * cos(input.velocity.x * gfCurrentTime);
    output.position.y = 1.0f * gfCurrentTime;
    output.position.z = 100.0f * sin(input.velocity.x * gfCurrentTime);
    
    return output;
}

VS_PARTICLE_DRAW_OUTPUT ITEMGetEffect(VS_PARTICLE_INPUT input)
{
    VS_PARTICLE_DRAW_OUTPUT output;
    
    output.color = float4(0.0f, 1.0f, 0.0f, 1.0f);
    float3 newPosition = input.position;
    float timeDelay = 0.05f;
    newPosition.y += (gfCurrentTime - input.iStartTime) * abs(input.velocity.y) * timeDelay;
    newPosition.x += (gfCurrentTime - input.iStartTime) * input.velocity.x * timeDelay;
    newPosition.z += (gfCurrentTime - input.iStartTime) * input.velocity.z * timeDelay;
    
    float nor = saturate((length(newPosition - float3(0.0f, 0.0f, 0.0f)) - 0.0f) / (0.5f - 0.0f));
    output.color.a = lerp(1.0f, 0.0f, nor);
    output.position = newPosition + input.iStartPos;
    
    return output;
}

VS_PARTICLE_DRAW_OUTPUT VSParticleDraw(VS_PARTICLE_INPUT input)
{
    VS_PARTICLE_DRAW_OUTPUT output;
    if (input.type == TP)
    {
        //output.color = float4(0.1f, 1.0f, 0.1f, 1.0f);
        //VS_PARTICLE_INPUT particle = input;
        //float3 newPosition = input.iStartPos;
        //output.position = newPosition;
        if (gfCurrentTime - input.iStartTime < 3.0f)
        {
            output = Velocity(input);
        }
        else
        {
            output = Explosion(input, gfCurrentTime - input.iStartTime - 3.0f);
        }
    }
    else if (input.type == SPARK)
    {
        output = ITEMGetEffect(input);
    }
    else if (input.type == BUBBLE || input.type == BUBBLE_C)
    {
        output.color = float4(0.4f, 0.4f, 0.4f, lerp(0.3f, 0.0f, length(input.position / 0.5f)));
        output.position = input.iStartPos + input.position;
    }
    else if (input.type == FOOTPRINT)
    {
        float visibleTime = 2.0f;
        float t = gfCurrentTime - input.lifetime + visibleTime;
        output.color = float4(0.65f, 0.0f, 0.0f, lerp(0.0f, 1.0f, clamp(input.lifetime - gfCurrentTime, 0.0f, 1.0f)));
        output.position = input.position + input.velocity * t;
    }
    
    output.type = input.type;
    
    output.size = 0.04f;
	
    return (output);
}

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

float4 PSParticleDraw(GS_PARTICLE_DRAW_OUTPUT input) : SV_TARGET
{
    float4 cColor = AlbedoTexture.Sample(gssWrap, input.uv);
    cColor *= input.color;

    return (cColor);
}
