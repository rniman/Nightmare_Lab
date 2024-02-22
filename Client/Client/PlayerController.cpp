#include "stdafx.h"
#include "PlayerController.h"

CBlueSuitAnimationController::CBlueSuitAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, CLoadedModelInfo* pModel)
	:CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pModel)
{
	// IDLE, WALK, RUN 3가지 상태
	m_nState = 3;
	m_nTransition = 4;
	m_pAnimationTransitions = new CAnimationTransition[m_nTransition];
	m_pAnimationTransitions[0].SetTransitionAnimationTrack(0, 1);	// i -> w
	m_pAnimationTransitions[1].SetTransitionAnimationTrack(1, 0);	// w -> i
	m_pAnimationTransitions[2].SetTransitionAnimationTrack(1, 2);	// w -> r
	m_pAnimationTransitions[3].SetTransitionAnimationTrack(2, 1);	// r -> w

	m_pAnimationTransitions[0].SetAnimationTransition(0.01, 0.0f, 0.25f);
	m_pAnimationTransitions[1].SetAnimationTransition(0.5, 0.0f, 0.25f);
	m_pAnimationTransitions[2].SetAnimationTransition(0.1, 0.0f, 0.25f);
	m_pAnimationTransitions[3].SetAnimationTransition(0.1, 0.0f, 0.25f);

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
		char* frameName = m_pAnimationSets->m_ppBoneFrameCaches[i]->GetFrameName();
		if (strncmp(frameName, "Scapula_L", strlen(frameName)) == 0) m_nStartLArm = i;
		if (strncmp(frameName, "ThumbFinger4_L", strlen(frameName)) == 0) m_nEndLArm = i;
		if (strncmp(frameName, "Scapula_R", strlen(frameName)) == 0) m_nStartRArm = i;
		if (strncmp(frameName, "ThumbFinger4_R", strlen(frameName)) == 0) m_nEndRArm = i;
		if (strncmp(frameName, "Spine1_M", strlen(frameName)) == 0) m_nStartSpine = i;
		if (strncmp(frameName, "Neck_M", strlen(frameName)) == 0) m_nStartNeck = i;
		if (strncmp(frameName, "JawEnd_M", strlen(frameName)) == 0) m_nEndNeck = i;
	}
}

