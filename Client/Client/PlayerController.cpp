#include "stdafx.h"
#include "Scene.h"
#include "Player.h"
#include "PlayerController.h"

float CMainScene::testAngle;

CBlueSuitAnimationController::CBlueSuitAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, const shared_ptr<CLoadedModelInfo>& pModel)
	:CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pModel)
{
	// IDLE, WALK, RUN 3가지 상태
	m_nState = 4;
	m_nTransition = 4 + 3;

	m_vAnimationTransitions.reserve(m_nTransition);
	for (int i = 0; i < m_nTransition; ++i)
	{
		m_vAnimationTransitions.emplace_back();
	}

	m_vAnimationTransitions[0].SetTransitionAnimationTrack(0, 1);	// i -> w
	m_vAnimationTransitions[1].SetTransitionAnimationTrack(1, 0);	// w -> i
	m_vAnimationTransitions[2].SetTransitionAnimationTrack(1, 2);	// w -> r
	m_vAnimationTransitions[3].SetTransitionAnimationTrack(2, 1);	// r -> w

	m_vAnimationTransitions[4].SetTransitionAnimationTrack(0, 3);	// i -> d
	m_vAnimationTransitions[5].SetTransitionAnimationTrack(1, 3);	// w -> d
	m_vAnimationTransitions[6].SetTransitionAnimationTrack(2, 3);	// r -> d

	m_vAnimationTransitions[0].SetAnimationTransition(0.01f, 0.0f, 0.25f);
	m_vAnimationTransitions[1].SetAnimationTransition(0.5f, 0.0f, 0.25f);
	m_vAnimationTransitions[2].SetAnimationTransition(0.1f, 0.0f, 0.25f);
	m_vAnimationTransitions[3].SetAnimationTransition(0.1f, 0.0f, 0.25f);
	
	m_vAnimationTransitions[4].SetAnimationTransition(0.05f, 0.0f, 0.15f);
	m_vAnimationTransitions[5].SetAnimationTransition(0.05f, 0.0f, 0.15f);
	m_vAnimationTransitions[6].SetAnimationTransition(0.05f, 0.0f, 0.15f);

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

	m_vAnimationTracks[5].m_nType = ANIMATION_TYPE_ONCE;

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
		if (strncmp(frameName, "Wrist_L", strlen(frameName)) == 0) m_nWrist_L = i;
		if (strncmp(frameName, "Shoulder_L", strlen(frameName)) == 0) m_nShoulder_L = i;
		if (strncmp(frameName, "Chest_M", strlen(frameName)) == 0) m_nChest_M = i;
		
		
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
			TransitionBlueSuitPlayer(fElapsedTime);
		}
		else if (m_vAnimationTracks[1].m_bEnable || m_vAnimationTracks[2].m_bEnable) //애니메이션 블렌딩
		{
			BlendAnimation(1, 2, fElapsedTime*1.5f, m_vfBlendWeight[0]);
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
					float fPosition = m_vAnimationTracks[k].UpdatePosition(m_vAnimationTracks[k].m_fPosition, fElapsedTime*1.5f, pAnimationSet->m_fLength);	// 현재 애니메이션 트랙을 재생(현재 재생중인 위치와 흐른 시간, 애니메이션 총 길이)
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

		if (m_nNowState == PlayerState::DEATH)	// Death이므로 아래의 일을 수행할 필요가 없음
		{
			if (m_vAnimationTracks[5].m_fPosition >= m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[5].m_nAnimationSet]->m_fLength)
			{
				m_bAnimation = false;
			}
			pRootGameObject->UpdateTransform(NULL);
			return;
		}
		XMFLOAT3 axis = { 0.f,0.f,1.f };
		static float time = 0.0f;
		time += 0.1f;
		XMMATRIX L_ElbowRotate = XMMatrixRotationAxis(XMLoadFloat3(&axis), XMConvertToRadians(-88.0f));
		XMStoreFloat4x4(&m_pAnimationSets->m_vpBoneFrameCaches[m_nElbow_L]->m_xmf4x4ToParent,
			XMMatrixMultiply(L_ElbowRotate, XMLoadFloat4x4(&m_pAnimationSets->m_vpBoneFrameCaches[m_nElbow_L]->m_xmf4x4ToParent)));
		XMMATRIX L_ArmRotate = XMMatrixRotationAxis(XMLoadFloat3(&axis), XMConvertToRadians(-340.0f));
		XMStoreFloat4x4(&m_pAnimationSets->m_vpBoneFrameCaches[m_nStartLArm]->m_xmf4x4ToParent,
			XMMatrixMultiply(L_ArmRotate, XMLoadFloat4x4(&m_pAnimationSets->m_vpBoneFrameCaches[m_nStartLArm]->m_xmf4x4ToParent)));

		axis = { 0.f,0.f,1.f };
		XMMATRIX L_ElbowRotate2 = XMMatrixRotationAxis(XMLoadFloat3(&axis), XMConvertToRadians(m_fLElbowPitch));
		XMStoreFloat4x4(&m_pAnimationSets->m_vpBoneFrameCaches[m_nElbow_L]->m_xmf4x4ToParent,
			XMMatrixMultiply(L_ElbowRotate2, XMLoadFloat4x4(&m_pAnimationSets->m_vpBoneFrameCaches[m_nElbow_L]->m_xmf4x4ToParent)));

		axis = { 0.f,0.f,1.f };
		XMMATRIX L_ShoulderRotate = XMMatrixRotationAxis(XMLoadFloat3(&axis), XMConvertToRadians(-m_fLShoulderPitch));
		XMStoreFloat4x4(&m_pAnimationSets->m_vpBoneFrameCaches[m_nChest_M]->m_xmf4x4ToParent,
			XMMatrixMultiply(L_ShoulderRotate, XMLoadFloat4x4(&m_pAnimationSets->m_vpBoneFrameCaches[m_nChest_M]->m_xmf4x4ToParent)));

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
	axis = { 0.f,0.f,1.f };
	XMMATRIX rElbowRotate = XMMatrixRotationAxis(XMLoadFloat3(&axis), XMConvertToRadians(346.0f));
	/*XMStoreFloat4x4(&m_pAnimationSets->m_vpBoneFrameCaches[m_nElbow_R]->m_xmf4x4ToParent,
		XMMatrixMultiply(rElbowRotate, XMLoadFloat4x4(&m_pAnimationSets->m_vpBoneFrameCaches[m_nElbow_R]->m_xmf4x4ToParent)));*/
	xmmtxRotate = XMMatrixMultiply(xmmtxRotate, xmmtxRotate2);
	xmmtxRotate = XMMatrixMultiply(xmmtxRotate, xmmtxRotate3);
	xmmtxRotate = XMMatrixMultiply(xmmtxRotate, rElbowRotate);
	
	XMStoreFloat4x4(&m_xmf4x4RightHandRotate, xmmtxRotate);


}


