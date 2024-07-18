#pragma once
#include "ServerObject.h"

constexpr WORD PRESS_LBUTTON = 0x10;
constexpr WORD PRESS_RBUTTON = 0x20;
constexpr FLOAT ASPECT_RATIO = 1600.0f / 1024.0f;

constexpr float BLUESUIT_WALK_VELCOCITY{ 6.0f };
constexpr float ZOMBIE_WALK_VELOCITY{ 7.0f };
constexpr float BLUESUIT_STAMINA_MAX{ 5.0f };
constexpr float BLUESUIT_STAMINA_EXHAUSTION{ 3.0f };

constexpr float TRACKING_DURATION{ 5.0f };
constexpr float TRACKING_COOLTIME{ 10.0f };
constexpr float INTERRUPTION_DURATION{ 5.0f };
constexpr float INTERRUPTION_COOLTIME{ 10.0f };
constexpr float ZOM_RUNNING_DURATION{ 6.0f };
constexpr float ZOM_RUNNING_COOLTIME{ 10.0f };

class CServerPlayer : public CServerGameObject
{
public:
	CServerPlayer();
	virtual ~CServerPlayer() {};

	virtual void UseItem(shared_ptr<CServerCollisionManager>& pCollisionManager) {};
	virtual void Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager) override;
	void Declare(float fElapsedTime);
	virtual void Collide(const shared_ptr<CServerCollisionManager>& pCollisionManager, float fElapsedTime, shared_ptr<CServerGameObject> pCollided) override;
	void CheckStairTrigger(const std::shared_ptr<CServerGameObject>& pGameObject, DirectX::BoundingSphere& aabbPlayer);
	void CollideWithPlayer(const shared_ptr<CServerCollisionManager>& pCollisionManager, float fElapsedTime, shared_ptr<CServerPlayer> pCollidedPlayer);
	virtual void Hit() {};	//BlueSuit: zombie Attack, Zombie: mine
	void Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity);

	virtual void UpdatePicking(INT8 nClientId) override {};
	//virtual void UseItem(int nSlot) {};

	void CalculateSpace();
	void OnUpdateToParent();
	// Interface
	void SetRecvData(bool bRecvData) { m_bRecvData = bRecvData; }

	void SetKeyBuffer(WORD wKeyBuffer) { m_wKeyBuffer = wKeyBuffer; }
	void SetPlayerId(INT8 nPlayerId) { m_nPlayerId = nPlayerId; }

	void SetStair(bool bStair) { m_bStair = bStair; }
	void SetStairY(float fMax, float fMin) { m_fStairMax = fMax; m_fStairMin = fMin; }
	void SetStairPlane(const XMFLOAT4& xmf4StairPlane) { m_xmf4StairPlane = xmf4StairPlane; }

	void SetPlayerPosition(const XMFLOAT3& xmf3Position) { m_xmf3Position = xmf3Position; }
	void SetPlayerOldPosition(const XMFLOAT3& xmf3OldPosition) { m_xmf3OldPosition = xmf3OldPosition; }

	void SetLook(const XMFLOAT3& xmf3Look) { m_xmf3Look = xmf3Look; }
	void SetRight(const XMFLOAT3& xmf3Right) { m_xmf3Right = xmf3Right; }
	void SetUp(const XMFLOAT3& xmf3Up) { m_xmf3Up = xmf3Up; }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }

	void SetPickedObject(const shared_ptr<CServerCollisionManager> pCollisionManager);
	void SetViewMatrix(const XMFLOAT4X4& xmf4x4View) { m_xmf4x4View = xmf4x4View; }

	bool IsRecvData() const { return m_bRecvData; }
	//UCHAR* GetKeysBuffer() { return m_pKeysBuffer; }
	WORD GetKeysBuffer() const { return m_wKeyBuffer; }
	INT8 GetPlayerId() const { return m_nPlayerId; }

	bool IsStair()const { return m_bStair; }
	float GetStairMax() const { return m_fStairMax; }
	float GetStairMin() const { return m_fStairMin; }
	XMFLOAT4 GetStairPlane() { return m_xmf4StairPlane; }

	XMFLOAT3 GetPosition() const { return m_xmf3Position; }
	XMFLOAT3 GetVelocity() const { return m_xmf3Velocity; }
	XMFLOAT3 GetLook() const { return m_xmf3Look; }
	XMFLOAT3 GetRight() const { return m_xmf3Right; }

	void SetRunning(bool bRunning) { m_bRunning = bRunning; }
	bool IsRunning() const { return m_bRunning; }
	weak_ptr<CServerGameObject> GetPickedObject() { return m_pPickedObject; }

	bool IsInvincibility() const { return m_bInvincibility; }

	virtual void RightClickProcess(shared_ptr<CServerCollisionManager>& pCollisionManager) {}
	void SetRightClick(bool val) { m_bRightClick = val; }

	//void SetWinner(bool bWinner) { m_bWinner = bWinner; }
	bool IsWinner() const { return m_bWinner; }
