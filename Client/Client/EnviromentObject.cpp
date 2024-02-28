#include "stdafx.h"
#include "EnviromentObject.h"

CDrawerObject::CDrawerObject(char* pstrFrameName, XMFLOAT4X4& xmf4x4World, CMesh* pMesh)
	: CGameObject(pstrFrameName, xmf4x4World, pMesh)
{
	m_xmf3OriginPosition = XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43);
	m_xmf3Forward = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMMATRIX mtxWorld = XMLoadFloat4x4(&m_xmf4x4World);
	m_xmf3Forward = Vector3::TransformNormal(m_xmf3Forward, mtxWorld);
}

CDrawerObject::~CDrawerObject()
{
}

void CDrawerObject::SetOOBB()
{
	//m_vpCollideFrame.push_back(FindFrame("Laboratory_Desk_Drawers_1"));
	//m_OOBB.push_back(m_vpCollideFrame[0]->m_pMesh->GetOOBB());
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

void CDrawerObject::AnimateOOBB()
{
	//m_vpCollideFrame[0]->m_pMesh->GetOOBB().Transform(m_OOBB[0], XMLoadFloat4x4(&m_xmf4x4World));
	// XMStoreFloat4(&m_OOBB[0].Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_OOBB[0].Orientation)));
}

void CDrawerObject::AnimatePicking(float fElapsedTime)
{

}

void CDrawerObject::CallbackPicking()
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

//CDoorObject::CDoorObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
//	: CGameObject(pd3dDevice, pd3dCommandList)
//{
//}
//
//CDoorObject::CDoorObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CLoadedModelInfo* pModelInfo)
//	: CGameObject(pd3dDevice, pd3dCommandList)
//{
//	SetChild(pModelInfo->m_pModelRootObject, true);
//	SetOOBB();
//	SetPosition(0.0f, 0.0f, 50.0f);
//	m_xmf4Quaternion = Vector4::Quaternion(0.0f, 0.0f, 0.0f);
//	Rotate(&m_xmf4Quaternion);
//
//	/*m_pMainDoor = FindFrame("Door_1");*/
//}

CDoorObject::CDoorObject(char* pstrFrameName, XMFLOAT4X4& xmf4x4World, CMesh* pMesh)
	: CGameObject(pstrFrameName, xmf4x4World, pMesh)
{
	m_xmf4Quaternion = Vector4::Quaternion(m_xmf4x4World);
	//Rotate(&m_xmf4Quaternion);
}

CDoorObject::~CDoorObject()
{
}

void CDoorObject::SetOOBB()
{
	//m_vpCollideFrame.push_back(FindFrame("Door_1"));
	//m_OOBB.push_back(m_vpCollideFrame[0]->m_pMesh->GetOOBB());
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
			//Z-up ¸ðµ¨ÀÓ
			XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.0f), XMConvertToRadians(0.0f), fRotationAngle);
			m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
	
			// Instancing °´Ã¼´Â worldÇà·Ä¸¸ ¼öÁ¤ÇØÁÖ¸éµÊ
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
	//int i = 0;
	//for (auto& collideFrame : m_vpCollideFrame)
	//{
	//	collideFrame->m_pMesh->GetOOBB().Transform(m_OOBB[i], XMLoadFloat4x4(&collideFrame->m_xmf4x4World));
	//	XMStoreFloat4(&m_OOBB[i].Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_OOBB[i].Orientation)));
	//	++i;
	//}	
}

void CDoorObject::AnimatePicking(float fElapsedTime)
{
}

void CDoorObject::CallbackPicking()
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