void CBlueSuitAnimationController::BlendAnimation(int nTrack1, int nTrack2, float fElapsedTime, float fBlendWeight)
{
	shared_ptr<CAnimationSet> pAnimationSet_0 = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[nTrack1].m_nAnimationSet];
	shared_ptr<CAnimationSet> pAnimationSet_1 = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[nTrack2].m_nAnimationSet];

	float fPosition_0; 
	float fPosition_1;
	if (fBlendWeight < 0.5f)	// TRACK[1]이 더 큼
	{
		fPosition_0 = m_vAnimationTracks[nTrack1].UpdatePosition(m_vAnimationTracks[nTrack1].m_fPosition, fElapsedTime, pAnimationSet_0->m_fLength);
		if(m_vAnimationTracks[nTrack1].m_fSpeed > 0.0f)
		{
			if (m_vAnimationTracks[nTrack2].m_fSpeed > 0.0f)
				fPosition_1 = m_vAnimationTracks[nTrack2].m_fPosition = fPosition_0;
			else
				fPosition_1 = m_vAnimationTracks[nTrack2].m_fPosition = pAnimationSet_0->m_fLength - fPosition_0;
		}
		else
		{
			if (m_vAnimationTracks[nTrack2].m_fSpeed > 0.0f)
				fPosition_1 = m_vAnimationTracks[nTrack2].m_fPosition = pAnimationSet_0->m_fLength - fPosition_0;
			else
				fPosition_1 = m_vAnimationTracks[nTrack2].m_fPosition = fPosition_0;
		}
	}
	else
	{
		fPosition_1 = m_vAnimationTracks[nTrack2].UpdatePosition(m_vAnimationTracks[nTrack2].m_fPosition, fElapsedTime, pAnimationSet_1->m_fLength);
		if (m_vAnimationTracks[nTrack2].m_fSpeed > 0.0f)
		{
			if (m_vAnimationTracks[nTrack1].m_fSpeed > 0.0f)
				fPosition_0 = m_vAnimationTracks[nTrack1].m_fPosition = fPosition_1;
			else
				fPosition_0 = m_vAnimationTracks[nTrack1].m_fPosition = pAnimationSet_1->m_fLength - fPosition_1;
		}
		else
		{
			if (m_vAnimationTracks[nTrack1].m_fSpeed > 0.0f)
				fPosition_0 = m_vAnimationTracks[nTrack1].m_fPosition = pAnimationSet_1->m_fLength - fPosition_1;
			else
				fPosition_0 = m_vAnimationTracks[nTrack1].m_fPosition = fPosition_1;
		}
	}

	for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
	{
		XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent;
		XMFLOAT4X4 xmf4x4TrackTransform_0 = pAnimationSet_0->GetSRT(j, fPosition_0);
		xmf4x4TrackTransform_0 = Matrix4x4::Scale(xmf4x4TrackTransform_0, m_vAnimationTracks[1].m_fWeight);

		XMFLOAT4X4 xmf4x4TrackTransform_1 = pAnimationSet_1->GetSRT(j, fPosition_1);
		xmf4x4TrackTransform_1 = Matrix4x4::Scale(xmf4x4TrackTransform_1, m_vAnimationTracks[2].m_fWeight);

		XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4TrackTransform_0, xmf4x4TrackTransform_1, fBlendWeight);
		xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, xmf4x4TrackTransform);
		m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
	}
	m_vAnimationTracks[nTrack1].HandleCallback();
	m_vAnimationTracks[nTrack2].HandleCallback();
}

