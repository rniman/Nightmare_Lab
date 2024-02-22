#include "stdafx.h"
#include "Player.h"
#include "Scene.h"
#include "Shader.h"
#include "PlayerController.h"
#include "Collision.h"

//#define _WITH_DEBUG_CALLBACK_DATA

void CSoundCallbackHandler::HandleCallback(void* pCallbackData, float fTrackPosition)
{
	_TCHAR* pWavName = (_TCHAR*)pCallbackData;
#ifdef _WITH_DEBUG_CALLBACK_DATA
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("%s(%f)\n"), pWavName, fTrackPosition);
	OutputDebugString(pstrDebug);
#endif
#ifdef _WITH_SOUND_RESOURCE
	PlaySound(pWavName, ::ghAppInstance, SND_RESOURCE | SND_ASYNC);
#else
	PlaySound(pWavName, NULL, SND_FILENAME | SND_ASYNC);
#endif
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer

CPlayer::CPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CScene* pScene, void* pContext)
	: CGameObject(pd3dDevice, pd3dCommandList)
{
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);

		Move(xmf3Shift, bUpdateVelocity);
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::Rotate(float x, float y, float z)
{
	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if ((nCurrentCameraMode == FIRST_PERSON_CAMERA) || (nCurrentCameraMode == THIRD_PERSON_CAMERA))
	{
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		m_pCamera->Rotate(x, y, z);
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CPlayer::Update(float fElapsedTime)
{
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Gravity);
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	float fMaxVelocityY = m_fMaxVelocityY;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fElapsedTime, false);
	Move(xmf3Velocity, false);

	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fElapsedTime);

	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update(m_xmf3Position, fElapsedTime);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fElapsedTime);
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);
	m_pCamera->RegenerateViewMatrix();

	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fElapsedTime);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));

}

CCamera* CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	CCamera* pNewCamera = NULL;
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		pNewCamera = new CFirstPersonCamera(m_pCamera);
		break;
	case THIRD_PERSON_CAMERA:
		pNewCamera = new CThirdPersonCamera(m_pCamera);
		break;
	}

	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		pNewCamera->SetPlayer(this);
	}

	if (m_pCamera) delete m_pCamera;

	return(pNewCamera);
}

CCamera* CPlayer::ChangeCamera(DWORD nNewCameraMode, float fElapsedTime)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(30.0f);
		SetMaxVelocityY(40.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 2.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 500.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(30.0f);
		SetMaxVelocityY(40.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.25f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 2.0f, -5.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 500.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	Update(fElapsedTime);

	return(m_pCamera);
}

void CPlayer::OnPrepareRender()
{
	m_xmf4x4ToParent._11 = m_xmf3Right.x; m_xmf4x4ToParent._12 = m_xmf3Right.y; m_xmf4x4ToParent._13 = m_xmf3Right.z;
	m_xmf4x4ToParent._21 = m_xmf3Up.x; m_xmf4x4ToParent._22 = m_xmf3Up.y; m_xmf4x4ToParent._23 = m_xmf3Up.z;
	m_xmf4x4ToParent._31 = m_xmf3Look.x; m_xmf4x4ToParent._32 = m_xmf3Look.y; m_xmf4x4ToParent._33 = m_xmf3Look.z;
	m_xmf4x4ToParent._41 = m_xmf3Position.x; m_xmf4x4ToParent._42 = m_xmf3Position.y; m_xmf4x4ToParent._43 = m_xmf3Position.z;
	
	XMMATRIX xmtxScale = XMMatrixScaling(m_xmf3Scale.x, m_xmf3Scale.y, m_xmf3Scale.z);
	m_xmf4x4ToParent = Matrix4x4::Multiply(xmtxScale, m_xmf4x4ToParent);
}

void CPlayer::Animate(float fElapsedTime)
{
	OnPrepareRender();

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->AdvanceTime(fElapsedTime, this);

	AnimateOOBB();

	if (m_pSibling) m_pSibling->Animate(fElapsedTime);
	if (m_pChild) m_pChild->Animate(fElapsedTime);
}

void CPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	CGameObject::Render(pd3dCommandList);
}

//void CPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
//{
//	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
//	if (nCameraMode == THIRD_PERSON_CAMERA) CGameObject::Render(pd3dCommandList, pCamera);
//}

