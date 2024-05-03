#pragma once
#include "Object.h"
#include "TextureBlendObject.h"

class CItemObject : public CGameObject
{
public:
	CItemObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CItemObject() {};

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList) override;;
	virtual void Animate(float fElapsedTime) override;

	void SetObtain(bool bObtained) { m_bObtained = bObtained; }
	void SetReferenceNumber(int nObjectNumber) { m_nReferenceNumber = nObjectNumber; }

	bool IsObtained() const { return m_bObtained; }
	int GetReferenceNumber() const { return m_nReferenceNumber; }
protected:
	bool m_bObtained = false;

	int m_nReferenceNumber = -1;
};

/// <CGameObject - CItemObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CEnvironmentObject>

class CEnvironmentObject :public CGameObject
{
public:
	CEnvironmentObject(char* pstrFrameName, XMFLOAT4X4& xmf4x4World, CMesh* pMesh);
	virtual ~CEnvironmentObject() {};

private:

};

/// <CGameObject - CEnvironmentObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CDoorObject>

class CDrawerObject : public CGameObject
{
public:
	CDrawerObject(char* pstrFrameName, XMFLOAT4X4& xmf4x4World, CMesh* pMesh);
	virtual ~CDrawerObject();

	virtual void Animate(float fElapsedTime) override;
	virtual void UpdatePicking() override;
private:
	bool m_bOpened = false;
	bool m_bAnimate = false;
	XMFLOAT3 m_xmf3OriginPosition;
	XMFLOAT3 m_xmf3Forward;	
};

/// <CGameObject - CDrawerObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CDoorObject>

class CDoorObject : public CGameObject
{
public:
	CDoorObject(char* pstrFrameName, XMFLOAT4X4& xmf4x4World, CMesh* pMesh);
	virtual ~CDoorObject();

	virtual void Animate(float fElapsedTime) override;
	virtual void UpdatePicking() override;

private:
	bool m_bOpened = false;
	float m_fDoorAngle = 0.0f;
	float m_fRotationAngle = 0.0f;

	XMFLOAT4 m_xmf4Quaternion = { 0.0f, 0.0f, 0.0f, 1.0f };
};

/// <CGameObject - CDrawerObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CElevatorDoorObject>

class CElevatorDoorObject : public CGameObject
{
public:
	CElevatorDoorObject(char* pstrFrameName, XMFLOAT4X4& xmf4x4World, CMesh* pMesh);
	virtual ~CElevatorDoorObject() {};

	virtual void Animate(float fElapsedTime) override;
	virtual void UpdatePicking() override;
private:
	bool m_bOpened = false;
	bool m_bAnimate = false;
	XMFLOAT3 m_xmf3OriginPosition;
	XMFLOAT3 m_xmf3Right;
};

/// <CGameObject - CElevatorDoorObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CItemObject - CTeleportObject>

class CTeleportObject : public CItemObject
{
public:
	CTeleportObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CTeleportObject();;

	virtual void LoadModelAndAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const shared_ptr<CLoadedModelInfo>& pLoadModelInfo) override;

	virtual void Animate(float fElapsedTime) override;
	virtual void UpdatePicking() override;
	virtual void UpdateUsing(const shared_ptr<CGameObject>& pGameObject) override;
};

/// <CGameObject - CTeleportObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CItemObject - CMineObject>

class CMineObject : public CItemObject
{
public:
	
	CMineObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CMineObject();

	virtual void LoadModelAndAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const shared_ptr<CLoadedModelInfo>& pLoadModelInfo) override;

	virtual void Animate(float fElapsedTime) override;
	virtual void UpdatePicking() override;
	virtual void UpdateUsing(const shared_ptr<CGameObject>& pGameObject) override;

	void SetInstall(bool val) { m_bInstall = val; }
	bool IsInstall() { return m_bInstall; }
private:
	bool m_bInstall = false;
	bool m_bCollide = false;

	shared_ptr<TextureBlendObject> m_pExplosionObject;
public:
	void SetExplosionObject(shared_ptr<TextureBlendObject> object) { m_pExplosionObject = object; }
	void CollideZombie();
};

/// <CGameObject - CMineObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CItemObject - CFuseObject>

class CFuseObject : public CItemObject
{
public:
	CFuseObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CFuseObject();

	virtual void LoadModelAndAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const shared_ptr<CLoadedModelInfo>& pLoadModelInfo) override;

	virtual void Animate(float fElapsedTime) override;
	virtual void UpdatePicking() override;
	virtual void UpdateUsing(const shared_ptr<CGameObject>& pGameObject) override;
private:

};

/// <CGameObject - CFuseObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CItemObject - CRadarObject>

class CRadarObject : public CItemObject
{
public:
	CRadarObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CRadarObject();

	virtual void LoadModelAndAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const shared_ptr<CLoadedModelInfo>& pLoadModelInfo) override;

	virtual void Animate(float fElapsedTime) override;
	virtual void UpdatePicking() override;
	virtual void UpdateUsing(const shared_ptr<CGameObject>& pGameObject) override;
};