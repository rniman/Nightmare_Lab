#include "stdafx.h"
#include "PlayerController.h"

CBlueSuitAnimationController::CBlueSuitAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, const shared_ptr<CLoadedModelInfo>& pModel)
	:CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pModel)
{
	// IDLE, WALK, RUN 3가지 상태
	m_nState = 3;
	m_nTransition = 4;

	m_vAnimationTransitions.reserve(m_nTransition);
	for (int i = 0; i < m_nTransition; ++i)
	{
		m_vAnimationTransitions.emplace_back();
	}

	m_vAnimationTransitions[0].SetTransitionAnimationTrack(0, 1);	// i -> w
	m_vAnimationTransitions[1].SetTransitionAnimationTrack(1, 0);	// w -> i
	m_vAnimationTransitions[2].SetTransitionAnimationTrack(1, 2);	// w -> r
	m_vAnimationTransitions[3].SetTransitionAnimationTrack(2, 1);	// r -> w

	m_vAnimationTransitions[0].SetAnimationTransition(0.01f, 0.0f, 0.25f);
	m_vAnimationTransitions[1].SetAnimationTransition(0.5f, 0.0f, 0.25f);
	m_vAnimationTransitions[2].SetAnimationTransition(0.1f, 0.0f, 0.25f);
	m_vAnimationTransitions[3].SetAnimationTransition(0.1f, 0.0f, 0.25f);

	m_bTransition = false;
	m_nNowState = PlayerState::IDLE;
	m_nNextState = PlayerState::IDLE;

	for (int i = 0; i < nAnimationTracks; ++i)
	{
		SetTrackAnimationSet(i, i);
		SetTrackEnable(i, false);
	}
	SetTrackEnable(0, true);
	AddBlendWeight(1.0f);

	for (int i = 0; i < m_pAnimationSets->m_nBoneFrames; ++i)
	{
		char* frameName = m_pAnimationSets->m_vpBoneFrameCaches[i]->GetFrameName();
		if (strncmp(frameName, "Scapula_L", strlen(frameName)) == 0) m_nStartLArm = i;
		if (strncmp(frameName, "ThumbFinger4_L", strlen(frameName)) == 0) m_nEndLArm = i;
		if (strncmp(frameName, "Scapula_R", strlen(frameName)) == 0) m_nStartRArm = i;
		if (strncmp(frameName, "ThumbFinger4_R", strlen(frameName)) == 0) m_nEndRArm = i;
		if (strncmp(frameName, "Spine1_M", strlen(frameName)) == 0) m_nStartSpine = i;
		if (strncmp(frameName, "Neck_M", strlen(frameName)) == 0) m_nStartNeck = i;
		if (strncmp(frameName, "JawEnd_M", strlen(frameName)) == 0) m_nEndNeck = i;
		if (strncmp(frameName, "Player_Flashlight", strlen(frameName)) == 0) m_nPlayerFlashLight = i;
		if (strncmp(frameName, "Elbow_L", strlen(frameName)) == 0) m_nElbow_L = i;
		if (strncmp(frameName, "Elbow_R", strlen(frameName)) == 0) m_nElbow_R = i;
		if (strncmp(frameName, "Head_M", strlen(frameName)) == 0) m_nHead_M = i;
		if (strncmp(frameName, "Item_Raider", strlen(frameName)) == 0) m_nRaderItem = i;
		if (strncmp(frameName, "Item_Teleport", strlen(frameName)) == 0) m_nTeleportItem = i;
	}

	m_xmf4x4RightHandRotate = Matrix4x4::Identity();

	CalculateRightHandMatrix();
}

