//--------------------------------------------------------------------------------------
#define MAX_LIGHTS			28
#define MAX_MATERIALS		16 

#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3

#define _WITH_LOCAL_VIEWER_HIGHLIGHTING
#define _WITH_THETA_PHI_CONES
//#define _WITH_REFLECT

struct LIGHT
{
    float4x4				m_viewProjection;
	float4					m_cAmbient;
    float4					m_cAlbedo;
	float4					m_cSpecular;
    float3                  m_vPosition;
    bool                    m_bEnable;
	float 					m_fFalloff;
	float3					m_vDirection;
	float 					m_fTheta; //cos(m_fTheta)
	float3					m_vAttenuation;
	float					m_fPhi; //cos(m_fPhi)
	int 					m_nType;
	float					m_fRange;
	float					padding;
};

cbuffer cbLights : register(b2)
{
	LIGHT					gLights[MAX_LIGHTS];
	float4					gcGlobalAmbientLight;
	int						gnLights;
};

Texture2D<float> ShadowMapTexture[MAX_LIGHTS] : register(t20);
SamplerComparisonState gssComparisonPCFShadow : register(s2);

#define DELTA_X					(1.0f / FRAME_BUFFER_WIDTH)
#define DELTA_Y					(1.0f / FRAME_BUFFER_HEIGHT)

float4 DirectionalLight(int nIndex, float3 vNormal, float3 vToCamera)
{
	float3 vToLight = -gLights[nIndex].m_vDirection;
	float fDiffuseFactor = dot(vToLight, vNormal);
	float fSpecularFactor = 0.0f;
	if (fDiffuseFactor > 0.0f)
	{
		if (gMaterial.m_cSpecular.a != 0.0f)
		{
#ifdef _WITH_REFLECT
			float3 vReflect = reflect(-vToLight, vNormal);
			fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterial.m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
			float3 vHalf = normalize(vToCamera + vToLight);
#else
			float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
			fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a);
#endif
		}
	}

	return((gLights[nIndex].m_cAmbient * gMaterial.m_cAmbient) + (gLights[nIndex].m_cAlbedo * fDiffuseFactor * gMaterial.m_cAlbedo) + (gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterial.m_cSpecular));
}

float4 PointLight(int nIndex, float3 vPosition, float3 vNormal, float3 vToCamera)
{
	float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
	float fDistance = length(vToLight);
	if (fDistance <= gLights[nIndex].m_fRange)
	{
		float fSpecularFactor = 0.0f;
		vToLight /= fDistance;
		float fDiffuseFactor = dot(vToLight, vNormal);
		if (fDiffuseFactor > 0.0f)
		{
			if (gMaterial.m_cSpecular.a != 0.0f)
			{
#ifdef _WITH_REFLECT
				float3 vReflect = reflect(-vToLight, vNormal);
				fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterial.m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
				float3 vHalf = normalize(vToCamera + vToLight);
#else
				float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
				fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a);
#endif
			}
		}
		float fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance*fDistance));

		return(((gLights[nIndex].m_cAmbient * gMaterial.m_cAmbient) + (gLights[nIndex].m_cAlbedo * fDiffuseFactor * gMaterial.m_cAlbedo) + (gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterial.m_cSpecular)) * fAttenuationFactor);
	}
	return(float4(0.0f, 0.0f, 0.0f, 0.0f));
}


float SpotFactor(int nLightType,float3 vLight,float3 vToLight,float fTheta,float fPhi,float fFalloff)
{
    if (nLightType != SPOT_LIGHT)
    {
        return 1.0f;
    }
    vLight = normalize(vLight);
    vToLight = normalize(vToLight);
    float fRho = dot(-vToLight, vLight);
	if(fRho <= 0.0f)
    {
        return 0.0f;
    }
    if (fRho > fTheta)
    {
        return 1.0f;
    }
    if (fRho <= fPhi)
    {
        return 0.0f;
    }
    return (pow(((fPhi - fRho) / (fPhi - fTheta)), fFalloff));

}

