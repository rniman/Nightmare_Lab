#include "stdafx.h"
#include "EnviromentObject.h"

CDrawerObject::CDrawerObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
	: CGameObject(pd3dDevice, pd3dCommandList)
{

}

CDrawerObject::CDrawerObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CLoadedModelInfo* pModelInfo)
	: CGameObject(pd3dDevice, pd3dCommandList)
{
	SetChild(pModelInfo->m_pModelRootObject, true);
	SetOOBB();
	SetPosition(20.0f, 0.0f, 20.0f);
	Rotate(0.0f, 0.0f, 0.0f);
	m_pFirstDrawer = FindFrame("Drawer_2");
	m_pSecondDrawer = FindFrame("Drawer_1");
}


CDrawerObject::~CDrawerObject()
{
}

void CDrawerObject::SetOOBB()
{
	m_vpCollideFrame.push_back(FindFrame("Laboratory_Desk_Drawers_1"));
	m_OOBB.push_back(m_vpCollideFrame[0]->m_pMesh->GetOOBB());
}

void CDrawerObject::AnimateOOBB()
{
	m_vpCollideFrame[0]->m_pMesh->GetOOBB().Transform(m_OOBB[0], XMLoadFloat4x4(&m_xmf4x4World));
	 XMStoreFloat4(&m_OOBB[0].Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_OOBB[0].Orientation)));
}

void CDrawerObject::AnimatePicking(float fElapsedTime)
{
	XMFLOAT3 xmf3DrawerPos = Vector3::TransformCoord(m_pFirstDrawer->GetPosition(), Matrix4x4::Inverse(m_pFirstDrawer->m_pParent->m_xmf4x4World));
	if (m_bOpened)
	{
		if (xmf3DrawerPos.x > -6.0)
		{
			m_pFirstDrawer->MoveStrafe(-fElapsedTime * 3.0f);
		}
	}
	else
	{
		if (xmf3DrawerPos.x <= 0.0)
		{
			m_pFirstDrawer->MoveStrafe(fElapsedTime * 3.0f);
			if(xmf3DrawerPos.x >= 0.0f)
			{
				m_pFirstDrawer->SetPosition(0.0f, 0.0f, 0.0f);
			}
		}
	}
}

void CDrawerObject::CallbackPicking()
{
	if (m_bOpened)
	{
		m_bOpened = false;
	}
	else
	{
		m_bOpened = true;
	}
}

/// <CGameObject - CDrawerObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CDoorObject>

CDoorObject::CDoorObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
	: CGameObject(pd3dDevice, pd3dCommandList)
{
}

CDoorObject::CDoorObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CLoadedModelInfo* pModelInfo)
	: CGameObject(pd3dDevice, pd3dCommandList)
{
	SetChild(pModelInfo->m_pModelRootObject, true);
	SetOOBB();
	SetPosition(0.0f, 0.0f, 50.0f);
	m_xmf4Quaternion = Vector4::Quaternion(0.0f, 90.0f, 0.0f);
	Rotate(&m_xmf4Quaternion);

	m_pMainDoor = FindFrame("Door_1");
}

CDoorObject::~CDoorObject()
{
}

void CDoorObject::SetOOBB()
{
	m_vpCollideFrame.push_back(FindFrame("Door_1"));
	m_OOBB.push_back(m_vpCollideFrame[0]->m_pMesh->GetOOBB());
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
			XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.0f), fRotationAngle, XMConvertToRadians(0.0f));
			m_pMainDoor->m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_pMainDoor->m_xmf4x4ToParent);
			m_pMainDoor->UpdateTransform(&m_pMainDoor->m_pParent->m_xmf4x4World);
		}
	}

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->AdvanceTime(fElapsedTime, this);

	AnimateOOBB();

	if (m_pSibling) m_pSibling->Animate(fElapsedTime);
	if (m_pChild) m_pChild->Animate(fElapsedTime);
}

void CDoorObject::AnimateOOBB()
{
	int i = 0;
	for (auto& collideFrame : m_vpCollideFrame)
	{
		collideFrame->m_pMesh->GetOOBB().Transform(m_OOBB[i], XMLoadFloat4x4(&collideFrame->m_xmf4x4World));
		XMStoreFloat4(&m_OOBB[i].Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_OOBB[i].Orientation)));
		++i;
	}	
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
