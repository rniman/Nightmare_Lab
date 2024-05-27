#pragma once
#include "Object.h"

enum PlayerState
{
	IDLE,
	WALK,
	RUN,
	DEATH
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
	void TransitionBlueSuitPlayer(float fElapsedTime);
	void TransitionIDLEtoWALK(float fElapsedTime, int nTransitionIndex);
	void TransitionWALKtoIDLE(float fElapsedTime, int nTransitionIndex);
	void TransitionWALKtoRUN(float fElapsedTime, int nTransitionIndex);
	void TransitionRUNtoWALK(float fElapsedTime, int nTransitionIndex);
	void TransitionToDEATH(float fElapsedTime, int nTransitionIndex);

	void BlendLeftArm(float fElapsedTime);

	//플레이어가 들고 있는 플레시라이트의 월드변환정보를 반환
	int GetBoneFrameIndexToFlashLight() { return m_nPlayerFlashLight; }
	int GetBoneFrameIndexToRightHandRaderItem() { return m_nRaderItem; }
	int GetBoneFrameIndexToRightHandTeleportItem() { return m_nTeleportItem; }

	void SetElbowPitch(float value);
	int GetBoneFrameIndex(char* s);
	void SetSelectItem(bool b) { m_bSelectItem = b; }

	void CalculateRightHandMatrix();

	XMFLOAT4X4 m_xmf4x4RightHandRotate;
private:
	int m_nWrist_L = -1;
	int m_nShoulder_L = -1;

	int m_nChest_M = -1;

	int m_nStartLArm = -1;
	int m_nEndLArm = -1;

	int m_nStartRArm = -1;
	int m_nEndRArm = -1;

	int m_nStartSpine = -1;
	int m_nStartNeck = -1;
	int m_nEndNeck = -1;

	int m_nPlayerFlashLight = -1;
	int m_nRaderItem = -1;
	int m_nTeleportItem = -1;

	int m_nElbow_L = -1;
	int m_nElbow_R = -1;
	int m_nHead_M = -1;

	// 팔의 각도
	float m_fLElbowPitch = 0.0f;
	float m_fLShoulderPitch = 0.0f;
	
	bool m_bSelectItem = false;
};

class CZombieAnimationController : public CAnimationController
{
public:
	CZombieAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, const shared_ptr<CLoadedModelInfo>& pModel);
	~CZombieAnimationController() {};

	virtual void OnRootMotion(CGameObject* pRootGameObject) {}
	virtual void AdvanceTime(float fElapsedTime, CGameObject* pRootGameObject);
	void BlendAnimation(int nTrack1, int nTrack2, float fElapsedTime, float fBlentWeight);
	void TransitionZombiePlayer(float fElapsedTime);
	void TransitionIDLEtoWALK(float fElapsedTime, int nTransitionIndex);
	void TransitionWALKtoIDLE(float fElapsedTime, int nTransitionIndex);

private:
	int m_nStartSpine = -1;
	int m_nStartNeck = -1;
	int m_nEndNeck = -1;
	int m_nEndSpine = -1;
};
