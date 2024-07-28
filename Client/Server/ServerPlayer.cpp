#include "stdafx.h"
#include "ServerPlayer.h"
#include "ServerCollision.h"
#include "ServerEnvironmentObject.h"

CServerPlayer::CServerPlayer()
	: CServerGameObject()
{
	//ZeroMemory(m_pKeysBuffer, 256);

	//m_xmf3Position = XMFLOAT3(9.f, 0.0f, 13.9);
	//m_xmf3OldPosition = m_xmf3Position;

	XMFLOAT3 xmf3Center = XMFLOAT3(0.0f, 0.8f, 0.0f);
	XMFLOAT3 xmf3Extents = XMFLOAT3(0.5f, 0.8f, 0.5f);
	XMFLOAT4 xmf4Orientation;
	XMStoreFloat4(&xmf4Orientation, XMQuaternionIdentity());

	m_voobbOrigin.emplace_back(xmf3Center, xmf3Extents, xmf4Orientation);

	SetFriction(250.0f);
	SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
	SetMaxVelocityXZ(BLUESUIT_WALK_VELCOCITY);
	SetMaxVelocityY(8.0f);

	m_xmf4x4Projection = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(60.0f), ASPECT_RATIO, 0.01f, 100.0f); // 1인칭
	//m_xmf4x4Projection = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(60.0f), ASPECT_RATIO, 1.01f, 100.0f);

	XMMATRIX mtxProjection = XMLoadFloat4x4(&m_xmf4x4Projection);
}

void CServerPlayer::Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	m_fCoolTimeInvincibility -= fElapsedTime;
	if (m_bInvincibility && m_fCoolTimeInvincibility < 0.0f)
	{
		m_bInvincibility = false;
	}

	DWORD dwDirection = 0;

	if (m_wKeyBuffer & KEY_W) dwDirection |= DIR_FORWARD;
	if (m_wKeyBuffer & KEY_S) dwDirection |= DIR_BACKWARD;
	if (m_wKeyBuffer & KEY_A) dwDirection |= DIR_LEFT;
	if (m_wKeyBuffer & KEY_D) dwDirection |= DIR_RIGHT;

	//if (m_wKeyBuffer & KEY_LBUTTON) dwDirection |= PRESS_LBUTTON;
	//if (m_wKeyBuffer & KEY_RBUTTON) dwDirection |= PRESS_RBUTTON;

	if (!m_bAlive)
	{
		if (m_fDeathTime < 4.0f)
		{
			m_fDeathTime += fElapsedTime;
			return;
		}

		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, 12.5f * fElapsedTime);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -12.5f * fElapsedTime);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, 12.5f * fElapsedTime);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -12.5f * fElapsedTime);

		Move(xmf3Shift, false);
		return;
	}

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
}

void CServerPlayer::Declare(float fElapsedTime)
{
	float fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fElapsedTime);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
}