void CBlueSuitAnimationController::AdvanceTime(float fElapsedTime, CGameObject* pRootGameObject)
{
	m_fTime += fElapsedTime;
	if (!m_vAnimationTracks.empty())
	{
		for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++) m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent = Matrix4x4::Zero();

		if (m_bTransition) // 트렌지션
		{
			int nTransitionIndex = -1;
			for (int i = 0; i < m_nTransition; ++i)
			{
				if (m_vAnimationTransitions[i].IsTransition(m_nNowState, m_nNextState))
				{
					nTransitionIndex = i;
					break;
				}
			}
			if (nTransitionIndex == -1)
			{
				//에러
			}
			else if (nTransitionIndex == 0)	// IDLE -> WALK
			{
				BlendAnimation(1, 2, fElapsedTime, m_vfBlendWeight[0]);
				shared_ptr<CAnimationSet> pAnimationSet = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[0].m_nAnimationSet];
				float fPosition = m_vAnimationTracks[0].UpdatePosition(m_vAnimationTracks[0].m_fPosition, fElapsedTime, pAnimationSet->m_fLength);

				for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
				{
					if (j == m_nStartSpine) continue;
					XMFLOAT4X4 xmf4x4WALKTransform = m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent;	// WALK
					XMFLOAT4X4 xmf4x4IDLETransform = pAnimationSet->GetSRT(j, fPosition);					// IDLE
					xmf4x4IDLETransform = Matrix4x4::Scale(xmf4x4IDLETransform, m_vAnimationTracks[0].m_fWeight);

					XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4IDLETransform, xmf4x4WALKTransform,
						m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime / m_vAnimationTransitions[nTransitionIndex].m_fTransitionDuration);

					m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4TrackTransform;
				}

				m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime += fElapsedTime;
				if (m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime - m_vAnimationTransitions[nTransitionIndex].m_fTransitionDuration >= -EPSILON)
				{
					m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime = 0.0f;
					m_nNowState = PlayerState::WALK;
					m_bTransition = false;
					SetTrackEnable(0, false);
					SetTrackPosition(0, 0.0f);
					SetTrackEnable(1, true);
					SetTrackEnable(2, true);
					SetTrackEnable(3, false);
					SetTrackPosition(3, 0.0f);
				}
			}
			else if (nTransitionIndex == 1)	// WALK -> IDLE
			{
				BlendAnimation(1, 2, fElapsedTime, m_vfBlendWeight[0]);
				shared_ptr<CAnimationSet>  pAnimationSet = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[0].m_nAnimationSet];
				float fPosition = m_vAnimationTracks[0].UpdatePosition(m_vAnimationTracks[0].m_fPosition, fElapsedTime, pAnimationSet->m_fLength);

				for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
				{
					if (j == m_nStartSpine) continue;
					XMFLOAT4X4 xmf4x4WALKTransform = m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent;	// WALK
					XMFLOAT4X4 xmf4x4IDLETransform = pAnimationSet->GetSRT(j, fPosition);					// IDLE
					xmf4x4IDLETransform = Matrix4x4::Scale(xmf4x4IDLETransform, m_vAnimationTracks[0].m_fWeight);

					XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4WALKTransform, xmf4x4IDLETransform,
						m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime / m_vAnimationTransitions[nTransitionIndex].m_fTransitionDuration);

					m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4TrackTransform;
				}

				m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime += fElapsedTime;
				if (m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime - m_vAnimationTransitions[nTransitionIndex].m_fTransitionDuration >= -EPSILON)
				{
					m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime = 0.0f;
					m_nNowState = PlayerState::IDLE;
					m_bTransition = false;
					SetTrackEnable(0, true);
					SetTrackEnable(1, false);
					SetTrackPosition(1, 0.0f);
					SetTrackSpeed(1, 1.0f);
					SetTrackEnable(2, false);
					SetTrackPosition(2, 0.0f);
					SetTrackSpeed(2, 1.0f);
					SetTrackEnable(3, false);
					SetTrackPosition(3, 0.0f);
				}
			}
			else if (nTransitionIndex == 2) // WALK -> RUN
			{
				BlendAnimation(1, 2, fElapsedTime, m_vfBlendWeight[0]);
				shared_ptr<CAnimationSet> pRunAnimationSet = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[3].m_nAnimationSet];
				float fPosition = m_vAnimationTracks[3].UpdatePosition(m_vAnimationTracks[3].m_fPosition, fElapsedTime, pRunAnimationSet->m_fLength);

				for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
				{
					XMFLOAT4X4 xmf4x4WALKTransform = m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent;	// WALK
					XMFLOAT4X4 xmf4x4RUNTransform = pRunAnimationSet->GetSRT(j, fPosition);					// RUN
					xmf4x4RUNTransform = Matrix4x4::Scale(xmf4x4RUNTransform, m_vAnimationTracks[3].m_fWeight);

					XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4WALKTransform, xmf4x4RUNTransform,
						m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime / m_vAnimationTransitions[nTransitionIndex].m_fTransitionDuration);

					m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4TrackTransform;
				}

				m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime += fElapsedTime;
				if (m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime - m_vAnimationTransitions[nTransitionIndex].m_fTransitionDuration >= -EPSILON)
				{
					m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime = 0.0f;
					m_nNowState = PlayerState::RUN;
					m_bTransition = false;
					SetTrackEnable(0, false);
					SetTrackPosition(0, 0.0f);
					SetTrackEnable(1, false);
					SetTrackPosition(1, 0.0f);
					SetTrackSpeed(1, 1.0f);
					SetTrackEnable(2, false);
					SetTrackPosition(2, 0.0f);
					SetTrackSpeed(2, 1.0f);
					SetTrackEnable(3, true);
				}
			}
			else if (nTransitionIndex == 3)	// RUN -> WALK
			{
				BlendAnimation(1, 2, fElapsedTime, m_vfBlendWeight[0]);
				shared_ptr<CAnimationSet> pRunAnimationSet = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[3].m_nAnimationSet];
				float fPosition = m_vAnimationTracks[3].UpdatePosition(m_vAnimationTracks[3].m_fPosition, fElapsedTime, pRunAnimationSet->m_fLength);

				for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
				{
					XMFLOAT4X4 xmf4x4WALKTransform = m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent;	// WALK
					XMFLOAT4X4 xmf4x4RUNTransform = pRunAnimationSet->GetSRT(j, fPosition);					// RUN
					xmf4x4RUNTransform = Matrix4x4::Scale(xmf4x4RUNTransform, m_vAnimationTracks[3].m_fWeight);

					XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4RUNTransform, xmf4x4WALKTransform,
						m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime / m_vAnimationTransitions[nTransitionIndex].m_fTransitionDuration);

					m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4TrackTransform;
				}

				m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime += fElapsedTime;
				if (m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime - m_vAnimationTransitions[nTransitionIndex].m_fTransitionDuration >= -EPSILON)
				{
					m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime = 0.0f;
					m_nNowState = PlayerState::WALK;
					m_bTransition = false;
					SetTrackEnable(0, false);
					SetTrackPosition(0, 0.0f);
					SetTrackEnable(1, true);
					SetTrackEnable(2, true); ;
					SetTrackEnable(3, true);
					SetTrackPosition(3, 0.0f);
				}
			}
		}
		else if (m_vAnimationTracks[1].m_bEnable || m_vAnimationTracks[2].m_bEnable) //애니메이션 블렌딩
		{
			BlendAnimation(1, 2, fElapsedTime, m_vfBlendWeight[0]);
			if(m_vAnimationTracks[1].m_bEnable)
			{
				m_vAnimationTracks[1].HandleCallback();
			}
			if (m_vAnimationTracks[2].m_bEnable)
			{
				m_vAnimationTracks[2].HandleCallback();
			}
		}
		else
		{
			for (int k = 0; k < m_nAnimationTracks; k++)
			{
				if (m_vAnimationTracks[k].m_bEnable)
				{
					shared_ptr<CAnimationSet>  pAnimationSet = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[k].m_nAnimationSet];	//애니메이션 트랙에 해당하는 애니메이션 sets을 가져온다
					float fPosition = m_vAnimationTracks[k].UpdatePosition(m_vAnimationTracks[k].m_fPosition, fElapsedTime, pAnimationSet->m_fLength);	// 현재 애니메이션 트랙을 재생(현재 재생중인 위치와 흐른 시간, 애니메이션 총 길이)
					for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
					{
						XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent;
						XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j, fPosition);
						xmf4x4TrackTransform = Matrix4x4::Scale(xmf4x4TrackTransform, m_vAnimationTracks[k].m_fWeight);
						xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, xmf4x4TrackTransform);
						m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
					}
					m_vAnimationTracks[k].HandleCallback();
				}
			}
		}

		int nTorchAnimationTrack = 4;
		int nIdleAnimationTrack = 0;
		shared_ptr<CAnimationSet> pBlendAnimationSet_0 = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[nTorchAnimationTrack].m_nAnimationSet];
		float fPos_0 = m_vAnimationTracks[nTorchAnimationTrack].UpdatePosition(m_vAnimationTracks[nTorchAnimationTrack].m_fPosition, fElapsedTime, pBlendAnimationSet_0->m_fLength);

		shared_ptr<CAnimationSet> pBlendAnimationSet_1 = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[nIdleAnimationTrack].m_nAnimationSet];
		float fPos_1 = m_vAnimationTracks[nIdleAnimationTrack].UpdatePosition(m_vAnimationTracks[nIdleAnimationTrack].m_fPosition, fElapsedTime, pBlendAnimationSet_1->m_fLength);

		for (int j = m_nStartLArm; j <= m_nEndLArm; j++)
		{
			XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent;
			XMFLOAT4X4 xmf4x4TrackTransform_0 = pBlendAnimationSet_0->GetSRT(j, fPos_0);
			xmf4x4TrackTransform_0 = Matrix4x4::Scale(xmf4x4TrackTransform_0, m_vAnimationTracks[nTorchAnimationTrack].m_fWeight);

			XMFLOAT4X4 xmf4x4TrackTransform_1 = pBlendAnimationSet_1->GetSRT(j, fPos_1);
			xmf4x4TrackTransform_1 = Matrix4x4::Scale(xmf4x4TrackTransform_1, m_vAnimationTracks[nIdleAnimationTrack].m_fWeight);

			XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4TrackTransform_0, xmf4x4TrackTransform_1, 0.65f);
			xmf4x4Transform = xmf4x4TrackTransform;
			m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
		}

		//[CJI 0407] 왼쪽 팔꿈치를 중심으로 축을 회전
		XMFLOAT3 axis = { 0.f,0.f,1.f };
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&axis), XMConvertToRadians(m_fElbowPitch));
		XMStoreFloat4x4(&m_pAnimationSets->m_vpBoneFrameCaches[m_nElbow_L]->m_xmf4x4ToParent,
			XMMatrixMultiply(xmmtxRotate, XMLoadFloat4x4(&m_pAnimationSets->m_vpBoneFrameCaches[m_nElbow_L]->m_xmf4x4ToParent)));
		if (m_bSelectItem) {//[CJI 0428] 선택한 아이템이 있으면 아이템을 들고 있도록 회전
			XMStoreFloat4x4(&m_pAnimationSets->m_vpBoneFrameCaches[m_nElbow_R]->m_xmf4x4ToParent,
				XMMatrixMultiply(XMLoadFloat4x4(&m_xmf4x4RightHandRotate), XMLoadFloat4x4(&m_pAnimationSets->m_vpBoneFrameCaches[m_nElbow_R]->m_xmf4x4ToParent)));
		}

		pRootGameObject->UpdateTransform(NULL);

		OnRootMotion(pRootGameObject);
		OnAnimationIK(pRootGameObject);
	}
}