CGameObject* CPlayer::GetPickedObject(int nx, int ny, CScene* pScene)
{
	CGameObject* pPickedObject = nullptr;
	XMFLOAT3 pickPosition;

	if(m_pCamera->GetMode() == THIRD_PERSON_CAMERA)
	{
		pickPosition.x = ((2.0f * nx) / (float)m_pCamera->GetViewport().Width - 1) / m_pCamera->GetProjectionMatrix()._11;
		pickPosition.y = -(((2.0f * ny) / (float)m_pCamera->GetViewport().Height - 1) / m_pCamera->GetProjectionMatrix()._22);
	
	}
	else
	{
		pickPosition.x = 0.0f;
		pickPosition.y = 0.0f;
	}
	pickPosition.z = 1.0f;

	float fNearestHitDistance = FLT_MAX;
	for (auto& pvCollisionObject : g_collisonManager.m_vvpCollisionGameObjects)
	{
		for (auto& pCollisionObject : pvCollisionObject)
		{
			float fHitDistance = FLT_MAX;
			if (CGameObject::CheckPicking(pCollisionObject, pickPosition, m_pCamera->GetViewMatrix(), fHitDistance))
			{
				if (fHitDistance < fNearestHitDistance)
				{
					fNearestHitDistance = fHitDistance;
					pPickedObject = pCollisionObject;
				}
			}
		}
	}

	//for (auto& pShader : pScene->m_vShader)
	//{
	//	for (auto& pGameObject : pShader->GetGameObjects())
	//	{
	//		//if (!gameObject->GetPickingDetection())
	//		//	continue;

	//		for (auto& pCollisionObject : pGameObject->m_vpCollideFrame)
	//		{
	//			float fHitDistance = FLT_MAX;
	//			if (CGameObject::CheckPicking(pCollisionObject, pickPosition, m_pCamera->GetViewMatrix(), fHitDistance))
	//			{
	//				if (fHitDistance < fNearestHitDistance)
	//				{
	//					fNearestHitDistance = fHitDistance;
	//					pPickedObject = pGameObject.get();
	//				}
	//			}
	//		}
	//	}
	//}

	if (pPickedObject) 
	{
		return pPickedObject;
	}

	return nullptr;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 

CBlueSuitPlayer::CBlueSuitPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CScene* pScene, void* pContext)
	:CPlayer(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pScene, pContext)
{
	m_xmf3Scale = XMFLOAT3(0.1f, 0.1f, 0.1f);
	CLoadedModelInfo* pBlueSuitPlayer = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Asset/Model/BlueSuitFree01.bin");
	SetChild(pBlueSuitPlayer->m_pModelRootObject, true);

	int nCbv = 0;
	nCbv = cntCbvModelObject(this, 0);

	m_pSkinnedAnimationController = new CBlueSuitAnimationController(pd3dDevice, pd3dCommandList, 5, pBlueSuitPlayer);

//	m_pSkinnedAnimationController->SetCallbackKeys(1, 2);
//#ifdef _WITH_SOUND_RESOURCE
//	m_pSkinnedAnimationController->SetCallbackKey(0, 0.1f, _T("Footstep01"));
//	m_pSkinnedAnimationController->SetCallbackKey(1, 0.5f, _T("Footstep02"));
//	m_pSkinnedAnimationController->SetCallbackKey(2, 0.9f, _T("Footstep03"));
//#else
//	m_pSkinnedAnimationController->SetCallbackKey(1, 0, 0.1f, _T("Asset/Sound/Footstep01.wav"));
//	m_pSkinnedAnimationController->SetCallbackKey(1, 1, 0.9f, _T("Asset/Sound/Footstep02.wav"));
//#endif
//	CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
//	m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);

	SetPlayerUpdatedContext(pContext);
	SetCameraUpdatedContext(pContext);

	pScene->m_vShader[SKINNEDANIMATION_STANDARD_SHADER]->AddGameObject(this);

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);
	if (pBlueSuitPlayer) delete pBlueSuitPlayer;
}

CBlueSuitPlayer::~CBlueSuitPlayer()
{
}

void CBlueSuitPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);

	if ((dwDirection & LSHIFT) && m_bAbleRun) 
	{
		m_bShiftRun = true;
	}
	else
	{
		m_bShiftRun = false;
	}

	CPlayer::Move(dwDirection, fDistance, bUpdateVelocity);
}