void CBlueSuitAnimationController::TransitionBlueSuitPlayer(float fElapsedTime)
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
		//exit(-1);
	}
	else if (nTransitionIndex == 0)	// IDLE -> WALK
	{
		TransitionIDLEtoWALK(fElapsedTime, nTransitionIndex);
	}
	else if (nTransitionIndex == 1)	// WALK -> IDLE
	{
		TransitionWALKtoIDLE(fElapsedTime, nTransitionIndex);
	}
	else if (nTransitionIndex == 2) // WALK -> RUN
	{
		TransitionWALKtoRUN(fElapsedTime, nTransitionIndex);
	}
	else if (nTransitionIndex == 3)	// RUN -> WALK
	{
		TransitionRUNtoWALK(fElapsedTime, nTransitionIndex);
	}
	else if (nTransitionIndex == 4 || nTransitionIndex == 5 || nTransitionIndex == 6)	// IDLE, WALK, RUN -> DEATH
	{
		TransitionToDEATH(fElapsedTime, nTransitionIndex);
	}
}

void CBlueSuitAnimationController::TransitionIDLEtoWALK(float fElapsedTime, int nTransitionIndex)
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

void CBlueSuitAnimationController::TransitionWALKtoIDLE(float fElapsedTime, int nTransitionIndex)
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

void CBlueSuitAnimationController::TransitionWALKtoRUN(float fElapsedTime, int nTransitionIndex)
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

void CBlueSuitAnimationController::TransitionRUNtoWALK(float fElapsedTime, int nTransitionIndex)
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
		SetTrackEnable(2, true);
		SetTrackEnable(3, false);
		SetTrackPosition(3, 0.0f);
	}
}

void CBlueSuitAnimationController::TransitionToDEATH(float fElapsedTime, int nTransitionIndex)
{
	int nAnimationTrack;
	if (nTransitionIndex == 5)
	{
		BlendAnimation(1, 2, fElapsedTime, m_vfBlendWeight[0]);
	}
	else if (nTransitionIndex == 4 || nTransitionIndex == 6)
	{
		int nTrackIndex;
		if (nTransitionIndex == 4)
		{
			nTrackIndex = 0;
		}
		else
		{
			nTrackIndex = 3;
		}

		shared_ptr<CAnimationSet>  pAnimationSet = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[nTrackIndex].m_nAnimationSet];
		float fPosition = m_vAnimationTracks[nTrackIndex].UpdatePosition(m_vAnimationTracks[nTrackIndex].m_fPosition, fElapsedTime, pAnimationSet->m_fLength);
		for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
		{
			XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent;
			XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j, fPosition);
			xmf4x4TrackTransform = Matrix4x4::Scale(xmf4x4TrackTransform, m_vAnimationTracks[nTrackIndex].m_fWeight);
			xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, xmf4x4TrackTransform);
			m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
		}
		m_vAnimationTracks[nTrackIndex].HandleCallback();
	}

	shared_ptr<CAnimationSet> pDeathAnimationSet = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[5].m_nAnimationSet];
	float fDeathPosition = m_vAnimationTracks[5].UpdatePosition(m_vAnimationTracks[5].m_fPosition, fElapsedTime, pDeathAnimationSet->m_fLength);

	for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
	{
		XMFLOAT4X4 xmf4x4WALKTransform = m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent;	// WALK
		XMFLOAT4X4 xmf4x4DeathTransform = pDeathAnimationSet->GetSRT(j, fDeathPosition);				// DEATH
		xmf4x4DeathTransform = Matrix4x4::Scale(xmf4x4DeathTransform, m_vAnimationTracks[5].m_fWeight);

		XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4DeathTransform, xmf4x4WALKTransform,
			m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime / m_vAnimationTransitions[nTransitionIndex].m_fTransitionDuration);

		m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4TrackTransform;
	}

	m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime += fElapsedTime;
	if (m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime - m_vAnimationTransitions[nTransitionIndex].m_fTransitionDuration >= -EPSILON)
	{
		m_vAnimationTransitions[nTransitionIndex].m_fTransitionTime = 0.0f;
		m_nNowState = PlayerState::DEATH;
		m_bTransition = false;
		SetTrackEnable(0, false);
		SetTrackEnable(1, false);
		SetTrackEnable(2, false);
		SetTrackEnable(3, false);
		SetTrackPosition(0, 0.0f);
		SetTrackPosition(1, 0.0f);
		SetTrackPosition(2, 0.0f);
		SetTrackPosition(3, 0.0f);
		SetTrackEnable(5, true);
	}
}

