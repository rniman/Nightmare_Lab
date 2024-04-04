#include "stdafx.h"
#include "Player.h"

CPlayer::CPlayer()
{
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
		//CalculateSpace();
		//m_pCamera->Move(xmf3Shift);
	}
}