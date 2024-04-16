#include "stdafx.h"
#include "ServerPlayer.h"
#include "ServerCollision.h"
#include "ServerEnvironmentObject.h"

CPlayer::CPlayer()
	: CGameObject()
{
	m_xmf3Position = XMFLOAT3(9.f, 0.0f, 13.9);
	m_xmf3OldPosition = m_xmf3Position;

	XMFLOAT3 xmf3Center = XMFLOAT3(0.0f, 0.8f, 0.0f);
	XMFLOAT3 xmf3Extents = XMFLOAT3(0.4f, 0.8f, 0.4f);
	XMFLOAT4 xmf4Orientation;
	XMStoreFloat4(&xmf4Orientation, XMQuaternionIdentity());

	m_voobbOrigin.emplace_back(xmf3Center, xmf3Extents, xmf4Orientation);

	SetFriction(250.0f);
	SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
	SetMaxVelocityXZ(8.0f);
	SetMaxVelocityY(40.0f);

	m_xmf4x4Projection = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(60.0f), ASPECT_RATIO, 0.01f, 100.0f); // 1인칭
	//m_xmf4x4Projection = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(60.0f), ASPECT_RATIO, 1.01f, 100.0f);

	XMMATRIX mtxProjection = XMLoadFloat4x4(&m_xmf4x4Projection);
}

void CPlayer::Update(float fElapsedTime)
{
	if (m_nPlayerId == -1)
	{
		return;
	}

	DWORD dwDirection = 0;
	if (m_pKeysBuffer[VK_LBUTTON] & 0xF0) dwDirection |= PRESS_LBUTTON;
	if (m_pKeysBuffer[VK_RBUTTON] & 0xF0) dwDirection |= PRESS_RBUTTON;

	if (m_pKeysBuffer['W'] & 0xF0) dwDirection |= DIR_FORWARD;
	if (m_pKeysBuffer['S'] & 0xF0) dwDirection |= DIR_BACKWARD;
	if (m_pKeysBuffer['A'] & 0xF0) dwDirection |= DIR_LEFT;
	if (m_pKeysBuffer['D'] & 0xF0) dwDirection |= DIR_RIGHT;

	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, 12.5f);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -12.5f);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, 12.5f);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -12.5f);

		Move(xmf3Shift, true);
	}

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
	if (!Vector3::IsZero(xmf3Velocity)) m_xmf3OldVelocity = xmf3Velocity;
	Move(xmf3Velocity, false);

	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fElapsedTime);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));

	printf("%d %d %d\n\n", m_nFloor, m_nWidth, m_nDepth);
}