void CServerPlayer::Collide(const shared_ptr<CServerCollisionManager>& pCollisionManager, float fElapsedTime, shared_ptr<CServerGameObject> pCollided)
{
	XMFLOAT3 xmf3Pos = m_xmf3Position;
	XMFLOAT3 xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3 xmf3VelNormal = m_xmf3Velocity;
	xmf3VelNormal = Vector3::Normalize(xmf3VelNormal);

	XMFLOAT3 xmf3Add = Vector3::CrossProduct(xmf3VelNormal, xmf3Up, true);
	XMFLOAT3 xmf3Point1 = Vector3::Normalize(xmf3Add);
	XMFLOAT3 xmf3Point2 = Vector3::ScalarProduct(xmf3Point1, -1.0f, true);

	XMFLOAT3 xmf3Velocity;
	XMFLOAT3 xmf3NormalOfVelocity = Vector3::Normalize(m_xmf3Velocity);

	XMFLOAT3 xmf3OldPosition = m_xmf3OldPosition;
	XMFLOAT3 xmf3Position = m_xmf3Position;
	m_bOccurredCollision = false;

	//BoundingBox aabbPlayer; 
	BoundingSphere oobbPlayer;

	XMFLOAT3 xmf3SubVelocity[2];
	xmf3SubVelocity[0] = xmf3Point1;
	xmf3SubVelocity[1] = xmf3Point2;

	xmf3Velocity = m_xmf3Velocity;
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
	XMFLOAT3 xmf3ReverseVelocity = Vector3::ScalarProduct(xmf3ResultVelocity, -1.0f, false);

	for (int k = 0; k < 2; ++k)
	{
		m_xmf3Position = xmf3Position;
		CalculateSpace();

		m_bOccurredCollision = false;
		xmf3SubVelocity[k] = Vector3::ScalarProduct(xmf3SubVelocity[k], Vector3::Length(xmf3ResultVelocity) * 2.0f, false);
		//xmf3SubVelocity[k] = Vector3::Add(xmf3SubVelocity[k], xmf3ReverseVelocity);
		Move(xmf3SubVelocity[k], false);

		OnUpdateToParent();
		/*aabbPlayer.Center = m_voobbOrigin[0].Center;
		aabbPlayer.Extents = m_voobbOrigin[0].Extents;
		XMVECTOR xmvTranslation = XMVectorSet(m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z, 1.0f);
		aabbPlayer.Transform(aabbPlayer, 1.0f, XMQuaternionIdentity(), xmvTranslation);*/
		oobbPlayer.Center = m_voobbOrigin[0].Center;
		oobbPlayer.Radius = m_voobbOrigin[0].Extents.z;
		oobbPlayer.Transform(oobbPlayer, XMLoadFloat4x4(&m_xmf4x4World));
		//XMStoreFloat4(&oobbPlayer.Orientation, XMQuaternionNormalize(XMLoadFloat4(&oobbPlayer.Orientation)));

		for (int i = m_nWidth - 2; i <= m_nWidth + 2 && !m_bOccurredCollision; ++i)
		{
			for (int j = m_nDepth - 2; j <= m_nDepth + 2 && !m_bOccurredCollision; ++j)
			{
				if (i < 0 || i >= pCollisionManager->GetWidth() || j < 0 || j >= pCollisionManager->GetDepth())
				{
					continue;
				}

				for (const auto& pGameObject : pCollisionManager->GetSpaceGameObjects(m_nFloor, i, j))
				{
					if (!pGameObject || !pGameObject->IsCollision())
					{
						continue;
					}

					if (pGameObject->GetCollisionType() == StairTrigger)
					{
						CheckStairTrigger(pGameObject, oobbPlayer);
						continue;
					}

					if (pGameObject->GetCollisionType() != Standard) //임시로 2면 넘김
					{
						continue;
					}

					for (const auto& oobbOrigin : pGameObject->GetVectorOOBB())
					{
						BoundingOrientedBox oobb;
						XMFLOAT4X4 xmf4x4World = pGameObject->GetWorldMatrix();
						oobbOrigin.Transform(oobb, XMLoadFloat4x4(&xmf4x4World));
						XMStoreFloat4(&oobb.Orientation, XMQuaternionNormalize(XMLoadFloat4(&oobb.Orientation)));

						if (oobb.Intersects(oobbPlayer))
						{
							m_bOccurredCollision = true;
							break;
						}
					}

					if (m_bOccurredCollision)
					{
						break;
					}
				}
			}
		}
		if (!m_bOccurredCollision)
		{
			if (!Vector3::IsZero(xmf3SubVelocity[k]))
			{
				m_xmf3OldVelocity = xmf3SubVelocity[k];
			}
			break;
		}
	}

	if (m_bOccurredCollision)
	{
		m_xmf3OldPosition = xmf3OldPosition;
		m_xmf3Position = m_xmf3OldPosition;
		//Move(xmf3ReverseVelocity, false);

		CalculateSpace();
	}
	OnUpdateToParent();
}