void CBlueSuitPlayer::Update(float fElapsedTime)
{
	if (m_bShiftRun)
	{
		m_fStamina -= fElapsedTime;
		if (m_fStamina < 0.0f)
		{
			m_bAbleRun = false;
			m_bShiftRun = false;
		}
	}
	else if(m_fStamina < 5.0f)
	{
		m_fStamina += fElapsedTime;
		if (!m_bAbleRun && m_fStamina > 3.0f)
		{
			m_bAbleRun = true;
		}
	}


	CPlayer::Update(fElapsedTime);

	if (m_pSkinnedAnimationController)
	{
		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);

		if (::IsZero(fLength))	//속력이 0이면 트랙 0을 다시 true
		{
			if (m_pSkinnedAnimationController->m_nNowState != PlayerState::IDLE)
			{
				if (!m_pSkinnedAnimationController->m_bTransition)
				{
					m_pSkinnedAnimationController->m_bTransition = true;
					if (m_pSkinnedAnimationController->m_nNowState == PlayerState::RUN)
					{
						m_pSkinnedAnimationController->m_nNextState = PlayerState::WALK;
						m_pSkinnedAnimationController->SetTrackPosition(2, m_pSkinnedAnimationController->m_pAnimationTracks[3].m_fPosition);
						m_pSkinnedAnimationController->SetTrackPosition(1, m_pSkinnedAnimationController->m_pAnimationTracks[3].m_fPosition);
					}
					else
					{
						m_pSkinnedAnimationController->m_nNextState = PlayerState::IDLE;
					}
				}
			}
		}
		else
		{
			if (m_pSkinnedAnimationController->m_nNowState == PlayerState::IDLE)
			{
				m_pSkinnedAnimationController->m_bTransition = true;
				m_pSkinnedAnimationController->m_nNextState = PlayerState::WALK;
			}
			else if (m_bShiftRun && m_pSkinnedAnimationController->m_nNowState == PlayerState::WALK)
			{
				m_pSkinnedAnimationController->m_bTransition = true;
				m_pSkinnedAnimationController->m_nNextState = PlayerState::RUN;
				m_pSkinnedAnimationController->SetTrackPosition(3, m_pSkinnedAnimationController->m_pAnimationTracks[1].m_fPosition);
			}
			else if (!m_bShiftRun && m_pSkinnedAnimationController->m_nNowState == PlayerState::RUN)
			{
				m_pSkinnedAnimationController->m_bTransition = true;
				m_pSkinnedAnimationController->m_nNextState = PlayerState::WALK;
				m_pSkinnedAnimationController->SetTrackPosition(2, m_pSkinnedAnimationController->m_pAnimationTracks[3].m_fPosition);
				m_pSkinnedAnimationController->SetTrackPosition(1, m_pSkinnedAnimationController->m_pAnimationTracks[3].m_fPosition);
			}

			XMFLOAT3 xmf3Direction = Vector3::Normalize(m_xmf3Velocity);
			float fAngle = Vector3::Angle(m_xmf3Look, xmf3Direction);
			float fRightWeight;
			if (Vector3::CrossProduct(m_xmf3Look, m_xmf3Velocity, false).y < 0.0f)
			{
				m_pSkinnedAnimationController->SetTrackSpeed(2, -1.0f);
			}
			else
			{
				m_pSkinnedAnimationController->SetTrackSpeed(2, 1.0f);
			}

			if (fAngle <= 90.0f)
			{
				m_pSkinnedAnimationController->SetTrackSpeed(1, 1.0f);
				fRightWeight = fAngle / 90.0f;
			}
			else
			{
				m_pSkinnedAnimationController->SetTrackSpeed(1, -1.0f);
				fRightWeight = 1 - ((fAngle - 90.0f) / 90.0f);
			}

			m_pSkinnedAnimationController->SetBlendWeight(0, fRightWeight);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 

CZombiePlayer::CZombiePlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CScene* pScene, void* pContext)
	:CPlayer(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pScene, pContext)
{
	m_xmf3Scale = XMFLOAT3(0.1f, 0.1f, 0.1f);
	CLoadedModelInfo* pZombiePlayer = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Asset/Model/Zom.bin");
	SetChild(pZombiePlayer->m_pModelRootObject, true);

	m_pSkinnedAnimationController = new CZombieAnimationController(pd3dDevice, pd3dCommandList, 3, pZombiePlayer);

	m_pSkinnedAnimationController->SetCallbackKeys(1, 2);
#ifdef _WITH_SOUND_RESOURCE
	m_pSkinnedAnimationController->SetCallbackKey(0, 0.1f, _T("Footstep01"));
	m_pSkinnedAnimationController->SetCallbackKey(1, 0.5f, _T("Footstep02"));
	m_pSkinnedAnimationController->SetCallbackKey(2, 0.9f, _T("Footstep03"));
#else

	m_pSkinnedAnimationController->SetCallbackKey(1, 0, 0.1f, (void*)_T("Sound/Footstep01.wav"));
	m_pSkinnedAnimationController->SetCallbackKey(1, 1, 0.9f, (void*)_T("Sound/Footstep02.wav"));
#endif
	CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);

	SetPlayerUpdatedContext(pContext);
	SetCameraUpdatedContext(pContext);
	pScene->m_vShader[SKINNEDANIMATION_STANDARD_SHADER]->AddGameObject(this);

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);
	if (pZombiePlayer) delete pZombiePlayer;
}

CZombiePlayer::~CZombiePlayer()
{
}

void CZombiePlayer::Update(float fElapsedTime)
{
	CPlayer::Update(fElapsedTime);

	if (m_pSkinnedAnimationController)
	{
		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);

		if (::IsZero(fLength))	//속력이 0이면 트랙 0을 다시 true
		{
			if (m_pSkinnedAnimationController->m_nNowState != PlayerState::IDLE)
			{
				if (!m_pSkinnedAnimationController->m_bTransition)
				{
					m_pSkinnedAnimationController->m_bTransition = true;
					m_pSkinnedAnimationController->m_nNextState = PlayerState::IDLE;
				}
			}
		}
		else
		{
			if (m_pSkinnedAnimationController->m_nNowState == PlayerState::IDLE)
			{
				m_pSkinnedAnimationController->m_bTransition = true;
				m_pSkinnedAnimationController->m_nNextState = PlayerState::WALK;
			}
		}
	}


}