protected:
	// 첫 데이터를 받기 시작
	bool m_bRecvData = false;

	// 통신을 통해 받는 정보
	INT8 m_nPlayerId = -1;	//m_vSocketInfoList인덱스 번호
	
	//[0511] 이제 WORD로 다룬다
	WORD m_wKeyBuffer = 0;

	bool m_bOccurredCollision = false;
	bool m_bStair = false;		// 계단에 있는 상태

	float m_fStairMax;
	float m_fStairMin;
	XMFLOAT4 m_xmf4StairPlane;

	XMFLOAT3					m_xmf3OldPosition;
	XMFLOAT3					m_xmf3Position;
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMFLOAT3					m_xmf3OldVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);	// 이동시 적용된 속력 저장
	XMFLOAT3					m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3     				m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float           			m_fMaxVelocityXZ = 0.0f;
	float           			m_fMaxVelocityY = 0.0f;
	float           			m_fFriction = 0.0f;

	bool m_bRunning = false;

	weak_ptr<CServerGameObject>	m_pPickedObject;
	bool						m_bPressed = false;

	XMFLOAT4X4					m_xmf4x4View;
	XMFLOAT4X4					m_xmf4x4Projection;

	bool						m_bInvincibility = false;	// HIT가 가능한 상태인지를 나타냄
	float						m_fCoolTimeInvincibility = 0.0f;
	float						m_fDeathTime = 0.0f;

	bool m_bRightClick = false;
	bool m_bWinner = false;;
};

///
///
/// 

enum ITEM_SLOT
{
	Teleport,
	Radar,
	Mine,
	Fuse
};

class CServerItemObject;
class CServerFuseObject;

class CServerBlueSuitPlayer : public CServerPlayer
{
public:
	CServerBlueSuitPlayer();
	virtual ~CServerBlueSuitPlayer() {};

	virtual void UseItem(shared_ptr<CServerCollisionManager>& pCollisionManager);
	virtual void Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager) override;
	virtual void UpdatePicking(INT8 nClientId) override;
	virtual void Hit() override;

	int AddItem(const shared_ptr<CServerGameObject>& pGameObject);
	//virtual void UseItem(int nSlot) override;
	void UseFuse(shared_ptr<CServerCollisionManager>& pCollisionManager);
	void TeleportRandomPosition();

	int GetReferenceSlotItemNum(int nIndex);
	int GetReferenceFuseItemNum(int nIndex);

	RightItem GetRightItem() { return m_selectItem; }
	virtual void RightClickProcess(shared_ptr<CServerCollisionManager>& pCollisionManager);

	bool IsAttacked() { return m_fCoolTimeInvincibility > 0.0f; }
private:
	RightItem m_selectItem = NONE;

	std::array<shared_ptr<CServerItemObject>, 3> m_apSlotItems;

	int m_nFuseNum = 0;
	std::array<shared_ptr<CServerFuseObject>, 3> m_apFuseItems;

	bool m_bAbleRun = true;
	float m_fStamina = 5.0f;

	int m_nHealthPoint = 3;
};

///
///
/// 

class CServerZombiePlayer : public CServerPlayer
{
public:
	CServerZombiePlayer();
	virtual ~CServerZombiePlayer() {};

	virtual void UseItem(shared_ptr<CServerCollisionManager>& pCollisionManager) override;;
	virtual void Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager) override;
	virtual void UpdatePicking(INT8 nClientId) override;
	virtual void Hit() override;;
	void CheckAttack(shared_ptr<CServerPlayer>& pPlayer, const BoundingSphere& aabbPlayer);

	bool IsTracking() const { return m_bTracking; }
	bool IsInterruption() const { return m_bInterruption; }
	bool IsAttack() const { return m_bAttack; }

private:
	BoundingOrientedBox m_oobbAttackBox;

	float m_fCoolTimeTracking = -1.0f;
	float m_fCoolTimeInterruption = -1.0f;
	float m_fCoolTimeRunning = -1.0f;
	float m_fCoolTimeAttack = -1.0f;

	bool m_bTracking = false;
	bool m_bInterruption = false;
	bool m_bAttack = false;	// true면 클라에서 track을 enable시킴

	// 좀비 지뢰 관련
	bool m_bCollisionMine = false;
	float m_fStopMove = 0.0f;
	float m_fNoStopTime = 0.0f;
	int m_iCollideMineRef = -1;
public:
	bool GetCollisionMine() { return m_bCollisionMine; }
	float GetNoStopTime() { return m_fNoStopTime; }
	int GetCollideMineRef() { return m_iCollideMineRef; }

	void SetCollideMineRef(int val) { m_iCollideMineRef = val; }
	void SetStopMove(float val) { m_fStopMove = val; }
	void CollisionMine(int ref);

	float expDelay = 0.0f;
};