void CServerPlayer::CheckStairTrigger(const std::shared_ptr<CServerGameObject>& pGameObject, DirectX::BoundingSphere& aabbPlayer)
{
	BoundingOrientedBox oobb;
	XMFLOAT4X4 xmf4x4World = pGameObject->GetWorldMatrix();
	pGameObject->GetOOBB(0).Transform(oobb, XMLoadFloat4x4(&xmf4x4World));
	XMStoreFloat4(&oobb.Orientation, XMQuaternionNormalize(XMLoadFloat4(&oobb.Orientation)));
	if (oobb.Intersects(aabbPlayer))
	{
		m_bStair = true;
		shared_ptr<CServerStairTriggerObject> pStairObject = dynamic_pointer_cast<CServerStairTriggerObject>(pGameObject);
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
}

void CServerPlayer::CollideWithPlayer(const shared_ptr<CServerCollisionManager>& pCollisionManager, float fElapsedTime, shared_ptr<CServerPlayer> pCollidedPlayer)
{
	XMFLOAT3 xmf3Pos = m_xmf3Position;
	XMFLOAT3 xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3 xmf3VelNormal = m_xmf3Velocity;
	xmf3VelNormal = Vector3::Normalize(xmf3VelNormal);

	XMFLOAT3 xmf3Add = Vector3::CrossProduct(xmf3VelNormal, xmf3Up, true);
	XMFLOAT3 xmf3Point1 = Vector3::Normalize(xmf3Add);
	XMFLOAT3 xmf3Point2 = Vector3::ScalarProduct(xmf3Point1, -1.0f, true);

	XMFLOAT3 xmf3Velocity;
	XMFLOAT3 xmf3NormalOfVelocity = Vector3::Normalize(m_xmf3Velocity);

	XMFLOAT3 xmf3OldPosition = m_xmf3OldPosition;
	XMFLOAT3 xmf3Position = m_xmf3Position;
	m_bOccurredCollision = false;

	//BoundingBox aabbPlayer; 
	BoundingSphere oobbPlayer;

	XMFLOAT3 xmf3SubVelocity[2];
	xmf3SubVelocity[0] = xmf3Point1;
	xmf3SubVelocity[1] = xmf3Point2;

	xmf3Velocity = m_xmf3Velocity;
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
	XMFLOAT3 xmf3ReverseVelocity = Vector3::ScalarProduct(xmf3ResultVelocity, -1.0f, false);

	for (int k = 0; k < 2; ++k)
	{
		m_xmf3Position = xmf3Position;
		CalculateSpace();

		m_bOccurredCollision = false;
		xmf3SubVelocity[k] = Vector3::ScalarProduct(xmf3SubVelocity[k], Vector3::Length(xmf3ResultVelocity) * 2.0f, false);
		//xmf3SubVelocity[k] = Vector3::Add(xmf3SubVelocity[k], xmf3ReverseVelocity);
		Move(xmf3SubVelocity[k], false);

		OnUpdateToParent();

		oobbPlayer.Center = m_voobbOrigin[0].Center;
		oobbPlayer.Radius = m_voobbOrigin[0].Extents.z;
		oobbPlayer.Transform(oobbPlayer, XMLoadFloat4x4(&m_xmf4x4World));

		BoundingSphere spherebb;
		spherebb.Center = pCollidedPlayer->GetVectorOOBB()[0].Center;
		spherebb.Radius = pCollidedPlayer->GetVectorOOBB()[0].Extents.z;
		XMFLOAT4X4 xmf4x4OtherWorld = pCollidedPlayer->GetWorldMatrix();
		spherebb.Transform(spherebb, XMLoadFloat4x4(&xmf4x4OtherWorld));

		if (spherebb.Intersects(oobbPlayer))
		{
			m_bOccurredCollision = true;
			continue;
		}

		//for (int i = m_nWidth - 2; i <= m_nWidth + 2 && !m_bOccurredCollision; ++i)
		//{
		//	for (int j = m_nDepth - 2; j <= m_nDepth + 2 && !m_bOccurredCollision; ++j)
		//	{
		//		if (i < 0 || i >= pCollisionManager->GetWidth() || j < 0 || j >= pCollisionManager->GetDepth())
		//		{
		//			continue;
		//		}

		//		for (const auto& pGameObject : pCollisionManager->GetSpaceGameObjects(m_nFloor, i, j))
		//		{
		//			if (!pGameObject || !pGameObject->IsCollision())
		//			{
		//				continue;
		//			}

		//			if (pGameObject->GetCollisionType() == StairTrigger)
		//			{
		//				CheckStairTrigger(pGameObject, oobbPlayer);
		//				continue;
		//			}

		//			if (pGameObject->GetCollisionType() != Standard) //임시로 2면 넘김
		//			{
		//				continue;
		//			}

		//			for (const auto& oobbOrigin : pGameObject->GetVectorOOBB())
		//			{
		//				BoundingOrientedBox oobb;
		//				XMFLOAT4X4 xmf4x4World = pGameObject->GetWorldMatrix();
		//				oobbOrigin.Transform(oobb, XMLoadFloat4x4(&xmf4x4World));
		//				XMStoreFloat4(&oobb.Orientation, XMQuaternionNormalize(XMLoadFloat4(&oobb.Orientation)));

		//				if (oobb.Intersects(oobbPlayer))
		//				{
		//					m_bOccurredCollision = true;
		//					break;
		//				}
		//			}

		//			if (m_bOccurredCollision)
		//			{
		//				break;
		//			}
		//		}
		//	}
		//}
		//if (!m_bOccurredCollision)
		//{
		//	if (!Vector3::IsZero(xmf3SubVelocity[k]))
		//	{
		//		m_xmf3OldVelocity = xmf3SubVelocity[k];
		//	}
		//	break;
		//}
	}

	if (m_bOccurredCollision)
	{
		m_xmf3OldPosition = xmf3OldPosition;
		m_xmf3Position = m_xmf3OldPosition;
		//Move(xmf3ReverseVelocity, false);

		CalculateSpace();
	}
	OnUpdateToParent();

}

void CServerPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
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

void CServerPlayer::CalculateSpace()
{
	m_nWidth = static_cast<int>((m_xmf3Position.x - GRID_START_X) / SPACE_SIZE_XZ);
	m_nFloor = static_cast<int>((m_xmf3Position.y - GRID_START_Y) / SPACE_SIZE_Y);
	m_nDepth = static_cast<int>((m_xmf3Position.z - GRID_START_Z) / SPACE_SIZE_XZ);

	if (m_nWidth < 0) m_nWidth = 0;
	if (m_nDepth < 0) m_nDepth = 0;
	if (m_nFloor < 0) m_nFloor = 0;

	if (m_nWidth >= SPACE_WIDTH) m_nWidth = SPACE_WIDTH - 1;
	if (m_nDepth >= SPACE_DEPTH) m_nDepth = SPACE_DEPTH - 1;
	if (m_nFloor >= SPACE_FLOOR) m_nFloor = SPACE_FLOOR - 1;
}

void CServerPlayer::OnUpdateToParent()
{
	m_xmf4x4ToParent._11 = m_xmf3Right.x; m_xmf4x4ToParent._12 = m_xmf3Right.y; m_xmf4x4ToParent._13 = m_xmf3Right.z;
	m_xmf4x4ToParent._21 = m_xmf3Up.x; m_xmf4x4ToParent._22 = m_xmf3Up.y; m_xmf4x4ToParent._23 = m_xmf3Up.z;
	m_xmf4x4ToParent._31 = m_xmf3Look.x; m_xmf4x4ToParent._32 = m_xmf3Look.y; m_xmf4x4ToParent._33 = m_xmf3Look.z;
	m_xmf4x4ToParent._41 = m_xmf3Position.x; m_xmf4x4ToParent._42 = m_xmf3Position.y; m_xmf4x4ToParent._43 = m_xmf3Position.z;

	//XMMATRIX xmtxScale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	//m_xmf4x4ToParent = Matrix4x4::Multiply(xmtxScale, m_xmf4x4ToParent);
	UpdateTransform(NULL);
}

void CServerPlayer::SetPickedObject(const shared_ptr<CServerCollisionManager> pCollisionManager)
{
	if (!IsRecvData() || !IsAlive())
	{
		return;
	}

	m_pPickedObject.reset();

	// 1인칭
	XMFLOAT3 pickPosition;
	pickPosition.x = 0.0f;
	pickPosition.y = 0.0f;
	pickPosition.z = 1.0f;

	// 이값 조정하면 피킹 최소거리 설정 가능
	float fNearestHitDistance = 3.0f;
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
				if (CServerGameObject::CheckPicking(pCollisionObject, pickPosition, m_xmf4x4View, fHitDistance))
				{
					shared_ptr<CServerDrawerObject> pDrawer = dynamic_pointer_cast<CServerDrawerObject>(pCollisionObject);
					if (pDrawer)	// 서랍의 경우 내부에 아이템을 고려해야함
					{
						if (!pDrawer->IsOpen() || !pDrawer->m_pStoredItem)	// 닫혀있거나 내부에 아이템이 없으면 
						{
							if (fHitDistance < fNearestHitDistance)
							{
								fNearestHitDistance = fHitDistance;
								m_pPickedObject = pCollisionObject;
							}
							continue;
						}

						float fItemHitDistance = FLT_MAX;
						if (CServerGameObject::CheckPicking(pDrawer->m_pStoredItem, pickPosition, m_xmf4x4View, fItemHitDistance))
						{
							if (fItemHitDistance < fNearestHitDistance)
							{
								fNearestHitDistance = fHitDistance = fItemHitDistance;
								m_pPickedObject = pDrawer->m_pStoredItem;
							}
						}
						else
						{
							if (fHitDistance < fNearestHitDistance)
							{
								fNearestHitDistance = fHitDistance;
								m_pPickedObject = pCollisionObject;
							}
						}
					}
					else if (fHitDistance < fNearestHitDistance)
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

CServerBlueSuitPlayer::CServerBlueSuitPlayer()
	: CServerPlayer()
{
	m_apSlotItems[Teleport] = make_shared<CServerTeleportObject>();
	m_apSlotItems[Teleport]->SetCollision(false);
	m_apSlotItems[Radar] = make_shared<CServerRadarObject>();
	m_apSlotItems[Radar]->SetCollision(false);
	m_apSlotItems[Mine] = make_shared<CServerMineObject>();
	m_apSlotItems[Mine]->SetCollision(false);

	for (int i = 0;i < 3;++i)
	{
		m_apFuseItems[i] = make_shared<CServerFuseObject>();
		m_apFuseItems[i]->SetCollision(false);
	}
}

void CServerBlueSuitPlayer::UseItem(shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	if (!IsAlive()) // 죽음
	{
		return;
	}

	array<shared_ptr<CServerGameObject>,3> apObjects;
	if (m_wKeyBuffer & KEY_1)
	{
		if (m_apSlotItems[Teleport]->IsObtained()) {
			m_selectItem = RightItem::TELEPORT;
		}
	}
	if (m_wKeyBuffer & KEY_2)
	{
		if (m_apSlotItems[Radar]->IsObtained())
		{
			m_selectItem = RightItem::RADAR;
		}
	}
	if (m_wKeyBuffer & KEY_3)
	{
		if (m_apSlotItems[Mine]->IsObtained())
		{
			m_selectItem = RightItem::LANDMINE;
		}
	}
	if (m_wKeyBuffer & KEY_4)
	{
		if (m_apFuseItems[0]->IsObtained() || m_apFuseItems[1]->IsObtained() || m_apFuseItems[2]->IsObtained())
		{
			m_selectItem = RightItem::FUSE;
		}
	}

}

void CServerBlueSuitPlayer::Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	if ((m_wKeyBuffer & KEY_LSHIFT) && m_bAbleRun)
	{
		m_bRunning = true;
		m_fMaxVelocityXZ = BLUESUIT_WALK_VELCOCITY * 2.7f;
	}
	else if (m_bRunning)
	{
		m_bRunning = false;
		m_fMaxVelocityXZ = BLUESUIT_WALK_VELCOCITY;
	}

	if (m_bRunning)
	{
		m_fStamina -= fElapsedTime;
		if (m_fStamina < 0.0f)
		{
			m_bAbleRun = false;
			m_bRunning = false;
			m_fMaxVelocityXZ = BLUESUIT_WALK_VELCOCITY;
		}
	}
	else if (m_fStamina < BLUESUIT_STAMINA_MAX)
	{
		m_fStamina += fElapsedTime;
		if (!m_bAbleRun && m_fStamina > BLUESUIT_STAMINA_EXHAUSTION)
		{
			m_bAbleRun = true;
			m_fStamina = BLUESUIT_STAMINA_EXHAUSTION;
		}
	}

	if (m_bTeleport)
	{
		m_fTeleportTime += fElapsedTime;
		if (m_fTeleportTime >= BLUESUIT_TELEPORT_TIME) {
			TeleportRandomPosition();
			m_bTeleport = false;
		}
	}

	CServerPlayer::Update(fElapsedTime, pCollisionManager);
}

void CServerBlueSuitPlayer::UpdatePicking(INT8 nClientId)
{
	if (!IsAlive()) // 죽음
	{
		return;
	}

	shared_ptr<CServerGameObject> pPickedObject = m_pPickedObject.lock();
	if (!pPickedObject)
	{
		return;
	}

	if (!(m_wKeyBuffer & KEY_E))
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

	if (AddItem(pPickedObject) != -1)	// 이미 있던 걸 피킹한거 -> 나중에 아예 피킹 안잡히게 수정해야함
	{
		pPickedObject->UpdatePicking(m_nPlayerId);
	}
}

void CServerBlueSuitPlayer::Hit()
{
	m_nHealthPoint -= 1;
	m_bInvincibility = true;
	m_fCoolTimeInvincibility = 1.5f;

	//printf("%d의 현재 체력: %d\n", m_nPlayerId, m_nHealthPoint);

	if (m_nHealthPoint <= 0)
	{
		m_bCollision = false;
		m_bAlive = false;
		m_xmf3OldPosition = m_xmf3Position;

		//PostMessage(TCPServer::m_hWnd, WM_SOUND, (WPARAM)SOUND_MESSAGE::BLUE_SUIT_DEAD, (LPARAM)m_nPlayerId);
	}
}

int CServerBlueSuitPlayer::AddItem(const shared_ptr<CServerGameObject>& pGameObject)
{
	if (!dynamic_pointer_cast<CServerItemObject>(pGameObject))	// 아이템이 아님
	{
		return 0;
	}

	int nSlot = -1;
	if (dynamic_pointer_cast<CServerRadarObject>(pGameObject))
	{
		nSlot = Radar;
	}
	else if (dynamic_pointer_cast<CServerMineObject>(pGameObject))
	{
		nSlot = Mine;
	}
	else if (dynamic_pointer_cast<CServerTeleportObject>(pGameObject))
	{
		nSlot = Teleport;
	}
	else if (dynamic_pointer_cast<CServerFuseObject>(pGameObject))
	{
		if (m_nFuseNum < 3)
		{
			m_apFuseItems[m_nFuseNum]->SetObtain(true);
			m_apFuseItems[m_nFuseNum]->SetReferenceNumber(pGameObject->GetCollisionNum());
			m_nFuseNum++;
			nSlot = 4;
		}
		return nSlot;
	}

	if (m_apSlotItems[nSlot]->IsObtained())	// 획득된 상태라면 획득 할 수없음
	{
		nSlot = -1;
	}
	else
	{
		switch (nSlot)
		{
		case Teleport:
			// 플레이어의 ITME은 ON 시키고 해당 아이템은 OFF 시켜야함
			//m_apSlotItems[nSlot] = dynamic_pointer_cast<CServerTeleportObject>(pGameObject);
			break;
		case Radar:
			//m_apSlotItems[nSlot] = dynamic_pointer_cast<CServerRadarObject>(pGameObject);
			break;
		case Mine:
			//m_apSlotItems[nSlot] = dynamic_pointer_cast<CServerMineObject>(pGameObject);
			break;
		default:
			break;
		}
		m_apSlotItems[nSlot]->SetObtain(true);
		m_apSlotItems[nSlot]->SetReferenceNumber(pGameObject->GetCollisionNum());
	}

	return nSlot;
}

void CServerBlueSuitPlayer::UseFuse(shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	// 나중에 퓨즈 박스앞에서 썼다는거 확인하기 위한값 
	bool bFuseBox = false;

	shared_ptr<CServerGameObject> pObject;
	for (auto& pFuseItem : m_apFuseItems)
	{
		if (pFuseItem && pFuseItem->IsObtained())
		{
			pObject = pCollisionManager->GetCollisionObjectWithNumber(pFuseItem->GetReferenceNumber());

			pObject->UpdateUsing(shared_from_this(), pCollisionManager);
			pFuseItem->SetReferenceNumber(-1);
			pFuseItem->SetObtain(false);
		}
	}
	m_nFuseNum = 0;
}

void CServerBlueSuitPlayer::TeleportRandomPosition()
{

	array<XMFLOAT3, 34> axmf3Positions = {
		XMFLOAT3(10.0f, 0.0f, 13.5),
		XMFLOAT3(10.0f, 0.0f, -13.5),
		XMFLOAT3(-10.0f, 0.0f, 18.5),
		XMFLOAT3(-10.0f, 0.0f, -13.5),
		XMFLOAT3(34.0f, 4.5f, -30.f),
		XMFLOAT3(33.0f, 4.5f, -13.f),

		XMFLOAT3(10.0f, 4.5f, 13.5),
		XMFLOAT3(10.0f, 4.5f, -13.5),
		XMFLOAT3(-10.0f, 4.5f, 13.5),
		XMFLOAT3(-10.0f, 4.5f, -13.5),
		XMFLOAT3(34.0f, 4.5f, -30.f),
		XMFLOAT3(33.0f, 4.5f, -13.f),

		XMFLOAT3(10.0f, 9.0f, 13.5),
		XMFLOAT3(10.0f, 9.0f, -13.5),
		XMFLOAT3(-10.0f, 9.0f, 13.5),
		XMFLOAT3(-10.0f, 9.0f, -13.5),
		XMFLOAT3(34.0f, 4.5f, -30.f),
		XMFLOAT3(33.0f, 4.5f, -13.f),

		XMFLOAT3(10.0f, 13.5f, 13.5),
		XMFLOAT3(10.0f, 13.5f, -13.5),
		XMFLOAT3(-10.0f, 13.5f, 13.5),
		XMFLOAT3(-10.0f, 13.5f, -13.5),

		XMFLOAT3(23.0f, 13.5f, -18.f),
		XMFLOAT3(22.0f, 13.5f, -2.f),
		XMFLOAT3(17.0f, 13.5f, 19.f),
		XMFLOAT3(24.0f, 9.f, -3.f),
		XMFLOAT3(-20.0f, 9.f, -20.f),
		XMFLOAT3(23.0f, 9.f, 17.f),
		XMFLOAT3(23.0f, 4.5f, 16.f),
		XMFLOAT3(34.0f, 4.5f, -30.f),
		XMFLOAT3(33.0f, 4.5f, -13.f),
		XMFLOAT3(20.0f, 4.5f, -32.f),
		XMFLOAT3(-20.0f, 4.5f, -20.f),
		XMFLOAT3(-30.0f, 4.5f, 12.f),
	};

	// 후보지를 두고 int 값에 따라 그곳에 가도록 해야할듯
	uniform_int_distribution<int> disFloatPosition(0, axmf3Positions.size() - 1);

	m_xmf3Position = axmf3Positions[disFloatPosition(TCPServer::m_mt19937Gen)];
	m_xmf3OldPosition = m_xmf3Position;
}

int CServerBlueSuitPlayer::GetReferenceSlotItemNum(int nIndex)
{
	return m_apSlotItems[nIndex]->GetReferenceNumber();
}

int CServerBlueSuitPlayer::GetReferenceFuseItemNum(int nIndex)
{
	return m_apFuseItems[nIndex]->GetReferenceNumber();
}

void CServerBlueSuitPlayer::RightClickProcess(shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	if (!m_bRightClick) {
		return;
	}

	array<shared_ptr<CServerGameObject>, 3> apObjects;

	switch (m_selectItem)
	{
	case RightItem::NONE:
		break;
	case RightItem::RADAR:
		break;
	case RightItem::TELEPORT:
		if (m_apSlotItems[Teleport]->IsObtained())
		{
			apObjects[Teleport] = pCollisionManager->GetCollisionObjectWithNumber(m_apSlotItems[Teleport]->GetReferenceNumber());
			m_selectItem = RightItem::NONE;
		}
		break;
	case RightItem::LANDMINE:
		if (m_apSlotItems[Mine]->IsObtained())
		{
			apObjects[Mine] = pCollisionManager->GetCollisionObjectWithNumber(m_apSlotItems[Mine]->GetReferenceNumber());
			m_selectItem = RightItem::NONE;
		}
		break;
	case RightItem::FUSE: {
		bool notFuse3 = false;
		for (auto& pFuseItem : m_apFuseItems) {
			if (!pFuseItem || !pFuseItem->IsObtained()) {
				notFuse3 = true;
				break;
			}
		}
		if (notFuse3) {
			break;
		}
		auto pPickedObject = m_pPickedObject.lock();

		auto door = dynamic_pointer_cast<CServerElevatorDoorObject>(pPickedObject);
		if (door) 
		{
			if (door->IsEscape())
			{
				door->EscapeDoorOpen(m_nPlayerId);
				UseFuse(pCollisionManager);
				m_selectItem = RightItem::NONE;
				m_bWinner = true;
			}
			else 
			{
				//std::cout << "탈출구가 아닙니다." << std::endl;
			}
		}
	}
		break;
	default:
		break;
	}

	for (int i = 0; i < 3; ++i)
	{
		if (apObjects[i])
		{
			apObjects[i]->UpdateUsing(shared_from_this(), pCollisionManager);
			m_apSlotItems[i]->SetObtain(false);
			m_apSlotItems[i]->SetReferenceNumber(-1);
		}
	}

	SetRightClick(false);
}

void CServerBlueSuitPlayer::TeleportItemUse()
{
	m_bTeleport = true;
	m_fTeleportTime = 0.0f;
}

////
////
////

CServerZombiePlayer::CServerZombiePlayer()
	: CServerPlayer()
{
	SetMaxVelocityXZ(ZOMBIE_WALK_VELOCITY);

	//m_voobbOrigin[0].Center = XMFLOAT3(0.0f, 0.8f, 0.0f);
	m_voobbOrigin[0].Extents = XMFLOAT3(0.5f, 0.8f, 0.7f);

	m_oobbAttackBox.Center = XMFLOAT3(0.0f, 0.8f, 0.0f);
	m_oobbAttackBox.Extents = XMFLOAT3(0.5f, 0.8f, 0.5f);
	XMStoreFloat4(&m_oobbAttackBox.Orientation, XMQuaternionIdentity());
}

void CServerZombiePlayer::UseItem(shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	if (m_bGameStartWait) {
		return;
	}
	//else printf("%f \n", m_fCoolTimeRunning);

	if ((m_wKeyBuffer & KEY_1) && m_fCoolTimeTracking <= 0.0f)	// 추적
	{
		m_fCoolTimeTracking = TRACKING_COOLTIME;
		m_bTracking = true;
	}
	if ((m_wKeyBuffer & KEY_2) && m_fCoolTimeInterruption <= 0.0f)	// 시야방해
	{
		m_fCoolTimeInterruption = INTERRUPTION_COOLTIME;
		m_bInterruption = true;
	}
	if ((m_wKeyBuffer & KEY_3) && m_fCoolTimeRunning <= 0.0f)	// 달리기
	{
		m_fCoolTimeRunning = ZOM_RUNNING_COOLTIME;
		m_bRunning = true;
		m_fMaxVelocityXZ = BLUESUIT_WALK_VELCOCITY * 2.7f - 2.f;
	}
}

void CServerZombiePlayer::Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	if (m_bGameStartWait) {
		m_fGameStartWait -= fElapsedTime;
		if (m_fGameStartWait <= 0.0f) {
			m_bGameStartWait = false;
		}
		return;
	}

	m_fCoolTimeAttack -= fElapsedTime;
	m_fExplosionDelay += fElapsedTime;
	if (m_fNoStopTime > 0.0f) { // 움직임 제한 쿨타임
		m_fNoStopTime -= fElapsedTime;

		if (m_bCollisionMine) {
			m_fStopMove -= fElapsedTime;
			if (m_fStopMove < 0.0f) {
				m_bCollisionMine = false;
			}
			return;
		}
	}

	if (m_fCoolTimeAttack <= 1.0f)
	{
		m_bAttack = false;
	}
	if ((m_wKeyBuffer & KEY_LBUTTON) && m_fCoolTimeAttack <= 0.0f)	// 일반 공격
	{
		// 원래 클라에서는 track 2번을 enable시킴
		m_bAttack = true;
		m_fCoolTimeAttack = 2.0f;
		
		m_oobbAttackBox.Center = XMFLOAT3(0.0f, 0.8f, 0.0f);
		m_oobbAttackBox.Extents = XMFLOAT3(0.5f, 0.8f, 0.5f);
		XMStoreFloat4(&m_oobbAttackBox.Orientation, XMQuaternionIdentity());
		
		OnUpdateToParent();
		m_oobbAttackBox.Transform(m_oobbAttackBox, XMLoadFloat4x4(&m_xmf4x4ToParent));
		XMStoreFloat4(&m_oobbAttackBox.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_oobbAttackBox.Orientation)));

		XMFLOAT3 xmf3Offset = Vector3::ScalarProduct(m_xmf3Look, 0.8f, false);
		m_oobbAttackBox.Center.x += xmf3Offset.x;
		m_oobbAttackBox.Center.y += xmf3Offset.y;
		m_oobbAttackBox.Center.z += xmf3Offset.z;
	}

	if (m_fCoolTimeTracking < TRACKING_COOLTIME - TRACKING_DURATION)
	{
		m_bTracking = false;
	}

	if (m_fCoolTimeInterruption < INTERRUPTION_COOLTIME - INTERRUPTION_DURATION)
	{
		m_bInterruption = false;
	}

	if (m_bRunning && m_fCoolTimeRunning < ZOM_RUNNING_COOLTIME - ZOM_RUNNING_DURATION / 2.0f)
	{
		m_fMaxVelocityXZ -= fElapsedTime * 2;

		if (m_fCoolTimeRunning < ZOM_RUNNING_COOLTIME - ZOM_RUNNING_DURATION)
		{
			m_fMaxVelocityXZ = ZOMBIE_WALK_VELOCITY;
			m_bRunning = false;
		}
	}

	m_fCoolTimeTracking -= fElapsedTime;
	if (m_fCoolTimeTracking < 0.0f) m_fCoolTimeTracking = -1.0f;
	m_fCoolTimeInterruption -= fElapsedTime;
	if (m_fCoolTimeInterruption < 0.0f) m_fCoolTimeInterruption = -1.0f;
	m_fCoolTimeRunning -= fElapsedTime;
	if (m_fCoolTimeRunning < 0.0f) m_fCoolTimeRunning = -1.0f;

	CServerPlayer::Update(fElapsedTime, pCollisionManager);
}

void CServerZombiePlayer::UpdatePicking(INT8 nClientId)
{
	shared_ptr<CServerGameObject> pPickedObject = m_pPickedObject.lock();
	if (!pPickedObject)
	{
		return;
	}

	if (!(m_wKeyBuffer & KEY_E))
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

	if (!dynamic_pointer_cast<CServerItemObject>(pPickedObject))	// 아이템이 아님
	{
		pPickedObject->UpdatePicking(m_nPlayerId);
	}
}

void CServerZombiePlayer::Hit() 
{
}

void CServerZombiePlayer::CheckAttack(shared_ptr<CServerPlayer>& pPlayer, const BoundingSphere& aabbPlayer)
{
	if (m_oobbAttackBox.Intersects(aabbPlayer))
	{
		pPlayer->Hit();
	}
}

void CServerZombiePlayer::CollisionMine(int ref)
{
	SetStopMove(2.0f);
	m_bCollisionMine = true;
	m_fNoStopTime = 3.0f;
	m_iCollideMineRef = ref;
}

void CServerZombiePlayer::GameStartLogic()
{
	m_fGameStartWait = 10.0f;
	m_bGameStartWait = ZOMBIEMOVELIMIT;
}
