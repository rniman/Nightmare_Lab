#pragma once
#include "Object.h"

enum PlayerState
{
	IDLE,
	WALK,
	RUN
};

class CAnimationController;

class CBlueSuitAnimationController : public CAnimationController
{
public:
	CBlueSuitAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, const shared_ptr<CLoadedModelInfo>& pModel);
	~CBlueSuitAnimationController() {};

	virtual void OnRootMotion(CGameObject* pRootGameObject) {}
	virtual void AdvanceTime(float fElapsedTime, CGameObject* pRootGameObject);

	void BlendAnimation(int nTrack1, int nTrack2, float fElapsedTime, float fBlentWeight);

private:
	int m_nStartLArm = -1;
	int m_nEndLArm = -1;

	int m_nStartRArm = -1;
	int m_nEndRArm = -1;

	int m_nStartSpine = -1;
	int m_nStartNeck = -1;
	int m_nEndNeck = -1;
};

class CZombieAnimationController : public CAnimationController
{
public:
	CZombieAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, const shared_ptr<CLoadedModelInfo>& pModel);
	~CZombieAnimationController() {};

	virtual void OnRootMotion(CGameObject* pRootGameObject) {}
	virtual void AdvanceTime(float fElapsedTime, CGameObject* pRootGameObject);
	void BlendAnimation(int nTrack1, int nTrack2, float fElapsedTime, float fBlentWeight);

private:
	int m_nStartSpine = -1;
	int m_nStartNeck = -1;
	int m_nEndNeck = -1;
	int m_nEndSpine = -1;
};