void CBlueSuitAnimationController::AdvanceTime(float fElapsedTime, CGameObject* pRootGameObject)
{
	m_fTime += fElapsedTime;
	if (m_pAnimationTracks)
	{
		for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++) m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = Matrix4x4::Zero();

		if (m_bTransition) // 트렌지션
		{
			int nTransitionIndex = -1;
			for (int i = 0; i < m_nTransition; ++i)
			{
				if (m_pAnimationTransitions[i].IsTransition(m_nNowState, m_nNextState))
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
				CAnimationSet* pAnimationSet = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[0].m_nAnimationSet];
				float fPosition = m_pAnimationTracks[0].UpdatePosition(m_pAnimationTracks[0].m_fPosition, fElapsedTime, pAnimationSet->m_fLength);

				for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
				{
					XMFLOAT4X4 xmf4x4WALKTransform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent;	// WALK
					XMFLOAT4X4 xmf4x4IDLETransform = pAnimationSet->GetSRT(j, fPosition);					// IDLE
					xmf4x4IDLETransform = Matrix4x4::Scale(xmf4x4IDLETransform, m_pAnimationTracks[0].m_fWeight);

					XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4IDLETransform, xmf4x4WALKTransform,
						m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime / m_pAnimationTransitions[nTransitionIndex].m_fTransitionDuration);

					m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4TrackTransform;
				}

				m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime += fElapsedTime;
				if (m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime - m_pAnimationTransitions[nTransitionIndex].m_fTransitionDuration >= -EPSILON)
				{
					m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime = 0.0f;
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
				CAnimationSet* pAnimationSet = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[0].m_nAnimationSet];
				float fPosition = m_pAnimationTracks[0].UpdatePosition(m_pAnimationTracks[0].m_fPosition, fElapsedTime, pAnimationSet->m_fLength);

				for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
				{
					XMFLOAT4X4 xmf4x4WALKTransform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent;	// WALK
					XMFLOAT4X4 xmf4x4IDLETransform = pAnimationSet->GetSRT(j, fPosition);					// IDLE
					xmf4x4IDLETransform = Matrix4x4::Scale(xmf4x4IDLETransform, m_pAnimationTracks[0].m_fWeight);

					XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4WALKTransform, xmf4x4IDLETransform,
						m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime / m_pAnimationTransitions[nTransitionIndex].m_fTransitionDuration);

					m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4TrackTransform;
				}

				m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime += fElapsedTime;
				if (m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime - m_pAnimationTransitions[nTransitionIndex].m_fTransitionDuration >= -EPSILON)
				{
					m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime = 0.0f;
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
				CAnimationSet* pRunAnimationSet = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[3].m_nAnimationSet];
				float fPosition = m_pAnimationTracks[3].UpdatePosition(m_pAnimationTracks[3].m_fPosition, fElapsedTime, pRunAnimationSet->m_fLength);

				for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
				{
					XMFLOAT4X4 xmf4x4WALKTransform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent;	// WALK
					XMFLOAT4X4 xmf4x4RUNTransform = pRunAnimationSet->GetSRT(j, fPosition);					// RUN
					xmf4x4RUNTransform = Matrix4x4::Scale(xmf4x4RUNTransform, m_pAnimationTracks[3].m_fWeight);

					XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4WALKTransform, xmf4x4RUNTransform,
						m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime / m_pAnimationTransitions[nTransitionIndex].m_fTransitionDuration);

					m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4TrackTransform;
				}

				m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime += fElapsedTime;
				if (m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime - m_pAnimationTransitions[nTransitionIndex].m_fTransitionDuration >= -EPSILON)
				{
					m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime = 0.0f;
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
				CAnimationSet* pRunAnimationSet = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[3].m_nAnimationSet];
				float fPosition = m_pAnimationTracks[3].UpdatePosition(m_pAnimationTracks[3].m_fPosition, fElapsedTime, pRunAnimationSet->m_fLength);

				for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
				{
					XMFLOAT4X4 xmf4x4WALKTransform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent;	// WALK
					XMFLOAT4X4 xmf4x4RUNTransform = pRunAnimationSet->GetSRT(j, fPosition);					// RUN
					xmf4x4RUNTransform = Matrix4x4::Scale(xmf4x4RUNTransform, m_pAnimationTracks[3].m_fWeight);

					XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4RUNTransform, xmf4x4WALKTransform,
						m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime / m_pAnimationTransitions[nTransitionIndex].m_fTransitionDuration);

					m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4TrackTransform;
				}

				m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime += fElapsedTime;
				if (m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime - m_pAnimationTransitions[nTransitionIndex].m_fTransitionDuration >= -EPSILON)
				{
					m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime = 0.0f;
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
		else if (m_pAnimationTracks[1].m_bEnable || m_pAnimationTracks[2].m_bEnable) //애니메이션 블렌딩
		{
			BlendAnimation(1, 2, fElapsedTime, m_vfBlendWeight[0]);
			if(m_pAnimationTracks[1].m_bEnable)
			{
				m_pAnimationTracks[1].HandleCallback();
			}
			if (m_pAnimationTracks[2].m_bEnable)
			{
				m_pAnimationTracks[2].HandleCallback();
			}
		}
		else
		{
			for (int k = 0; k < m_nAnimationTracks; k++)
			{
				if (m_pAnimationTracks[k].m_bEnable)
				{
					CAnimationSet* pAnimationSet = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[k].m_nAnimationSet];	//애니메이션 트랙에 해당하는 애니메이션 sets을 가져온다
					float fPosition = m_pAnimationTracks[k].UpdatePosition(m_pAnimationTracks[k].m_fPosition, fElapsedTime, pAnimationSet->m_fLength);	// 현재 애니메이션 트랙을 재생(현재 재생중인 위치와 흐른 시간, 애니메이션 총 길이)
					for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
					{
						XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent;
						XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j, fPosition);
						xmf4x4TrackTransform = Matrix4x4::Scale(xmf4x4TrackTransform, m_pAnimationTracks[k].m_fWeight);
						xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, xmf4x4TrackTransform);
						m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
					}
					m_pAnimationTracks[k].HandleCallback();
				}
			}
		}

		int nTorchAnimationTrack = 4;
		int nIdleAnimationTrack = 0;
		CAnimationSet* pBlendAnimationSet_0 = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[nTorchAnimationTrack].m_nAnimationSet];
		float fPos_0 = m_pAnimationTracks[nTorchAnimationTrack].UpdatePosition(m_pAnimationTracks[nTorchAnimationTrack].m_fPosition, fElapsedTime, pBlendAnimationSet_0->m_fLength);

		CAnimationSet* pBlendAnimationSet_1 = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[nIdleAnimationTrack].m_nAnimationSet];
		float fPos_1 = m_pAnimationTracks[nIdleAnimationTrack].UpdatePosition(m_pAnimationTracks[nIdleAnimationTrack].m_fPosition, fElapsedTime, pBlendAnimationSet_1->m_fLength);

		for (int j = m_nStartLArm; j <= m_nEndLArm; j++)
		{
			XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent;
			XMFLOAT4X4 xmf4x4TrackTransform_0 = pBlendAnimationSet_0->GetSRT(j, fPos_0);
			xmf4x4TrackTransform_0 = Matrix4x4::Scale(xmf4x4TrackTransform_0, m_pAnimationTracks[nTorchAnimationTrack].m_fWeight);

			XMFLOAT4X4 xmf4x4TrackTransform_1 = pBlendAnimationSet_1->GetSRT(j, fPos_1);
			xmf4x4TrackTransform_1 = Matrix4x4::Scale(xmf4x4TrackTransform_1, m_pAnimationTracks[nIdleAnimationTrack].m_fWeight);

			XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4TrackTransform_0, xmf4x4TrackTransform_1, 0.65f);
			xmf4x4Transform = xmf4x4TrackTransform;
			m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;

		}

		pRootGameObject->UpdateTransform(NULL);

		OnRootMotion(pRootGameObject);
		OnAnimationIK(pRootGameObject);
	}
}

