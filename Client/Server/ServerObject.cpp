#include "stdafx.h"
#include "ServerObject.h"

CGameObject::CGameObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb)
{
	strcpy(m_pstrFrameName, pstrFrameName);
	m_xmf4x4World = xmf4x4World;
	m_xmf4x4ToParent = xmf4x4World;
	m_voobbOrigin = voobb;
}

void CGameObject::Update(float fElapsedTime)
{
}

void CGameObject::Move(XMFLOAT3 xmf3Offset)
{
	m_xmf4x4ToParent._41 += xmf3Offset.x;
	m_xmf4x4ToParent._42 += xmf3Offset.y;
	m_xmf4x4ToParent._43 += xmf3Offset.z;

	UpdateTransform(NULL);
}

void CGameObject::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4ToParent, *pxmf4x4Parent) : m_xmf4x4ToParent;

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);
}

bool CGameObject::CheckPicking(const shared_ptr<CGameObject>& pCollisionGameObject, const XMFLOAT3& xmf3PickPosition, const XMFLOAT4X4& xmf4x4ViewMatrix, float& fDistance)
{
	if (!pCollisionGameObject)
	{
		return false;
	}

	XMFLOAT4X4 xmf4x4WorldMatrix = pCollisionGameObject->m_xmf4x4World;
	XMFLOAT4X4 xmf4x4ModelMatrix;
	XMStoreFloat4x4(&xmf4x4ModelMatrix, XMMatrixInverse(nullptr, XMLoadFloat4x4(&xmf4x4WorldMatrix) * XMLoadFloat4x4(&xmf4x4ViewMatrix)));
	XMMATRIX mtxModelMatrix = XMMatrixInverse(nullptr, XMLoadFloat4x4(&xmf4x4WorldMatrix) * XMLoadFloat4x4(&xmf4x4ViewMatrix));

	XMFLOAT3 xmf3CameraPosition{ 0.0f, 0.0f, 0.0f };
	XMVECTOR xmvCameraOrigin = XMVector3TransformCoord(XMLoadFloat3(&xmf3CameraPosition), mtxModelMatrix);
	XMFLOAT3 xmf3CameraOrigin;
	XMStoreFloat3(&xmf3CameraOrigin, xmvCameraOrigin);

	XMVECTOR xmvPickDirection = XMVector3Normalize(XMVector3TransformCoord(XMLoadFloat3(&xmf3PickPosition), mtxModelMatrix) - xmvCameraOrigin);
	XMFLOAT3 xmf3PickDirection;
	XMStoreFloat3(&xmf3PickDirection, xmvPickDirection);

	for (auto& oobb : pCollisionGameObject->GetVectorOOBB())
	{
		XMFLOAT4X4 xmf4x4ToModel = Matrix4x4::Inverse(pCollisionGameObject->m_xmf4x4ToParent);
		XMMATRIX mtxToModel = XMLoadFloat4x4(&xmf4x4ToModel);
		if (oobb.Intersects(xmvCameraOrigin, xmvPickDirection, fDistance))
		{
			return true;
		}
	}

	return false;
}