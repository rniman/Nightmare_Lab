#pragma once
#include "Object.h"
#include "EnvironmentObject.h"
#include "GlobalDefine.h"

class CMainScene;
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

	//virtual void Move(DWORD dwDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f) {};
	virtual void Rotate(float x, float y, float z);

	virtual void Update(float fElapsedTime);
	virtual void CalculateSpace();
	virtual void UpdateEnding(float fEndingElapsedTime, int nGameState) {};

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
	void SetPitch(float pitch) { m_fPitch = pitch; }
	float GetRoll() const { return(m_fRoll); }

	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }

	void SetWorldPostion(const XMFLOAT3& xmf3Position) { m_xmf3Position = xmf3Position; }
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
	void SetPickedObject(const shared_ptr<CGameObject> pGameObject);
	shared_ptr<CGameObject> GetPickedObject() { return m_pPickedObject; }
	virtual void UpdatePicking() override {};
	virtual void UseItem(int nSlot) {};

	int GetFloor() const { return m_nFloor; }
	int GetWidth() const { return m_nWidth; }
	int GetDepth() const { return m_nDepth; }

	void SetClientId(INT8 nClientId) { m_nClientId = nClientId; }
	INT8 GetClientId()const { return m_nClientId; }
	void SetLook(const XMFLOAT3& xmf3Look) { m_xmf3Look = xmf3Look; }
	void SetRight(const XMFLOAT3& xmf3Right) { m_xmf3Right = xmf3Right; }
	void SetUp(const XMFLOAT3& xmf3Up) { m_xmf3Up = xmf3Up; }

	virtual void RightClickProcess() {}
	bool IsRightClick() { return m_bRightClick; }
	void SetRightClick(bool val) { m_bRightClick = val; }

	virtual void SetTracking(bool bTracking) { m_bTracking = bTracking; }
	virtual void SetInterruption(bool bInterruption) { m_bInterruption = bInterruption; }
	virtual void SetRunning(bool bRunning) { m_bRunning = bRunning; }
	bool IsTracking()const { return m_bTracking; }
	bool IsInterruption() const { return m_bInterruption; }
	bool IsRunning() const { return m_bRunning; }

	void SetPlayerVolume(float fPlayerVolume);;
	float GetPlayerVolume()const { return m_fPlayerVolume; }
protected:
	INT8 m_nClientId = -1;

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
	shared_ptr<CGameObject> m_pPickedObject;

	bool m_bRightClick = false;

	bool m_bTracking = false;
	float m_fTrackingTime = 0.0f;
	bool m_bInterruption = false; // true시 안개 효과 심해짐
	float m_fInterruptionTime = 0.0f;
	float m_fInterruption = 0.0f;
	bool m_bRunning = false;
	float m_fRunningTime = 0.0f;

	bool m_ShadowRender;
	bool m_SelfShadowRender;

	float m_fPlayerVolume = 0.0f;
public:
	void SetShadowRender(bool val) { m_ShadowRender = val; }
	void SetSelfShadowRender(bool val) { m_SelfShadowRender = val; }

};

constexpr float BLUESUIT_STAMINA_MAX{ 5.0f };
constexpr float BLUESUIT_STAMINA_EXHAUSTION{ 3.0f };

class CBlueSuitPlayer : public CPlayer
{
public:
	CBlueSuitPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual ~CBlueSuitPlayer();