void CBlueSuitAnimationController::BlendAnimation(int nTrack1, int nTrack2, float fElapsedTime, float fBlentWeight)
{
	CAnimationSet* pAnimationSet_0 = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[nTrack1].m_nAnimationSet];
	CAnimationSet* pAnimationSet_1 = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[nTrack2].m_nAnimationSet];
	float fPosition_0 = m_pAnimationTracks[nTrack1].UpdatePosition(m_pAnimationTracks[nTrack1].m_fPosition, fElapsedTime, pAnimationSet_0->m_fLength);
	float fPosition_1 = m_pAnimationTracks[nTrack2].UpdatePosition(m_pAnimationTracks[nTrack2].m_fPosition, fElapsedTime, pAnimationSet_1->m_fLength);

	for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
	{
		XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent;
		XMFLOAT4X4 xmf4x4TrackTransform_0 = pAnimationSet_0->GetSRT(j, fPosition_0);
		xmf4x4TrackTransform_0 = Matrix4x4::Scale(xmf4x4TrackTransform_0, m_pAnimationTracks[1].m_fWeight);

		XMFLOAT4X4 xmf4x4TrackTransform_1 = pAnimationSet_1->GetSRT(j, fPosition_1);
		xmf4x4TrackTransform_1 = Matrix4x4::Scale(xmf4x4TrackTransform_1, m_pAnimationTracks[2].m_fWeight);

		XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4TrackTransform_0, xmf4x4TrackTransform_1, fBlentWeight);
		xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, xmf4x4TrackTransform);
		m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
	}
	m_pAnimationTracks[nTrack1].HandleCallback();
	m_pAnimationTracks[nTrack2].HandleCallback();
}