void CBlueSuitAnimationController::CalculateRightHandMatrix()
{
	XMFLOAT3 axis = { 0.f,0.f,1.f };
	XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&axis), XMConvertToRadians(-90.0f));
	axis = { 0.f,1.f,0.f };
	XMMATRIX xmmtxRotate2 = XMMatrixRotationAxis(XMLoadFloat3(&axis), XMConvertToRadians(18.0f));
	axis = { 1.f,0.f,0.f };
	XMMATRIX xmmtxRotate3 = XMMatrixRotationAxis(XMLoadFloat3(&axis), XMConvertToRadians(45.0f));

	XMStoreFloat4x4(&m_xmf4x4RightHandRotate, XMMatrixMultiply(XMMatrixMultiply(xmmtxRotate, xmmtxRotate2), xmmtxRotate3));
}

void CBlueSuitAnimationController::BlendAnimation(int nTrack1, int nTrack2, float fElapsedTime, float fBlentWeight)
{
	shared_ptr<CAnimationSet> pAnimationSet_0 = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[nTrack1].m_nAnimationSet];
	shared_ptr<CAnimationSet> pAnimationSet_1 = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[nTrack2].m_nAnimationSet];
	float fPosition_0 = m_vAnimationTracks[nTrack1].UpdatePosition(m_vAnimationTracks[nTrack1].m_fPosition, fElapsedTime, pAnimationSet_0->m_fLength);
	float fPosition_1 = m_vAnimationTracks[nTrack2].UpdatePosition(m_vAnimationTracks[nTrack2].m_fPosition, fElapsedTime, pAnimationSet_1->m_fLength);

	for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
	{
		XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent;
		XMFLOAT4X4 xmf4x4TrackTransform_0 = pAnimationSet_0->GetSRT(j, fPosition_0);
		xmf4x4TrackTransform_0 = Matrix4x4::Scale(xmf4x4TrackTransform_0, m_vAnimationTracks[1].m_fWeight);

		XMFLOAT4X4 xmf4x4TrackTransform_1 = pAnimationSet_1->GetSRT(j, fPosition_1);
		xmf4x4TrackTransform_1 = Matrix4x4::Scale(xmf4x4TrackTransform_1, m_vAnimationTracks[2].m_fWeight);

		XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4TrackTransform_0, xmf4x4TrackTransform_1, fBlentWeight);
		xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, xmf4x4TrackTransform);
		m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
	}
	m_vAnimationTracks[nTrack1].HandleCallback();
	m_vAnimationTracks[nTrack2].HandleCallback();
}