void CBlueSuitAnimationController::BlendLeftArm(float fElapsedTime)
{
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

		XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4TrackTransform_0, xmf4x4TrackTransform_1, 0.0f);
		xmf4x4Transform = xmf4x4TrackTransform;
		m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
	}
}

void CBlueSuitAnimationController::SetElbowPitch(float value)
{
	float fOld_pitch = m_fLElbowPitch;
	m_fLElbowPitch = value;

	if (m_fLElbowPitch > 80.0f || m_fLElbowPitch < -5.0f) {
		m_fLElbowPitch = fOld_pitch;
		if (value < -5.0f) {
			m_fLShoulderPitch = value - fOld_pitch;
			if (m_fLShoulderPitch < -30.0f) {
				m_fLShoulderPitch = -30.0f;
			}
		}
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
	if (strncmp(frameName, "Wrist_L", strlen(frameName)) == 0) i = m_nWrist_L;
	if (strncmp(frameName, "Shoulder_L", strlen(frameName)) == 0) i = m_nShoulder_L;
	if (strncmp(frameName, "Chest_M", strlen(frameName)) == 0) i = m_nChest_M;

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
		if (strncmp(frameName, "EyesSock", strlen(frameName)) == 0) m_nEyesSock = i;
		if (strncmp(frameName, "mixamorig:Hips", strlen(frameName)) == 0) m_nHips = i;

		
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
			TransitionZombiePlayer(fElapsedTime);
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
		pRootGameObject->UpdateTransform(NULL);

		if (m_pPlayer) {
			XMFLOAT3 offset = GetBoneFramePositionVector(m_nEyesSock);
			XMFLOAT3 pl_pos = m_pPlayer->GetPosition();
			offset.x = offset.x - pl_pos.x;
			offset.y = offset.y - pl_pos.y;
			offset.z = offset.z - pl_pos.z;
			if (m_pPlayer->GetCamera()->GetMode() != THIRD_PERSON_CAMERA) {
				m_pPlayer->GetCamera()->SetOffset(offset);
			}
		}

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

void CZombieAnimationController::TransitionZombiePlayer(float fElapsedTime)
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
		TransitionIDLEtoWALK(fElapsedTime, nTransitionIndex);
	}
	else if (nTransitionIndex == 1)	// WALK -> IDLE
	{
		TransitionWALKtoIDLE(fElapsedTime, nTransitionIndex);
	}
}

void CZombieAnimationController::TransitionIDLEtoWALK(float fElapsedTime, int nTransitionIndex)
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

void CZombieAnimationController::TransitionWALKtoIDLE(float fElapsedTime, int nTransitionIndex)
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

int CZombieAnimationController::GetBoneFrameIndex(char* frameName)
{
	int i = -1;
	if (strncmp(frameName, "mixamorig:Spine", strlen(frameName)) == 0) i = m_nStartSpine;
	if (strncmp(frameName, "mixamorig:Neck", strlen(frameName)) == 0) i = m_nStartNeck;
	if (strncmp(frameName, "mixamorig:HeadTop_End", strlen(frameName)) == 0) i = m_nEndNeck;
	if (strncmp(frameName, "mixamorig:RightHandThumb4", strlen(frameName)) == 0) i = m_nEndSpine;
	if (strncmp(frameName, "EyesSock", strlen(frameName)) == 0) i = m_nEyesSock;
	if (strncmp(frameName, "mixamorig:Hips", strlen(frameName)) == 0) i = m_nHips;

	if (i == -1) {
		assert(0);
	}

	return i;
}