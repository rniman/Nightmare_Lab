#pragma once
#include "Object.h"

class CEnviromentObject
{
public:


private:

};

/// <CGameObject - CEnviromentObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CDoorObject>

class CDrawerObject : public CGameObject
{
public:
	CDrawerObject(char* pstrFrameName, XMFLOAT4X4& xmf4x4World, CMesh* pMesh);
	virtual ~CDrawerObject();

	virtual void SetOOBB() override;
	virtual void Animate(float fElapsedTime) override;
	virtual void AnimateOOBB() override;
	virtual void AnimatePicking(float fElapsedTime) override;
	virtual void CallbackPicking() override;
private:
	bool m_bOpened = false;
	bool m_bAnimate = false;
	XMFLOAT3 m_xmf3OriginPosition;
	XMFLOAT3 m_xmf3Forward;	
};

/// <CGameObject - CDrawerObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CDoorObject>

class CDoorObject : public CGameObject
{
public:
	CDoorObject(char* pstrFrameName, XMFLOAT4X4& xmf4x4World, CMesh* pMesh);
	virtual ~CDoorObject();

	virtual void SetOOBB() override;
	virtual void Animate(float fElapsedTime) override;
	virtual void AnimateOOBB() override;
	virtual void AnimatePicking(float fElapsedTime) override;
	virtual void CallbackPicking() override;

private:
	bool m_bOpened = false;
	float m_fDoorAngle = 0.0f;
	float m_fRotationAngle = 0.0f;

	XMFLOAT4 m_xmf4Quaternion = { 0.0f, 0.0f, 0.0f, 1.0f };
};