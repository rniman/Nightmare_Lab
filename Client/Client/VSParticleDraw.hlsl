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

struct VS_PARTICLE_DRAW_OUTPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
    float size : SCALE;
    uint type : PARTICLETYPE;
    uint id : PARTICLEID;
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
        t += lerp(0.0f, speed, attractValue);
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

VS_PARTICLE_DRAW_OUTPUT Explosion(VS_PARTICLE_INPUT input, float fTime)
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
        output.color = float4(0.45f, 0.0f, 0.0f, lerp(0.0f, 1.0f, clamp(input.lifetime - gfCurrentTime, 0.0f, 1.0f)));
        output.position = input.position + input.velocity * t;
    }
    output.type = input.type;
    
    output.size = 0.04f;
	
    return (output);
}