CZombieAnimationController::CZombieAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, CLoadedModelInfo* pModel)
	: CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pModel)
{
	// IDLE, WALK, RUN 2가지 상태
	m_nState = 2;
	m_nTransition = 4;
	m_pAnimationTransitions = new CAnimationTransition[m_nTransition];
	m_pAnimationTransitions[0].SetTransitionAnimationTrack(0, 1);	// i -> w
	m_pAnimationTransitions[1].SetTransitionAnimationTrack(1, 0);	// w -> i

	m_pAnimationTransitions[0].SetAnimationTransition(0.01, 0.0f, 0.25f);
	m_pAnimationTransitions[1].SetAnimationTransition(0.5, 0.0f, 0.25f);

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
		char* frameName = m_pAnimationSets->m_ppBoneFrameCaches[i]->GetFrameName();

		if (strncmp(frameName, "mixamorig:Spine", strlen(frameName)) == 0) m_nStartSpine = i;
		if (strncmp(frameName, "mixamorig:Neck", strlen(frameName)) == 0) m_nStartNeck = i;
		if (strncmp(frameName, "mixamorig:HeadTop_End", strlen(frameName)) == 0) m_nEndNeck = i;
		if (strncmp(frameName, "mixamorig:RightHandThumb4", strlen(frameName)) == 0) m_nEndSpine = i;
	}
}

