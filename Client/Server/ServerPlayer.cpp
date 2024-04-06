#include "stdafx.h"
#include "ServerPlayer.h"
#include "Collision.h"

CPlayer::CPlayer()
	:CGameObject()
{
	XMFLOAT3 xmf3Center = XMFLOAT3(0.0f, 0.8f, 0.0f);
	XMFLOAT3 xmf3Extents = XMFLOAT3(0.4f, 0.8f, 0.4f);
	XMFLOAT4 xmf4Orientation;
	XMStoreFloat4(&xmf4Orientation, XMQuaternionIdentity());

	m_voobbOrigin.emplace_back(xmf3Center, xmf3Extents, xmf4Orientation);

	SetFriction(250.0f);
	SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
	SetMaxVelocityXZ(8.0f);
	SetMaxVelocityY(40.0f);
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
	//if (m_pKeysBuffer[VK_PRIOR] & 0xF0) dwDirection |= DIR_UP;
	//if (m_pKeysBuffer[VK_NEXT] & 0xF0) dwDirection |= DIR_DOWN;

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


	//for (int i = 7; i >= 0; --i)
	//{
	//	int result = dwDirection >> i & 1;
	//	printf("%d", result);
	//}
	//printf("  Vel: %f, %f", m_xmf3Position.x, m_xmf3Position.z);
	//printf("\n");
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
		//m_pCamera->Move(Vector3::ScalarProduct(xmf3SubVelocity[k], -1.0f, false));

		OnUpdateToParent();
		aabbPlayer.Center = m_voobbOrigin[0].Center;
		aabbPlayer.Extents = m_voobbOrigin[0].Extents;
		XMVECTOR xmvTranslation = XMVectorSet(m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z, 1.0f);
		aabbPlayer.Transform(aabbPlayer, 1.0f, XMQuaternionIdentity(), xmvTranslation);

		for (int i = m_nWidth - 1; i <= m_nWidth + 1 && !m_bCollision; ++i)
		{
			for (int j = m_nDepth - 1; j <= m_nDepth + 1 && !m_bCollision; ++j)
			{
				if (i < 0 || i >= pCollisionManager->GetWidth() || j < 0 || j >= pCollisionManager->GetDepth())
				{
					continue;
				}

				for (const auto& pGameObject : pCollisionManager->GetSpaceGameObjects(m_nFloor, i, j))
				{
					if (!pGameObject || pGameObject->GetCollisionType() == 2)	//임시로 2면 넘김
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

	//DWORD nCurrentCameraMode = m_pCamera->GetMode();
	//m_pCamera->Update(m_xmf3Position, fElapsedTime);
	//m_pCamera->RegenerateViewMatrix();

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
		//m_pCamera->Move(xmf3Shift);
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