void CBlueSuitAnimationController::SetElbowPitch(float value)
{
	float fOld_pitch = m_fElbowPitch;
	m_fElbowPitch = value;

	if (m_fElbowPitch > 30.0f || m_fElbowPitch < -45.0f) {
		m_fElbowPitch = fOld_pitch;
	}
}

int CBlueSuitAnimationController::GetBoneFrameIndex(char* frameName)
{
	int i = -1;
	if (strncmp(frameName, "Scapula_L", strlen(frameName)) == 0) i = m_nStartLArm ;
	if (strncmp(frameName, "ThumbFinger4_L", strlen(frameName)) == 0) i = m_nEndLArm ;
	if (strncmp(frameName, "Scapula_R", strlen(frameName)) == 0) i = m_nStartRArm ;
	if (strncmp(frameName, "ThumbFinger4_R", strlen(frameName)) == 0) i = m_nEndRArm ;
	if (strncmp(frameName, "Spine1_M", strlen(frameName)) == 0) i = m_nStartSpine ;
	if (strncmp(frameName, "Neck_M", strlen(frameName)) == 0) i = m_nStartNeck ;
	if (strncmp(frameName, "JawEnd_M", strlen(frameName)) == 0) i = m_nEndNeck ;
	if (strncmp(frameName, "Player_Flashlight", strlen(frameName)) == 0) i = m_nPlayerFlashLight ;
	if (strncmp(frameName, "Elbow_L", strlen(frameName)) == 0) i = m_nElbow_L ;
	if (strncmp(frameName, "Elbow_R", strlen(frameName)) == 0) i = m_nElbow_R;
	if (strncmp(frameName, "Head_M", strlen(frameName)) == 0) i = m_nHead_M ;
	if (strncmp(frameName, "Item_Raider", strlen(frameName)) == 0) i = m_nRaderItem;
	if (strncmp(frameName, "Item_Teleport", strlen(frameName)) == 0) i = m_nTeleportItem;

	if (i == -1) {
		assert(0);
	}

	return i;
}

