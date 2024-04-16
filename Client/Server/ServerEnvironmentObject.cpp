#include "stdafx.h"
#include "ServerEnvironmentObject.h"
#include "ServerPlayer.h"

/// <CGameObject - CItemObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CEnvironmentObject>

CEnvironmentObject::CEnvironmentObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb)
	: CGameObject(pstrFrameName, xmf4x4World, voobb)
{
	m_nCollisionType = Standard;
}

/// <CGameObject - CEnvironmentObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CDoorObject>

CDrawerObject::CDrawerObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb)
	: CGameObject(pstrFrameName, xmf4x4World, voobb)
{
	m_nCollisionType = Picking;
	
	m_xmf3OriginPosition = XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43);
	m_xmf3Forward = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMMATRIX mtxWorld = XMLoadFloat4x4(&m_xmf4x4World);
	m_xmf3Forward = Vector3::TransformNormal(m_xmf3Forward, mtxWorld);
}

CDrawerObject::~CDrawerObject()
{
}

void CDrawerObject::Update(float fElapsedTime)
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

CDoorObject::CDoorObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb)
	: CGameObject(pstrFrameName, xmf4x4World, voobb)
{
	m_nCollisionType = Standard;
}

CDoorObject::~CDoorObject()
{
}

void CDoorObject::Update(float fElapsedTime)
{
	if ((m_bOpened && m_fRotationAngle < m_fDoorAngle) || (!m_bOpened && m_fRotationAngle > m_fDoorAngle))
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

CElevatorDoorObject::CElevatorDoorObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb)
	: CGameObject(pstrFrameName, xmf4x4World, voobb)
{
	m_nCollisionType = Standard;

	m_xmf3OriginPosition = XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43);
	m_xmf3Right = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMMATRIX mtxWorld = XMLoadFloat4x4(&m_xmf4x4World);
	m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, mtxWorld);
}

void CElevatorDoorObject::Update(float fElapsedTime)
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

CTeleportObject::~CTeleportObject()
{
}

//void CTeleportObject::Update(float fElapsedTime)
//{
//	CGameObject::Animate(fElapsedTime);
//}

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

CMineObject::~CMineObject()
{
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

CFuseObject::~CFuseObject()
{
}

void CFuseObject::UpdatePicking()
{
	m_bObtained = true;
	m_bCollision = false;
}

void CFuseObject::UpdateUsing(const shared_ptr<CGameObject>& pGameObject)
{
	shared_ptr<CBlueSuitPlayer> pBlueSuitPlayer = dynamic_pointer_cast<CBlueSuitPlayer>(pGameObject);
	if (!pBlueSuitPlayer)
	{
		return;
	}
	m_bObtained = false;
	m_bCollision = true;
}

/// <CGameObject - CFuseObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CRadarObject>

CRadarObject::~CRadarObject()
{
}

void CRadarObject::UpdatePicking()
{
}

void CRadarObject::UpdateUsing(const shared_ptr<CGameObject>& pGameObject)
{

}

/// <CGameObject - CRadarObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CStairTriggerObject>

CStairTriggerObject::CStairTriggerObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb)
	: CGameObject(pstrFrameName, xmf4x4World, voobb)
{
	m_nCollisionType = COLLISION_TYPE::StairTrigger;
	XMFLOAT3 xmf3Position = XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
	XMFLOAT3 xmf3Point1, xmf3Point2, xmf3Point3;

	/*float fx, fy, fz, fOffsetY;*/
	if (xmf3Position.y < 1.0f) // 1Ãþ
	{
		m_fy = 0.2f;
	}
	else if (xmf3Position.y < 5.5f) // 2Ãþ
	{
		m_fy = 4.7f;
	}
	else if (xmf3Position.y < 10.0f) // 3Ãþ
	{
		m_fy = 9.2f;
	}
	else if (xmf3Position.y < 14.5f) // 4Ãþ
	{
		m_fy = 13.7f;
	}

	if (xmf3Position.z > 0.0f) 
	{
		if (xmf3Position.x > 0.0f) // °è´Ü ¾Æ·¡
		{
			m_fOffsetY = 4.5f;
			m_fx = 8.0f;
		}
		else
		{
			m_fOffsetY = -4.5f;
			m_fx = -8.0f;
		}
		m_fz = 13.9;
	}
	else
	{
		if (xmf3Position.x > 0.0f) // °è´Ü À§
		{
			m_fOffsetY = -4.5f;
			m_fx = 8.0f;
		}
		else
		{
			m_fOffsetY = 4.5f;
			m_fx = -8.0f;
		}
		m_fz = -13.9;
	}
	xmf3Point1 = XMFLOAT3(m_fx, m_fy, m_fz + 3.5f / 2.f);
	xmf3Point2 = XMFLOAT3(m_fx, m_fy, m_fz - 3.5f / 2.f);
	xmf3Point3 = XMFLOAT3(-m_fx, m_fy + m_fOffsetY, m_fz + 3.5f / 2.f);
	m_xmf4Plane = Plane::CreateFromPoints(xmf3Point1, xmf3Point2, xmf3Point3);
}
