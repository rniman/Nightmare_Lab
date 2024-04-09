#include "stdafx.h"
#include "Player.h"
#include "EnviromentObject.h"

CItemObject::CItemObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
	: CGameObject(pd3dDevice, pd3dCommandList)
{
}

void CItemObject::Render(ID3D12GraphicsCommandList* pd3dCommandList) 
{
	if (m_bObtained) // 그리지 않는다
	{
		return;
	}

	CGameObject::Render(pd3dCommandList);
}

/// <CGameObject - CItemObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CEnviromentObejct>

CEnviromentObejct::CEnviromentObejct(char* pstrFrameName, XMFLOAT4X4& xmf4x4World, CMesh* pMesh)
	: CGameObject(pstrFrameName, xmf4x4World, pMesh)
{
	m_nCollisionType = 1;
}

/// <CGameObject - CEnviromentObejct>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CDoorObject>

CDrawerObject::CDrawerObject(char* pstrFrameName, XMFLOAT4X4& xmf4x4World, CMesh* pMesh)
	: CGameObject(pstrFrameName, xmf4x4World, pMesh)
{
	m_nCollisionType = 2;

	m_xmf3OriginPosition = XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43);
	m_xmf3Forward = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMMATRIX mtxWorld = XMLoadFloat4x4(&m_xmf4x4World);
	m_xmf3Forward = Vector3::TransformNormal(m_xmf3Forward, mtxWorld);
}

CDrawerObject::~CDrawerObject()
{
}

void CDrawerObject::Animate(float fElapsedTime)
{
	XMFLOAT3 xmf3Position = XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
	float fDistance = Vector3::Distance(xmf3Position, m_xmf3OriginPosition);

	if (m_bAnimate)
	{
		if (m_bOpened)
		{
			if (fDistance < 0.6f)
			{
				XMFLOAT3 xmf3Offset = Vector3::ScalarProduct(m_xmf3Forward, fElapsedTime * 2.0f);
				Move(xmf3Offset);
			}
			else
			{
				m_bAnimate = false;
			}
		}
		else
		{
			if (fDistance >= 0.0f)
			{
				XMFLOAT3 xmf3Offset = Vector3::ScalarProduct(m_xmf3Forward, -fElapsedTime * 2.0f);
				Move(xmf3Offset);
				xmf3Position = XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
				XMFLOAT3 xmf3ToPosition = Vector3::Subtract(xmf3Position, m_xmf3OriginPosition);
				if (Vector3::DotProduct(m_xmf3Forward, xmf3ToPosition) < 0.0f)
				{
					m_xmf4x4World._41 = m_xmf3OriginPosition.x;
					m_xmf4x4World._42 = m_xmf3OriginPosition.y;
					m_xmf4x4World._43 = m_xmf3OriginPosition.z;
					m_bAnimate = false;
				}
			}
		}
	}

}

void CDrawerObject::UpdatePicking()
{
	if (m_bOpened)
	{
		m_bOpened = false;
		m_bAnimate = true;
	}
	else
	{
		m_bOpened = true;
		m_bAnimate = true;
	}
}

/// <CGameObject - CDrawerObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CDoorObject>

CDoorObject::CDoorObject(char* pstrFrameName, XMFLOAT4X4& xmf4x4World, CMesh* pMesh)
	: CGameObject(pstrFrameName, xmf4x4World, pMesh)
{
	m_nCollisionType = 2;
	m_xmf4Quaternion = Vector4::Quaternion(m_xmf4x4World);
	//Rotate(&m_xmf4Quaternion);
}

CDoorObject::~CDoorObject()
{

}

void CDoorObject::Animate(float fElapsedTime)
{
	if( (m_bOpened && m_fRotationAngle < m_fDoorAngle) || (!m_bOpened && m_fRotationAngle > m_fDoorAngle) )
	{
		m_fRotationAngle += m_bOpened ? fElapsedTime * 60.0f : -fElapsedTime * 60.0f;
		float fRotationAngle = m_bOpened ? XMConvertToRadians(fElapsedTime * 60.0f) : -XMConvertToRadians(fElapsedTime * 60.0f);
		if ((m_bOpened && m_fRotationAngle > m_fDoorAngle) || (!m_bOpened && m_fRotationAngle < m_fDoorAngle))
		{
			m_fRotationAngle = m_fDoorAngle;
		}
		else
		{
			//Z-up 모델임
			XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.0f), XMConvertToRadians(0.0f), fRotationAngle);
			m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
	
			// Instancing 객체는 world행렬만 수정해주면됨
			//UpdateTransform(NULL);
		}
	}

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->AdvanceTime(fElapsedTime, this);

	AnimateOOBB();

	if (m_pSibling) m_pSibling->Animate(fElapsedTime);
	if (m_pChild) m_pChild->Animate(fElapsedTime);
}

void CDoorObject::AnimateOOBB() 
{
	CGameObject::AnimateOOBB();
}

void CDoorObject::UpdatePicking()
{
	if (m_bOpened)
	{
		m_bOpened = false;
		m_fDoorAngle = 0.0f;
	}
	else
	{
		m_bOpened = true;
		m_fDoorAngle = 150.0f;
	}
}

/// <CGameObject - CDrawerObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CElevatorDoorObject>

CElevatorDoorObject::CElevatorDoorObject(char* pstrFrameName, XMFLOAT4X4& xmf4x4World, CMesh* pMesh)
	: CGameObject(pstrFrameName, xmf4x4World, pMesh)
{
	m_nCollisionType = 2;

	m_xmf3OriginPosition = XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43);
	m_xmf3Right = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMMATRIX mtxWorld = XMLoadFloat4x4(&m_xmf4x4World);
	m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, mtxWorld);
}