CZombieAnimationController::CZombieAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, const shared_ptr<CLoadedModelInfo>& pModel)
	: CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pModel)
{
	// IDLE, WALK, RUN 2가지 상태
	m_nState = 2;
	m_nTransition = 4;
	m_vAnimationTransitions.reserve(m_nTransition);
	for (int i = 0; i < m_nTransition; ++i)
	{
		m_vAnimationTransitions.emplace_back();
	}

	m_vAnimationTransitions[0].SetTransitionAnimationTrack(0, 1);	// i -> w
	m_vAnimationTransitions[1].SetTransitionAnimationTrack(1, 0);	// w -> i

	m_vAnimationTransitions[0].SetAnimationTransition(0.01f, 0.0f, 0.25f);
	m_vAnimationTransitions[1].SetAnimationTransition(0.5f, 0.0f, 0.25f);

	m_bTransition = false;
	m_nNowState = PlayerState::IDLE;
	m_nNextState = PlayerState::IDLE;

	for (int i = 0; i < nAnimationTracks; ++i)
	{
		SetTrackAnimationSet(i, i);
		SetTrackEnable(i, false);
	}
	SetTrackEnable(0, true);
	//AddBlendWeight(1.0f);

	SetTrackSpeed(1, 1.5f);
	SetTrackSpeed(2, 1.5f);

	for (int i = 0; i < m_pAnimationSets->m_nBoneFrames; ++i)
	{
		char* frameName = m_pAnimationSets->m_vpBoneFrameCaches[i]->GetFrameName();

		if (strncmp(frameName, "mixamorig:Spine", strlen(frameName)) == 0) m_nStartSpine = i;
		if (strncmp(frameName, "mixamorig:Neck", strlen(frameName)) == 0) m_nStartNeck = i;
		if (strncmp(frameName, "mixamorig:HeadTop_End", strlen(frameName)) == 0) m_nEndNeck = i;
		if (strncmp(frameName, "mixamorig:RightHandThumb4", strlen(frameName)) == 0) m_nEndSpine = i;
	}
}

