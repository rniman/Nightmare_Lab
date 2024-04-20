#pragma once
#include "ServerObject.h"

class CItemObject : public CServerGameObject
{
public:
	virtual ~CItemObject() {};

	virtual void UpdateUsing(const shared_ptr<CServerGameObject>& pGameObject) {};

	bool GetObtained() const { return m_bObtained; }
protected:
	bool m_bObtained = false;
};

/// <CGameObject - CItemObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CEnvironmentObject>

class CEnvironmentObject :public CServerGameObject
{
public:
	CEnvironmentObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb);
	virtual ~CEnvironmentObject() {};

private:

};

/// <CGameObject - CEnvironmentObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CDoorObject>

class CDrawerObject : public CServerGameObject
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

class CDoorObject : public CServerGameObject
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

class CElevatorDoorObject : public CServerGameObject
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
	virtual void UpdateUsing(const shared_ptr<CServerGameObject>& pGameObject) override;
};

/// <CGameObject - CTeleportObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CItemObject - CMineObject>

class CMineObject : public CItemObject
{
public:
	virtual ~CMineObject();

	virtual void UpdatePicking() override;
	virtual void UpdateUsing(const shared_ptr<CServerGameObject>& pGameObject) override;
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
	virtual void UpdateUsing(const shared_ptr<CServerGameObject>& pGameObject) override;

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
	virtual void UpdateUsing(const shared_ptr<CServerGameObject>& pGameObject) override;
};

/// <CGameObject - CRadarObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CStairObject>

class CStairTriggerObject : public CServerGameObject
{
public:
	CStairTriggerObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb);

	float GetOffsetY() const { return m_fOffsetY; }
	float GetY() const { return m_fy; }
	XMFLOAT4 GetStairPlane() const { return m_xmf4Plane; }
private:
	float m_fx;
	float m_fy;
	float m_fz;
	float m_fOffsetY;

	XMFLOAT4 m_xmf4Plane;
};