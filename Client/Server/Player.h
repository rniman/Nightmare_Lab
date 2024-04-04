#pragma once
#include "Object.h"

constexpr WORD DIR_FORWARD = 0x01;
constexpr WORD DIR_BACKWARD = 0x02;
constexpr WORD DIR_LEFT = 0x04;
constexpr WORD DIR_RIGHT = 0x08;
constexpr WORD PRESS_LBUTTON = 0x10;
constexpr WORD PRESS_RBUTTON = 0x20;
constexpr WORD DIR_UP = 0x40;
constexpr WORD DIR_DOWN = 0x80;

class CPlayer : public CGameObject
{
public:
	CPlayer();
	virtual ~CPlayer() {};

	virtual void Update(float fElapsedTime) override;
	void Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity);

	// Interface
	void SetPlayerId(int nPlayerId) { m_nPlayerId = nPlayerId; }

	void SetLook(const XMFLOAT3& xmf3Look) { m_xmf3Look = xmf3Look; }
	void SetRight(const XMFLOAT3& xmf3Right) { m_xmf3Right = xmf3Right; }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }

	UCHAR* GetKeysBuffer() { return m_pKeysBuffer; }
	int GetPlayerId() const { return m_nPlayerId; }
	XMFLOAT3 GetPosition() const { return m_xmf3Position; }
	XMFLOAT3 GetVelocity() const { return m_xmf3Velocity; }
	XMFLOAT3 GetLook() const { return m_xmf3Look; }
	XMFLOAT3 GetRight() const { return m_xmf3Right; }
protected:
	// 통시을 통해 받는 정보
	int m_nPlayerId = -1;
	UCHAR m_pKeysBuffer[256];

	bool m_bCollision = false;
	int m_nFloor = 0;
	int m_nWidth = 0;
	int m_nDepth = 0;

	XMFLOAT3					m_xmf3OldPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	float           			m_fPitch = 0.0f;
	float           			m_fYaw = 0.0f;
	float           			m_fRoll = 0.0f;

	XMFLOAT3					m_xmf3OldVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);	// 이동시 적용된 속력 저장
	XMFLOAT3					m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3     				m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float           			m_fMaxVelocityXZ = 0.0f;
	float           			m_fMaxVelocityY = 0.0f;
	float           			m_fFriction = 0.0f;

	LPVOID						m_pPlayerUpdatedContext = NULL;
};

///
///
/// 