void CZombieAnimationController::AdvanceTime(float fElapsedTime, CGameObject* pRootGameObject)
{
	m_fTime += fElapsedTime;
	if (!m_vAnimationTracks.empty())
	{
		for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++) m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent = Matrix4x4::Zero();

		if (m_bTransition)
		{
			int nTransitionIndex = -1;
			for (int i = 0; i < m_nTransition; ++i)
			{
				if (m_vAnimationTransitions[i].IsTransition(m_nNowState, m_nNextState))
				{
					nTransitionIndex = i;
					break;
				}
			}
			if (nTransitionIndex == -1)
			{
				//에러
			}
			else if (nTransitionIndex == 0)	// IDLE -> WALK
			{
				shared_ptr<CAnimationSet> pIDLEAnimationSet = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[0].m_nAnimationSet];
				float fIDLEPosition = m_vAnimationTracks[0].UpdatePosition(m_vAnimationTracks[0].m_fPosition, fElapsedTime, pIDLEAnimationSet->m_fLength);

				shared_ptr<CAnimationSet> pWALKAnimationSet = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[1].m_nAnimationSet];
				float fIWALKPosition = m_vAnimationTracks[1].UpdatePosition(m_vAnimationTracks[1].m_fPosition, fElapsedTime, pWALKAnimationSet->m_fLength);

				for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
				{
					XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent;
					XMFLOAT4X4 xmf4x4IDLETransform = pIDLEAnimationSet->GetSRT(j, fIDLEPosition);	// IDLE
					xmf4x4IDLETransform = Matrix4x4::Scale(xmf4x4IDLETransform, m_vAnimationTracks[0].m_fWeight);

					XMFLOAT4X4 xmf4x4WALKTransform = pWALKAnimationSet->GetSRT(j, fIWALKPosition);	// WALK
					xmf4x4WALKTransform = Matrix4x4::Scale(xmf4x4WALKTransform, m_vAnimationTracks[1].m_fWeight);

					XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4IDLETransform, xmf4x4WALKTransform,
						m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime / m_vAnimationTransitions[nTransitionIndex].m_fTransitionDuration);

					xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, xmf4x4TrackTransform);
					m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
				}

				m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime += fElapsedTime;
				if (m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime - m_vAnimationTransitions[nTransitionIndex].m_fTransitionDuration >= -EPSILON)
				{
					m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime = 0.0f;
					m_nNowState = PlayerState::WALK;
					m_bTransition = false;
					SetTrackEnable(0, false);
					SetTrackPosition(0, 0.0f);
					SetTrackEnable(1, true);
				}
			}
			else if (nTransitionIndex == 1)	// WALK -> IDLE
			{
				shared_ptr<CAnimationSet>  pIDLEAnimationSet = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[0].m_nAnimationSet];
				float fIDLEPosition = m_vAnimationTracks[0].UpdatePosition(m_vAnimationTracks[0].m_fPosition, fElapsedTime, pIDLEAnimationSet->m_fLength);

				shared_ptr<CAnimationSet>  pWALKAnimationSet = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[1].m_nAnimationSet];
				float fIWALKPosition = m_vAnimationTracks[1].UpdatePosition(m_vAnimationTracks[1].m_fPosition, fElapsedTime, pWALKAnimationSet->m_fLength);

				for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
				{
					XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent;
					XMFLOAT4X4 xmf4x4IDLETransform = pIDLEAnimationSet->GetSRT(j, fIDLEPosition);	// IDLE
					xmf4x4IDLETransform = Matrix4x4::Scale(xmf4x4IDLETransform, m_vAnimationTracks[0].m_fWeight);

					XMFLOAT4X4 xmf4x4WALKTransform = pWALKAnimationSet->GetSRT(j, fIWALKPosition);	// WALK
					xmf4x4WALKTransform = Matrix4x4::Scale(xmf4x4WALKTransform, m_vAnimationTracks[1].m_fWeight);

					XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4WALKTransform, xmf4x4IDLETransform,
						m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime / m_vAnimationTransitions[nTransitionIndex].m_fTransitionDuration);

					xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, xmf4x4TrackTransform);
					m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
				}

				m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime += fElapsedTime;
				if (m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime - m_vAnimationTransitions[nTransitionIndex].m_fTransitionDuration >= -EPSILON)
				{
					m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime = 0.0f;
					m_nNowState = PlayerState::IDLE;
					m_bTransition = false;
					SetTrackEnable(0, true);
					SetTrackEnable(1, false);
					SetTrackPosition(1, 0.0f);
				}
			}
		}
		else
		{
			for (int k = 0; k < m_nAnimationTracks - 1; k++)
			{
				if (m_vAnimationTracks[k].m_bEnable)
				{
					shared_ptr<CAnimationSet> pAnimationSet = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[k].m_nAnimationSet];	//애니메이션 트랙에 해당하는 애니메이션 sets을 가져온다
					float fPosition = m_vAnimationTracks[k].UpdatePosition(m_vAnimationTracks[k].m_fPosition, fElapsedTime, pAnimationSet->m_fLength);	// 현재 애니메이션 트랙을 재생(현재 재생중인 위치와 흐른 시간, 애니메이션 총 길이)
					for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
					{
						XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent;
						XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j, fPosition);
						xmf4x4TrackTransform = Matrix4x4::Scale(xmf4x4TrackTransform, m_vAnimationTracks[k].m_fWeight);
						xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, xmf4x4TrackTransform);
						m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
					}
					m_vAnimationTracks[k].HandleCallback();
				}
			}

			if (m_vAnimationTracks[2].m_bEnable)
			{
				int nAttackAnimationTrack = 2;
				shared_ptr<CAnimationSet> pAnimationSet = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[nAttackAnimationTrack].m_nAnimationSet];
				float fPosition = m_vAnimationTracks[nAttackAnimationTrack].UpdatePosition(m_vAnimationTracks[nAttackAnimationTrack].m_fPosition, fElapsedTime, pAnimationSet->m_fLength);

				for (int j = m_nStartSpine; j <= m_nEndSpine; j++)
				{
					if (j >= m_nStartNeck && j <= m_nEndNeck)
					{
						continue;
					}

					XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent;
					XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j, fPosition);
					xmf4x4Transform = Matrix4x4::Scale(xmf4x4TrackTransform, m_vAnimationTracks[nAttackAnimationTrack].m_fWeight);
					m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
				}

				if (fPosition >= pAnimationSet->m_fLength - ANIMATION_CALLBACK_EPSILON)
				{
					SetTrackEnable(2, false);
					SetTrackPosition(2, 0.0f);
				}
			}

		}

		pRootGameObject->UpdateTransform(NULL);

		OnRootMotion(pRootGameObject);
		OnAnimationIK(pRootGameObject);
	}
}