void CPlayer::Collide(const shared_ptr<CCollisionManager>& pCollisionManager, float fElapsedTime, shared_ptr<CGameObject> pCollided)
{
	XMFLOAT3 xmf3Velocity;
	XMFLOAT3 xmf3NormalOfVelocity = Vector3::Normalize(m_xmf3Velocity);

	XMFLOAT3 xmf3OldPosition = m_xmf3OldPosition;
	m_bCollision = false;

	BoundingBox aabbPlayer;

	XMFLOAT3 xmf3SubVelocity[3];
	xmf3SubVelocity[0] = XMFLOAT3(xmf3NormalOfVelocity.x, 0.0f, xmf3NormalOfVelocity.z);
	xmf3SubVelocity[1] = XMFLOAT3(xmf3NormalOfVelocity.x, 0.0f, 0.0f);
	xmf3SubVelocity[2] = XMFLOAT3(0.0f, 0.0f, xmf3NormalOfVelocity.z);

	xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Gravity);
	float fLength = sqrtf(xmf3Velocity.x * xmf3Velocity.x + xmf3Velocity.z * xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ;
	if (fLength > m_fMaxVelocityXZ)
	{
		xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	float fMaxVelocityY = m_fMaxVelocityY;
	fLength = sqrtf(xmf3Velocity.y * xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY) xmf3Velocity.y *= (fMaxVelocityY / fLength);

	XMFLOAT3 xmf3ResultVelocity = Vector3::ScalarProduct(xmf3Velocity, fElapsedTime, false);

	for (int k = 0; k < 3; ++k)
	{
		m_xmf3Position = xmf3OldPosition;
		CalculateSpace();

		m_bCollision = false;
		xmf3SubVelocity[k] = Vector3::ScalarProduct(xmf3SubVelocity[k], Vector3::Length(xmf3ResultVelocity), false);
		Move(xmf3SubVelocity[k], false);

		OnUpdateToParent();
		aabbPlayer.Center = m_voobbOrigin[0].Center;
		aabbPlayer.Extents = m_voobbOrigin[0].Extents;
		XMVECTOR xmvTranslation = XMVectorSet(m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z, 1.0f);
		aabbPlayer.Transform(aabbPlayer, 1.0f, XMQuaternionIdentity(), xmvTranslation);

		for (int i = m_nWidth - 2; i <= m_nWidth +  2 && !m_bCollision; ++i)
		{
			for (int j = m_nDepth - 2; j <= m_nDepth + 2 && !m_bCollision; ++j)
			{
				if (i < 0 || i >= pCollisionManager->GetWidth() || j < 0 || j >= pCollisionManager->GetDepth())
				{
					continue;
				}

				for (const auto& pGameObject : pCollisionManager->GetSpaceGameObjects(m_nFloor, i, j))
				{
					if (pGameObject->GetCollisionType() == StairTrigger)
					{
						BoundingOrientedBox oobb;
						XMFLOAT4X4 xmf4x4World = pGameObject->GetWorldMatrix();
						pGameObject->GetOOBB(0).Transform(oobb, XMLoadFloat4x4(&xmf4x4World));
						XMStoreFloat4(&oobb.Orientation, XMQuaternionNormalize(XMLoadFloat4(&oobb.Orientation)));
						if (oobb.Intersects(aabbPlayer))
						{
							m_bStair = true;
							shared_ptr<CStairTriggerObject> pStairObject = dynamic_pointer_cast<CStairTriggerObject>(pGameObject);
							if (pStairObject)
							{
								if (pStairObject->GetOffsetY() < 0.0f)
								{
									SetStairY(pStairObject->GetY() - 0.2f, pStairObject->GetY() - 0.2f - 4.5f);
								}
								else
								{
									SetStairY(pStairObject->GetY() - 0.2f + 4.5f, pStairObject->GetY() - 0.2f);
								}
								m_xmf4StairPlane = pStairObject->GetStairPlane();
							}
						}
						continue;
					}

					if (!pGameObject || pGameObject->GetCollisionType() != Standard) //임시로 2면 넘김
					{
						continue;
					}

					for (const auto& oobbOrigin : pGameObject->GetVectorOOBB())
					{
						BoundingOrientedBox oobb;
						XMFLOAT4X4 xmf4x4World = pGameObject->GetWorldMatrix();
						oobbOrigin.Transform(oobb, XMLoadFloat4x4(&xmf4x4World));
						XMStoreFloat4(&oobb.Orientation, XMQuaternionNormalize(XMLoadFloat4(&oobb.Orientation)));

						if (oobb.Intersects(aabbPlayer))
						{
							m_bCollision = true;
							break;
						}
					}

					if (m_bCollision)
					{
						break;
					}
				}
			}
		}
		if (!m_bCollision)
		{
			if (!Vector3::IsZero(xmf3SubVelocity[k]))
			{
				m_xmf3OldVelocity = xmf3SubVelocity[k];
			}
			break;
		}
	}

	if (m_bCollision)
	{
		m_xmf3Position = m_xmf3OldPosition = xmf3OldPosition;
		CalculateSpace();
	}
	OnUpdateToParent();
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else //속력이 확정되면 이것이 작동함 or 강제로 위치를 옮길때
	{
		if (!Vector3::IsZero(xmf3Shift))
		{
			m_xmf3OldPosition = m_xmf3Position;
		}
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		CalculateSpace();
	}
}

void CPlayer::CalculateSpace()
{
	m_nWidth = static_cast<int>((m_xmf3Position.x - GRID_START_X) / SPACE_SIZE_XZ);
	m_nFloor = static_cast<int>((m_xmf3Position.y - GRID_START_Y) / SPACE_SIZE_Y);
	m_nDepth = static_cast<int>((m_xmf3Position.z - GRID_START_Z) / SPACE_SIZE_XZ);
}

void CPlayer::OnUpdateToParent()
{
	m_xmf4x4ToParent._11 = m_xmf3Right.x; m_xmf4x4ToParent._12 = m_xmf3Right.y; m_xmf4x4ToParent._13 = m_xmf3Right.z;
	m_xmf4x4ToParent._21 = m_xmf3Up.x; m_xmf4x4ToParent._22 = m_xmf3Up.y; m_xmf4x4ToParent._23 = m_xmf3Up.z;
	m_xmf4x4ToParent._31 = m_xmf3Look.x; m_xmf4x4ToParent._32 = m_xmf3Look.y; m_xmf4x4ToParent._33 = m_xmf3Look.z;
	m_xmf4x4ToParent._41 = m_xmf3Position.x; m_xmf4x4ToParent._42 = m_xmf3Position.y; m_xmf4x4ToParent._43 = m_xmf3Position.z;

	XMMATRIX xmtxScale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	m_xmf4x4ToParent = Matrix4x4::Multiply(xmtxScale, m_xmf4x4ToParent);
}

void CPlayer::SetPickedObject(const shared_ptr<CCollisionManager> pCollisionManager)
{
	if (!IsRecvData())
	{
		return;
	}

	if (!(m_pKeysBuffer[VK_RBUTTON] & 0xF0))
		return;

	m_pPickedObject.reset();

	// 1인칭
	XMFLOAT3 pickPosition;
	pickPosition.x = 0.0f;
	pickPosition.y = 0.0f;
	pickPosition.z = 1.0f;

	float fNearestHitDistance = FLT_MAX;
	for (int i = m_nWidth - 1; i <= m_nWidth + 1; ++i)
	{
		for (int j = m_nDepth - 1; j <= m_nDepth + 1; ++j)
		{
			if (i < 0 || i >= pCollisionManager->GetWidth() || j < 0 || j >= pCollisionManager->GetDepth())
			{
				continue;
			}

			for (auto& pCollisionObject : pCollisionManager->GetSpaceGameObjects(m_nFloor, i, j))
			{
				if (!pCollisionObject || pCollisionObject->GetCollisionType() == COLLISION_TYPE::None) //
				{
					continue;
				}

				float fHitDistance = FLT_MAX;
				if (CGameObject::CheckPicking(pCollisionObject, pickPosition, m_xmf4x4View, fHitDistance))
				{
					if (fHitDistance < fNearestHitDistance)
					{
						fNearestHitDistance = fHitDistance;
						m_pPickedObject = pCollisionObject;
					}
				}
			}
		}
	}
}

////
////
////

CBlueSuitPlayer::CBlueSuitPlayer()
	: CPlayer()
{
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
	else if (m_fStamina < 5.0f)
	{
		m_fStamina += fElapsedTime;
		if (!m_bAbleRun && m_fStamina > 3.0f)
		{
			m_bAbleRun = true;
		}
	}

	CPlayer::Update(fElapsedTime);
}

void CBlueSuitPlayer::UpdatePicking()
{
	shared_ptr<CGameObject> pPickiedObject = m_pPickedObject.lock();
	if (!pPickiedObject)
	{
		return;
	}

	if (!(m_pKeysBuffer['E'] & 0xF0))
	{
		m_bPressed = false;
		return;
	}
	else if (!m_bPressed)	// Press가 지속됨
	{
		m_bPressed = true;
	}
	else
	{
		return;
	}

	if (AddItem(pPickiedObject) != -2)
	{
		pPickiedObject->UpdatePicking();
	}
}

int CBlueSuitPlayer::AddItem(const shared_ptr<CGameObject>& pGameObject)
{
	int nSlot = -2;
	if (dynamic_pointer_cast<CTeleportObject>(pGameObject))
	{
		nSlot = 0;
	}
	else if (dynamic_pointer_cast<CRadarObject>(pGameObject))
	{
		nSlot = 1;
	}
	else if (dynamic_pointer_cast<CMineObject>(pGameObject))
	{
		nSlot = 2;
	}
	else if (dynamic_pointer_cast<CFuseObject>(pGameObject))
	{
		if (dynamic_pointer_cast<CFuseObject>(pGameObject)->GetObtained())
		{
			return nSlot;
		}

		if (m_nFuseNum < 3)
		{
			m_apFuseItems[m_nFuseNum].reset();
			m_apFuseItems[m_nFuseNum] = pGameObject;
			m_nFuseNum++;
			nSlot = -1;
		}
	}
	else
	{
		nSlot = -1;
	}

	if (nSlot <= -1)
	{
		return nSlot;
	}

	if (m_apSlotItems[nSlot].lock())
	{

	}
	else
	{
		m_apSlotItems[nSlot].reset();
		m_apSlotItems[nSlot] = pGameObject;
	}

	return nSlot;
}

void CBlueSuitPlayer::UseItem(int nSlot)
{
	if (nSlot == 3)
	{
		UseFuse();
	}
	else if (shared_ptr<CGameObject> pGameObject = m_apSlotItems[nSlot].lock())
	{
		pGameObject->UpdateUsing(shared_from_this());
		m_apSlotItems[nSlot].reset();
	}
}

void CBlueSuitPlayer::UseFuse()
{
	for (auto& fuseItem : m_apFuseItems)
	{
		if (fuseItem.lock())
		{
			fuseItem.lock()->UpdateUsing(shared_from_this());
			fuseItem.reset();
		}
	}
	m_nFuseNum = 0;
}

void CBlueSuitPlayer::Teleport()
{
	//XMFLOAT3 randomPos = { 4.0f, 4.0f, 4.0f };
	//SetPosition(randomPos);
}