	virtual void LoadModelAndAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const shared_ptr<CLoadedModelInfo>& pLoadModelInfo) override;
	virtual shared_ptr<CCamera> ChangeCamera(DWORD nNewCameraMode, float fElapsedTime);

	virtual void Rotate(float x, float y, float z);
	//virtual void Move(DWORD dwDirection, float fDistance, bool bVelocity = false);
	virtual void Update(float fElapsedTime) override;
	void UpdateAnimation();
	virtual void UpdateEnding(float fEndingElapsedTime, int nGameState) override;

	virtual void Animate(float fElapsedTime);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void MainPlayerRender(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void UpdatePicking() override;
	virtual void RightClickProcess();

	int AddItem(const shared_ptr<CGameObject>& pGameObject);
	virtual void UseItem(int nSlot) override;
	void UseFuse();;
	void Teleport();

	bool IsSlotItemObtain(int nIndex) { return m_apSlotItems[nIndex]->IsObtained(); }
	bool IsFuseObtain(int nIndex) { return m_apFuseItems[nIndex]->IsObtained(); }

	void SetSlotItem(int nIndex, int nReferenceObjectNum);
	void SetSlotItemEmpty(int nIndex);
	void SetFuseItem(int nIndex, int nReferenceObjectNum);
	void SetFuseItemEmpty(int nIndex);

	int GetReferenceSlotItemNum(int nIndex) { return m_apSlotItems[nIndex]->GetReferenceNumber(); }
	int GetReferenceFuseItemNum(int nIndex) { return m_apFuseItems[nIndex]->GetReferenceNumber(); }

	void SelectItem(RightItem item) { m_selectItem = item; }
	RightItem GetSelectItem() const { return m_selectItem; }
	void AddEnvironmentMineItems(shared_ptr<CMineObject> object);
	void UseMine(int item_id);

	float GetStamina() const { return m_fStamina; }
	float GetFullStaminaTime() const { return m_fFullStaminaTime; }

	void SetEscapePos(XMFLOAT3 pos);
private:
	RightItem m_selectItem = NONE;

	std::array<shared_ptr<CItemObject>, 3> m_apSlotItems;

	int m_nFuseNum = 0;
	std::array<shared_ptr<CItemObject>, 3> m_apFuseItems;

	float m_fStamina = BLUESUIT_STAMINA_MAX;
	float m_fFullStaminaTime = 0.0f;

	XMFLOAT3 m_fEscapePos = XMFLOAT3(0.0f, 0.0f, 0.0f);
private: 
	shared_ptr<CGameObject> m_pFlashlight; // 플래시라이트
	shared_ptr<CRadarObject> m_pRader; // 레이더
	shared_ptr<CTeleportObject> m_pTeleport; // 텔레포트아이템
	shared_ptr<CMineObject> m_pMine; // 텔레포트아이템
	shared_ptr<CFuseObject> m_pFuse;// 퓨즈 아이템

	vector<shared_ptr<CMineObject>> m_vpEnvironmentMineItems; // 마인 아이템을 담은 컨테이너
public:
	XMFLOAT4X4* GetLeftHandItemFlashLightModelTransform() const;
	XMFLOAT4X4 GetRightHandItemRaderModelTransform() const;
	XMFLOAT4X4* GetRightHandItemTeleportItemModelTransform() const;

	XMFLOAT4X4* GetFlashLigthWorldTransform();
	void SetFlashLight(shared_ptr<CGameObject> object) { m_pFlashlight = object; }
	void SetRader(shared_ptr<CRadarObject> object) { m_pRader = object; }
	void SetTeleportItem(shared_ptr<CTeleportObject> object) { m_pTeleport = object; }
	void SetMineItem(shared_ptr<CMineObject> object) { m_pMine = object; }
	void SetFuseItem(shared_ptr<CFuseObject> object) { m_pFuse = object; }

	XMFLOAT4X4* RaderUpdate(float fElapsedTime);
	bool PlayRaiderUI() { return m_fOpenRaderTime == 0.0f && m_bRightClick && m_selectItem == RADAR; }
	float GetEscapeLength();
private:
	// 레이더 아이템 행렬
	XMFLOAT4X4 m_xmf4x4Rader;
	float m_fOpenRaderTime;
	bool m_bRightClick = false;
	
	//피격 텍스쳐링을 위한 재질
	shared_ptr<CMaterial> m_pHitEffectMaterial;
	FrameTimeInfo* m_pcbMappedTime;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dTimeCbvGPUDescriptorHandle;
	ComPtr<ID3D12Resource>			m_pd3dcbTime;

	FrameTimeInfo* m_pcbMappedTimeEnd;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dTimeCbvGPUDescriptorHandleEnd;
	ComPtr<ID3D12Resource>			m_pd3dcbTimeEnd;
	bool m_bHitEffectBlend = false;

public:
	void SetHitEvent();
private:
	float m_fStopMoving = 0.0f;
	int m_iMineobjectNum = -1;
};

struct FrameTimeInfo;

constexpr float TRACKING_DURATION{ 5.0f };
constexpr float TRACKING_COOLTIME{ 10.0f };
constexpr float INTERRUPTION_DURATION{ 5.0f };
constexpr float INTERRUPTION_COOLTIME{ 10.0f };
constexpr float ZOM_RUNNING_DURATION{ 6.0f };
constexpr float ZOM_RUNNING_COOLTIME{ 10.0f };

class CZombiePlayer : public CPlayer
{
private:
	FrameTimeInfo* m_pcbMappedTime;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dTimeCbvGPUDescriptorHandle;
	ComPtr<ID3D12Resource>			m_pd3dcbTime;

	FrameTimeInfo* m_pcbMappedTimeEnd;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dTimeCbvGPUDescriptorHandleEnd;
	ComPtr<ID3D12Resource>			m_pd3dcbTimeEnd;
public:
	CZombiePlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual ~CZombiePlayer();

	virtual void LoadModelAndAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const shared_ptr<CLoadedModelInfo>& pLoadModelInfo) override;

	virtual void Update(float fElapsedTime) override;
	void UpdateAnimation();
	virtual void UpdateEnding(float fEndingElapsedTime, int nGameState) override;

	virtual void Animate(float fElapsedTime);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);

	void UpdateSkill(float fElapsedTime);

	virtual void SetTracking(bool bTracking) override;
	virtual void SetInterruption(bool bInterruption) override;
	virtual void SetRunning(bool bRunning) override;
	bool IsAbleTracking() const { return m_bAbleTracking; }
	bool IsAbleInterruption() const { return m_bAbleInterruption; }
	bool IsAbleRunning() const { return m_bAbleRunning; }
private:
	bool m_bElectricBlend = false;
	shared_ptr<CMaterial> m_pElectircaterial;

	bool m_bAbleTracking = true;
	bool m_bAbleInterruption = true;
	bool m_bAbleRunning = true;

	shared_ptr<CGameObject> m_pBodyObject;
	shared_ptr<CGameObject> m_pEyesObject;
public:
	void SetEectricShock();
	void SetElectiricMt(shared_ptr<CMaterial> mt) { m_pElectircaterial = mt; }

	virtual shared_ptr<CCamera> ChangeCamera(DWORD nNewCameraMode, float fElapsedTime);
};