void CElevatorDoorObject::Animate(float fElapsedTime)
{
	XMFLOAT3 xmf3Position = XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
	float fDistance = Vector3::Distance(xmf3Position, m_xmf3OriginPosition);

	if (m_bAnimate)
	{
		if (m_bOpened)
		{
			if (fDistance < 3.0f)
			{
				XMFLOAT3 xmf3Offset = Vector3::ScalarProduct(m_xmf3Right, fElapsedTime * 2.0f);
				Move(xmf3Offset);
			}
			else
			{
				m_bAnimate = false;
			}
		}
		else
		{
			if (fDistance >= 0.0f)
			{
				XMFLOAT3 xmf3Offset = Vector3::ScalarProduct(m_xmf3Right, -fElapsedTime * 2.0f);
				Move(xmf3Offset);
				xmf3Position = XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
				XMFLOAT3 xmf3ToPosition = Vector3::Subtract(xmf3Position, m_xmf3OriginPosition);
				if (Vector3::DotProduct(m_xmf3Right, xmf3ToPosition) < 0.0f)
				{
					m_xmf4x4World._41 = m_xmf3OriginPosition.x;
					m_xmf4x4World._42 = m_xmf3OriginPosition.y;
					m_xmf4x4World._43 = m_xmf3OriginPosition.z;
					m_bAnimate = false;
				}
			}
		}
	}

	AnimateOOBB();
}

void CElevatorDoorObject::UpdatePicking()
{
	if (m_bOpened)
	{
		m_bOpened = false;
		m_bAnimate = true;
	}
	else
	{
		m_bOpened = true;
		m_bAnimate = true;
	}
}


/// <CGameObject - CElevatorDoorObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CItemObject - CTeleportObject>

CTeleportObject::CTeleportObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
	: CItemObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	m_nCollisionType = 2;
}

CTeleportObject::~CTeleportObject() 
{
}

void CTeleportObject::LoadModelAndAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const shared_ptr<CLoadedModelInfo>& pLoadModelInfo)
{
	SetChild(pLoadModelInfo->m_pModelRootObject, true);
	
	LoadBoundingBox(m_voobbOrigin);
}

void CTeleportObject::SetOOBB()
{
}

void CTeleportObject::Animate(float fElapsedTime)
{
	CGameObject::Animate(fElapsedTime);
}

void CTeleportObject::AnimateOOBB()
{
	CGameObject::AnimateOOBB();
}

void CTeleportObject::UpdatePicking()
{
	m_bObtained = true;
}

void CTeleportObject::UpdateUsing(const shared_ptr<CGameObject>& pGameObject)
{
	shared_ptr<CBlueSuitPlayer> pBlueSuitPlayer = dynamic_pointer_cast<CBlueSuitPlayer>(pGameObject);
	if (!pBlueSuitPlayer)
	{
		return;
	}
	pBlueSuitPlayer->Teleport();
	m_bObtained = false;
}

/// <CGameObject - CTeleportObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CMineObject>

CMineObject::CMineObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
	: CItemObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	m_nCollisionType = 2;
}

CMineObject::~CMineObject()
{
}

void CMineObject::LoadModelAndAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const shared_ptr<CLoadedModelInfo>& pLoadModelInfo)
{
	SetChild(pLoadModelInfo->m_pModelRootObject, true);

	LoadBoundingBox(m_voobbOrigin);
}

void CMineObject::SetOOBB()
{
}

void CMineObject::Animate(float fElapsedTime)
{
}

void CMineObject::AnimateOOBB()
{
	CGameObject::AnimateOOBB();
}

void CMineObject::UpdatePicking()
{
}

void CMineObject::UpdateUsing(const shared_ptr<CGameObject>& pGameObject)
{
}

/// <CGameObject - CMineObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CFuseObject>

CFuseObject::CFuseObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
	: CItemObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	m_nCollisionType = 2;
}

CFuseObject::~CFuseObject()
{
}

void CFuseObject::LoadModelAndAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const shared_ptr<CLoadedModelInfo>& pLoadModelInfo)
{
	SetChild(pLoadModelInfo->m_pModelRootObject, true);

	LoadBoundingBox(m_voobbOrigin);
}

void CFuseObject::SetOOBB()
{
}

void CFuseObject::Animate(float fElapsedTime)
{
	CGameObject::Animate(fElapsedTime);
}

void CFuseObject::AnimateOOBB()
{
	CGameObject::AnimateOOBB();
}

void CFuseObject::UpdatePicking()
{
	m_bObtained = true;
	m_bCollsion = false;
}

void CFuseObject::UpdateUsing(const shared_ptr<CGameObject>& pGameObject)
{
	shared_ptr<CBlueSuitPlayer> pBlueSuitPlayer = dynamic_pointer_cast<CBlueSuitPlayer>(pGameObject);
	if (!pBlueSuitPlayer)
	{
		return;
	}
	m_bObtained = false;
	m_bCollsion = true;
}

/// <CGameObject - CFuseObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CRadarObject>

CRadarObject::CRadarObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
	: CItemObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	m_nCollisionType = 2;
}

CRadarObject::~CRadarObject()
{
}

void CRadarObject::LoadModelAndAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const shared_ptr<CLoadedModelInfo>& pLoadModelInfo)
{
	SetChild(pLoadModelInfo->m_pModelRootObject, true);

	LoadBoundingBox(m_voobbOrigin);
}

void CRadarObject::SetOOBB()
{
}

void CRadarObject::Animate(float fElapsedTime)
{
}

void CRadarObject::AnimateOOBB()
{
	CGameObject::AnimateOOBB();
}

void CRadarObject::UpdatePicking()
{
}

void CRadarObject::UpdateUsing(const shared_ptr<CGameObject>& pGameObject)
{
	
}