void CZombieAnimationController::BlendAnimation(int nTrack1, int nTrack2, float fElapsedTime, float fBlentWeight)
{
	shared_ptr<CAnimationSet> pAnimationSet_0 = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[nTrack1].m_nAnimationSet];
	shared_ptr<CAnimationSet> pAnimationSet_1 = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[nTrack2].m_nAnimationSet];
	float fPosition_0 = m_vAnimationTracks[nTrack1].UpdatePosition(m_vAnimationTracks[nTrack1].m_fPosition, fElapsedTime, pAnimationSet_0->m_fLength);
	float fPosition_1 = m_vAnimationTracks[nTrack2].UpdatePosition(m_vAnimationTracks[nTrack2].m_fPosition, fElapsedTime, pAnimationSet_1->m_fLength);

	for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
	{
		XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent;
		XMFLOAT4X4 xmf4x4TrackTransform_0 = pAnimationSet_0->GetSRT(j, fPosition_0);
		xmf4x4TrackTransform_0 = Matrix4x4::Scale(xmf4x4TrackTransform_0, m_vAnimationTracks[1].m_fWeight);

		XMFLOAT4X4 xmf4x4TrackTransform_1 = pAnimationSet_1->GetSRT(j, fPosition_1);
		xmf4x4TrackTransform_1 = Matrix4x4::Scale(xmf4x4TrackTransform_1, m_vAnimationTracks[2].m_fWeight);

		XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4TrackTransform_0, xmf4x4TrackTransform_1, fBlentWeight);
		xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, xmf4x4TrackTransform);
		m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
	}
	m_vAnimationTracks[nTrack1].HandleCallback();
	m_vAnimationTracks[nTrack2].HandleCallback();
}
