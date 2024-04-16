#pragma once
#include "Object.h"
#include "EnvironmentObject.h"

class CScene;
class CCamera;
class CRadarObject;

class CSoundCallbackHandler : public CAnimationCallbackHandler
{
public:
	CSoundCallbackHandler() { }
	~CSoundCallbackHandler() { }

public:
	virtual void HandleCallback(void* pCallbackData, float fTrackPosition);
};

class CPlayer : public CGameObject
{
public:
	CPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual ~CPlayer();

	virtual void LoadModelAndAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const shared_ptr<CLoadedModelInfo>& pLoadModelInfo) override {};

	virtual void Move(DWORD dwDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f) {};
	virtual void Rotate(float x, float y, float z);

	virtual void Update(float fElapsedTime);
	virtual void CalculateSpace();

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void OnUpdateToParent();
	virtual void Animate(float fElapsedTime);
	virtual void AnimateOOBB() override;
	virtual void Collide(float fElapsedTime, const shared_ptr<CGameObject>& pCollidedObject) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList) override;

	shared_ptr<CCamera> OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);
	virtual shared_ptr<CCamera> ChangeCamera(DWORD nNewCameraMode, float fElapsedTime);

	// Interface
	XMFLOAT3 GetPosition() const { return m_xmf3Position; }
	XMFLOAT3 GetOldPosition() const { return m_xmf3OldPosition; }
	XMFLOAT3 GetLookVector() const { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() const { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() const { return(m_xmf3Right); }

	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }

	void SetPosition(const XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }
	void SetScale(XMFLOAT3& xmf3Scale) { m_xmf3Scale = xmf3Scale; }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }

	virtual void OnPlayerUpdateCallback(float fElapsedTime) { }
	virtual void OnCameraUpdateCallback(float fElapsedTime) { }
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	shared_ptr<CCamera> GetCamera() { return m_pCamera; }
	void SetCamera(shared_ptr<CCamera> pCamera) { m_pCamera = pCamera; }

	// Picking
	void SetPickedObject(int nx, int ny, CScene* pScene);
	weak_ptr<CGameObject> GetPickedObject() { return m_pPickedObject; }
	virtual void UpdatePicking() override {};
	virtual void UseItem(int nSlot) {};

	int GetFloor() const { return m_nFloor; }
	int GetWidth() const { return m_nWidth; }
	int GetDepth() const { return m_nDepth; }

	void SetClientId(int nClientId) { m_nClientId = nClientId; }
	int GetClientId()const { return m_nClientId; }
	void SetLook(const XMFLOAT3& xmf3Look) { m_xmf3Look = xmf3Look; }
	void SetRight(const XMFLOAT3& xmf3Right) { m_xmf3Right = xmf3Right; }
protected:
	int m_nClientId = -1;

	bool m_bCollision = false;
	int m_nFloor = 0;
	int m_nWidth = 0;
	int m_nDepth = 0;

	XMFLOAT3					m_xmf3OldPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMFLOAT3					m_xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

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
	LPVOID						m_pCameraUpdatedContext = NULL;

	// 카메라 실체는 플레이어가 다룬다
	shared_ptr<CCamera> m_pCamera;
	weak_ptr<CGameObject> m_pPickedObject;
};

class CBlueSuitPlayer : public CPlayer
{
private:
	enum RightItem {
		NONE = 0,
		RAIDER,
	};
public:
	CBlueSuitPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual ~CBlueSuitPlayer();

	virtual void LoadModelAndAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const shared_ptr<CLoadedModelInfo>& pLoadModelInfo) override;
	virtual shared_ptr<CCamera> ChangeCamera(DWORD nNewCameraMode, float fElapsedTime);

	virtual void Rotate(float x, float y, float z);
	virtual void Move(DWORD dwDirection, float fDistance, bool bVelocity = false);
	virtual void Update(float fElapsedTime) override;
	virtual void Animate(float fElapsedTime);

	virtual void UpdatePicking() override;;
	virtual void RightClickProcess();

	int AddItem(const shared_ptr<CGameObject>& pGameObject);
	virtual void UseItem(int nSlot) override;
	void UseFuse();;
	void Teleport();

private:
	RightItem m_selectItem = RAIDER;

	std::array<weak_ptr<CGameObject>, 3> m_apSlotItems;

	int m_nFuseNum = 0;
	std::array<weak_ptr<CGameObject>, 3> m_apFuseItems;

	bool m_bShiftRun = false;
	bool m_bAbleRun = true;
	float m_fStamina = 5.0f;

private: 
	shared_ptr<CGameObject> m_pFlashlight; // 플래시라이트
	shared_ptr<CRadarObject> m_pRaider; // 레이더
public:
	XMFLOAT4X4* GetLeftHandItemFlashLightModelTransform() const;
	XMFLOAT4X4 GetRightHandItemRaiderModelTransform() const;

	XMFLOAT4X4* GetFlashLigthWorldTransform();
	void SetFlashLight(shared_ptr<CGameObject> object) { m_pFlashlight = object; }
	void SetRaider(shared_ptr<CGameObject> object) { m_pRaider->SetChild(object); }

	XMFLOAT4X4* RaiderUpdate(float fElapsedTime);

	bool PlayRaiderUI() { return m_fOpenRaiderTime == 0.0f && m_bRightClick; }
	float GetEscapeLength();
private:
	// 레이더 아이템
	XMFLOAT4X4 m_xmf4x4Raider;
	float m_fOpenRaiderTime;
	bool m_bRightClick = false;
};

class CZombiePlayer : public CPlayer
{
public:
	CZombiePlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual ~CZombiePlayer();

	virtual void LoadModelAndAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const shared_ptr<CLoadedModelInfo>& pLoadModelInfo) override;

	virtual void Update(float fElapsedTime) override;
private:

};