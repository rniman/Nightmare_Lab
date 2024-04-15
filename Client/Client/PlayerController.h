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

	//플레이어가 들고 있는 플레시라이트의 월드변환정보를 반환
	int GetBoneFrameIndexToFlashLight() { return m_nPlayerFlashLight; }
	int GetBoneFrameIndexToRightHandRaiderItem() { return m_nRaiderItem; }
	void SetElbowPitch(float value);
	int GetBoneFrameIndex(char* s);
private:
	int m_nStartLArm = -1;
	int m_nEndLArm = -1;

	int m_nStartRArm = -1;
	int m_nEndRArm = -1;

	int m_nStartSpine = -1;
	int m_nStartNeck = -1;
	int m_nEndNeck = -1;

	int m_nPlayerFlashLight = -1;
	int m_nRaiderItem = -1;

	int m_nElbow_L = -1;
	int m_nHead_M = -1;

	// 팔의 각도
	float m_fElbowPitch = 0.0f;
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