float Shadowdecrease(int nIndex, float3 vPosition, float3 vToCamera)
{
    //if (nIndex > 0)
    {
        float4 uvs = mul(float4(vPosition, 1.0f), gLights[nIndex].m_viewProjection);
        
        float wPositionDepth = uvs.z / uvs.w; //«»ºø ±Ì¿Ã∏¶ ¿«πÃ«‘.
        //float shadow = ShadowMapTexture[1].SampleLevel(gssWrap, uvs.xy / uvs.ww, 0).r;
        wPositionDepth -= 0.0015f;
        float shadow = ShadowMapTexture[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uvs.xy / uvs.ww, wPositionDepth).x;
        shadow += ShadowMapTexture[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uvs.xy / uvs.ww + float2(-DELTA_X, 0.0f), wPositionDepth).x;
        shadow += ShadowMapTexture[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uvs.xy / uvs.ww + float2(+DELTA_X, 0.0f), wPositionDepth).x;
        shadow += ShadowMapTexture[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uvs.xy / uvs.ww + float2(0.0f, -DELTA_Y), wPositionDepth).x;
        shadow += ShadowMapTexture[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uvs.xy / uvs.ww + float2(0.0f, +DELTA_Y), wPositionDepth).x;
        shadow += ShadowMapTexture[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uvs.xy / uvs.ww + float2(-DELTA_X, -DELTA_Y), wPositionDepth).x;
        shadow += ShadowMapTexture[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uvs.xy / uvs.ww + float2(-DELTA_X, +DELTA_Y), wPositionDepth).x;
        shadow += ShadowMapTexture[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uvs.xy / uvs.ww + float2(+DELTA_X, -DELTA_Y), wPositionDepth).x;
        shadow += ShadowMapTexture[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uvs.xy / uvs.ww + float2(+DELTA_X, +DELTA_Y), wPositionDepth).x;
        return shadow / 9.0f;
    }
    return 1.0f;
}

float4 SpotLight(int nIndex, float3 vPosition, float3 vNormal, float3 vToCamera)
{    
    float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
  
    float fDistance = length(vToLight);
	
    if (fDistance <= gLights[nIndex].m_fRange)
    {
        float fSpecularFactor = 0.0f;
        vToLight = normalize(vToLight);
        float fDiffuseFactor = dot(vToLight, vNormal);

        if (fDiffuseFactor > 0.0f)
        {
            if (gMaterial.m_cSpecular.a != 0.0f)
            {
                float3 vHalf = normalize(vToCamera + vToLight);
                fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a);
            }
        }
        float fSpotFactor = SpotFactor(SPOT_LIGHT, gLights[nIndex].m_vDirection, vToLight, gLights[nIndex].m_fTheta, gLights[nIndex].m_fPhi, gLights[nIndex].m_fFalloff);
		
        float fAttenuationFactor = (1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance)));
		
        float distanceAttenuation = 1 - (fDistance / gLights[nIndex].m_fRange);
		
        return (((gLights[nIndex].m_cAmbient * gMaterial.m_cAmbient + 0.0f) + (gLights[nIndex].m_cAlbedo * fDiffuseFactor * gMaterial.m_cAlbedo) + (gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterial.m_cSpecular)) * fAttenuationFactor * distanceAttenuation * fSpotFactor);
    }
    return (float4(0.0f, 0.0f, 0.0f, 0.0f));
}

float4 Lighting(float3 vPosition, float3 vNormal)
{
	float3 vCameraPosition = float3(gvCameraPosition.x, gvCameraPosition.y, gvCameraPosition.z);
	float3 vToCamera = normalize(vCameraPosition - vPosition);
    
    float4 cColor = float4(0.05f, 0.05f, 0.05f, 0.0f);
	[unroll(MAX_LIGHTS)] for (int i = 0; i < gnLights; i++)
	{
		if (gLights[i].m_bEnable)
		{
            float fShadowFactor = 1.0f;
            fShadowFactor = Shadowdecrease(i, vPosition, vCameraPosition);
           
			if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
			{
				//cColor += DirectionalLight(i, vNormal, vToCamera);
			}
			else if (gLights[i].m_nType == POINT_LIGHT)
			{
				//cColor += PointLight(i, vPosition, vNormal, vToCamera);
			}
			else if (gLights[i].m_nType == SPOT_LIGHT)
            {
                cColor += SpotLight(i, vPosition, vNormal, vCameraPosition) * fShadowFactor;

            }
        }
	}
	cColor += (gcGlobalAmbientLight * gMaterial.m_cAmbient);
	cColor.a = gMaterial.m_cAlbedo.a;

	return(cColor);
}

