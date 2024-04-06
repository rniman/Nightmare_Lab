#pragma once
#include "ServerObject.h"

class CItemObject : public CGameObject
{
public:
	virtual ~CItemObject() {};

	virtual void UpdateUsing(const shared_ptr<CGameObject>& pGameObject) {};

	bool GetObtained() const { return m_bObtained; }
protected:
	bool m_bObtained = false;
};

/// <CGameObject - CItemObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CEnviromentObejct>

class CEnviromentObejct :public CGameObject
{
public:
	CEnviromentObejct(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb);
	virtual ~CEnviromentObejct() {};

private:

};

/// <CGameObject - CEnviromentObejct>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CDoorObject>

class CDrawerObject : public CGameObject
{
public:
	CDrawerObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb);
	virtual ~CDrawerObject();

	virtual void Update(float fElapsedTime) override;
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
	CDoorObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb);
	virtual ~CDoorObject();

	virtual void Update(float fElapsedTime) override;
	virtual void UpdatePicking() override;

private:
	bool m_bOpened = false;
	float m_fDoorAngle = 0.0f;
	float m_fRotationAngle = 0.0f;

};

/// <CGameObject - CDrawerObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CElevatorDoorObject>

class CElevatorDoorObject : public CGameObject
{
public:
	CElevatorDoorObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb);
	virtual ~CElevatorDoorObject() {};

	virtual void Update(float fElapsedTime) override;
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
	virtual ~CTeleportObject();;

	virtual void UpdatePicking() override;
	virtual void UpdateUsing(const shared_ptr<CGameObject>& pGameObject) override;
};

/// <CGameObject - CTeleportObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CItemObject - CMineObject>

class CMineObject : public CItemObject
{
public:
	virtual ~CMineObject();

	virtual void UpdatePicking() override;
	virtual void UpdateUsing(const shared_ptr<CGameObject>& pGameObject) override;
private:

};

/// <CGameObject - CMineObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CItemObject - CFuseObject>

class CFuseObject : public CItemObject
{
public:
	virtual ~CFuseObject();

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
	virtual ~CRadarObject();

	virtual void UpdatePicking() override;
	virtual void UpdateUsing(const shared_ptr<CGameObject>& pGameObject) override;
};