void CZombieAnimationController::AdvanceTime(float fElapsedTime, CGameObject* pRootGameObject)
{
	m_fTime += fElapsedTime;
	if (m_pAnimationTracks)
	{
		for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++) m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = Matrix4x4::Zero();

		if (m_bTransition)
		{
			int nTransitionIndex = -1;
			for (int i = 0; i < m_nTransition; ++i)
			{
				if (m_pAnimationTransitions[i].IsTransition(m_nNowState, m_nNextState))
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
				CAnimationSet* pIDLEAnimationSet = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[0].m_nAnimationSet];
				float fIDLEPosition = m_pAnimationTracks[0].UpdatePosition(m_pAnimationTracks[0].m_fPosition, fElapsedTime, pIDLEAnimationSet->m_fLength);

				CAnimationSet* pWALKAnimationSet = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[1].m_nAnimationSet];
				float fIWALKPosition = m_pAnimationTracks[1].UpdatePosition(m_pAnimationTracks[1].m_fPosition, fElapsedTime, pWALKAnimationSet->m_fLength);

				for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
				{
					XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent;
					XMFLOAT4X4 xmf4x4IDLETransform = pIDLEAnimationSet->GetSRT(j, fIDLEPosition);	// IDLE
					xmf4x4IDLETransform = Matrix4x4::Scale(xmf4x4IDLETransform, m_pAnimationTracks[0].m_fWeight);

					XMFLOAT4X4 xmf4x4WALKTransform = pWALKAnimationSet->GetSRT(j, fIWALKPosition);	// WALK
					xmf4x4WALKTransform = Matrix4x4::Scale(xmf4x4WALKTransform, m_pAnimationTracks[1].m_fWeight);

					XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4IDLETransform, xmf4x4WALKTransform,
						m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime / m_pAnimationTransitions[nTransitionIndex].m_fTransitionDuration);

					xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, xmf4x4TrackTransform);
					m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
				}

				m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime += fElapsedTime;
				if (m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime - m_pAnimationTransitions[nTransitionIndex].m_fTransitionDuration >= -EPSILON)
				{
					m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime = 0.0f;
					m_nNowState = PlayerState::WALK;
					m_bTransition = false;
					SetTrackEnable(0, false);
					SetTrackPosition(0, 0.0f);
					SetTrackEnable(1, true);
				}
			}
			else if (nTransitionIndex == 1)	// WALK -> IDLE
			{
				CAnimationSet* pIDLEAnimationSet = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[0].m_nAnimationSet];
				float fIDLEPosition = m_pAnimationTracks[0].UpdatePosition(m_pAnimationTracks[0].m_fPosition, fElapsedTime, pIDLEAnimationSet->m_fLength);

				CAnimationSet* pWALKAnimationSet = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[1].m_nAnimationSet];
				float fIWALKPosition = m_pAnimationTracks[1].UpdatePosition(m_pAnimationTracks[1].m_fPosition, fElapsedTime, pWALKAnimationSet->m_fLength);

				for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
				{
					XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent;
					XMFLOAT4X4 xmf4x4IDLETransform = pIDLEAnimationSet->GetSRT(j, fIDLEPosition);	// IDLE
					xmf4x4IDLETransform = Matrix4x4::Scale(xmf4x4IDLETransform, m_pAnimationTracks[0].m_fWeight);

					XMFLOAT4X4 xmf4x4WALKTransform = pWALKAnimationSet->GetSRT(j, fIWALKPosition);	// WALK
					xmf4x4WALKTransform = Matrix4x4::Scale(xmf4x4WALKTransform, m_pAnimationTracks[1].m_fWeight);

					XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4WALKTransform, xmf4x4IDLETransform,
						m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime / m_pAnimationTransitions[nTransitionIndex].m_fTransitionDuration);

					xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, xmf4x4TrackTransform);
					m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
				}

				m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime += fElapsedTime;
				if (m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime - m_pAnimationTransitions[nTransitionIndex].m_fTransitionDuration >= -EPSILON)
				{
					m_pAnimationTransitions[nTransitionIndex].m_fTransitionTime = 0.0f;
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
				if (m_pAnimationTracks[k].m_bEnable)
				{
					CAnimationSet* pAnimationSet = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[k].m_nAnimationSet];	//애니메이션 트랙에 해당하는 애니메이션 sets을 가져온다
					float fPosition = m_pAnimationTracks[k].UpdatePosition(m_pAnimationTracks[k].m_fPosition, fElapsedTime, pAnimationSet->m_fLength);	// 현재 애니메이션 트랙을 재생(현재 재생중인 위치와 흐른 시간, 애니메이션 총 길이)
					for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
					{
						XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent;
						XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j, fPosition);
						xmf4x4TrackTransform = Matrix4x4::Scale(xmf4x4TrackTransform, m_pAnimationTracks[k].m_fWeight);
						xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, xmf4x4TrackTransform);
						m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
					}
					m_pAnimationTracks[k].HandleCallback();
				}
			}

			if (m_pAnimationTracks[2].m_bEnable)
			{
				int nAttackAnimationTrack = 2;
				CAnimationSet* pAnimationSet = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[nAttackAnimationTrack].m_nAnimationSet];
				float fPosition = m_pAnimationTracks[nAttackAnimationTrack].UpdatePosition(m_pAnimationTracks[nAttackAnimationTrack].m_fPosition, fElapsedTime, pAnimationSet->m_fLength);

				for (int j = m_nStartSpine; j <= m_nEndSpine; j++)
				{
					if (j >= m_nStartNeck && j <= m_nEndNeck)
					{
						continue;
					}

					XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent;
					XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j, fPosition);
					xmf4x4Transform = Matrix4x4::Scale(xmf4x4TrackTransform, m_pAnimationTracks[nAttackAnimationTrack].m_fWeight);
					m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
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
	CAnimationSet* pAnimationSet_0 = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[nTrack1].m_nAnimationSet];
	CAnimationSet* pAnimationSet_1 = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[nTrack2].m_nAnimationSet];
	float fPosition_0 = m_pAnimationTracks[nTrack1].UpdatePosition(m_pAnimationTracks[nTrack1].m_fPosition, fElapsedTime, pAnimationSet_0->m_fLength);
	float fPosition_1 = m_pAnimationTracks[nTrack2].UpdatePosition(m_pAnimationTracks[nTrack2].m_fPosition, fElapsedTime, pAnimationSet_1->m_fLength);

	for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
	{
		XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent;
		XMFLOAT4X4 xmf4x4TrackTransform_0 = pAnimationSet_0->GetSRT(j, fPosition_0);
		xmf4x4TrackTransform_0 = Matrix4x4::Scale(xmf4x4TrackTransform_0, m_pAnimationTracks[1].m_fWeight);

		XMFLOAT4X4 xmf4x4TrackTransform_1 = pAnimationSet_1->GetSRT(j, fPosition_1);
		xmf4x4TrackTransform_1 = Matrix4x4::Scale(xmf4x4TrackTransform_1, m_pAnimationTracks[2].m_fWeight);

		XMFLOAT4X4 xmf4x4TrackTransform = Matrix4x4::Interpolate(xmf4x4TrackTransform_0, xmf4x4TrackTransform_1, fBlentWeight);
		xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, xmf4x4TrackTransform);
		m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
	}
	m_pAnimationTracks[nTrack1].HandleCallback();
	m_pAnimationTracks[nTrack2].HandleCallback();
}
