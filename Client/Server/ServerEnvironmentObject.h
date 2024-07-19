#pragma once
#include "ServerObject.h"

class CServerDrawerObject;
class CServerCollisionManager;

class CServerItemObject : public CServerGameObject
{
public:
	CServerItemObject();
	virtual ~CServerItemObject() {};

	virtual void Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager);
	virtual void UpdateUsing(const shared_ptr<CServerGameObject>& pGameObject, shared_ptr<CServerCollisionManager>& pCollisionManager) {};

	static void SetDrawerIdContainer(vector<pair<int, int>> vDrawerId);

	void SetObtain(bool bObtained) { m_bObtained = bObtained; }
	void SetDrawerNumber(int nDrawerNumber) { m_nDrawerNumber = nDrawerNumber; }
	void SetDrawer(const shared_ptr<CServerDrawerObject>& pDrawerObject) { m_pDrawerObject = pDrawerObject; }
	void SetDrawerType(int nDrawerNumber) { m_nDrawerType = nDrawerNumber; }
	//void SetReferencePlayerId(int nPlayerId) { m_nReferencePlayerId = nPlayerId; }
	void SetReferenceNumber(int nObjectNumber) { m_nReferenceNumber = nObjectNumber; }

	void SetRandomRotation(const XMFLOAT3& xmf3Rotation) { m_xmf3Rotation = xmf3Rotation; }
	virtual void SetRandomPosition(shared_ptr<CServerCollisionManager>& pCollisionManager) {};	// DrawerNumber를 다시 할당
	virtual void SetWorldMatrix(const XMFLOAT4X4& xmf4x4World);

	void SetRandomOffset(const XMFLOAT3& xmf3Offset);

	bool IsObtained() const { return m_bObtained; }
	int GetDrawerNumber() const { return m_nReferenceNumber; }
	int GetReferenceNumber() const { return m_nReferenceNumber; }
protected:
	/*static int m_nStartDrawer1;
	static int m_nEndDrawer1;
	static int m_nStartDrawer2;
	static int m_nEndDrawer2;*/
	static vector<pair<int, int>> m_vDrawerId; // <ObjectCount,type>

	// Drawer1 2에 따라 다르게 offset을 준다
	static float m_fDrawer1OffsetY;
	static float m_fDrawer2OffsetY;

	XMFLOAT3 m_xmf3Rotation;
	XMFLOAT3 m_xmf3PositionOffset; // Drawer의 world 위치와의 차이

	bool m_bObtained = false;
	int m_nDrawerNumber = -1;		// CollisionManager에 있는 Drawer중 자기가 속한 인덱스 값
	int m_nDrawerType = -1;
	shared_ptr<CServerDrawerObject> m_pDrawerObject;

	int m_nReferenceNumber = -1;	// 플레이어가 가진 아이템 오브젝트가 현재 어떤 오브젝트를 가졌는지(플레이어 내부 오브젝트 전용)
	// 사용시 다시 -1이 되야함
};

/// <CGameObject - CItemObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CEnvironmentObject>

class CServerEnvironmentObject :public CServerGameObject
{
public:
	CServerEnvironmentObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb);
	virtual ~CServerEnvironmentObject() {};

private:

};

/// <CGameObject - CEnvironmentObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CDoorObject>

class CServerDrawerObject : public CServerGameObject
{
public:
	CServerDrawerObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb);
	virtual ~CServerDrawerObject();

	virtual void Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager) override;
	virtual void UpdatePicking(INT8 nClientId) override;

	bool IsOpen() const { return m_bOpened; }

	shared_ptr<CServerItemObject> m_pStoredItem;
private:
	bool m_bOpened = false;
	bool m_bAnimate = false;
	XMFLOAT3 m_xmf3OriginPosition;
	XMFLOAT3 m_xmf3Forward;
};

/// <CGameObject - CDrawerObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CDoorObject>

class CServerDoorObject : public CServerGameObject
{
public:
	CServerDoorObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb);
	virtual ~CServerDoorObject();

	virtual void Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager) override;
	virtual void UpdatePicking(INT8 nClientId) override;

private:
	bool m_bOpened = false;
	float m_fDoorAngle = 0.0f;
	float m_fRotationAngle = 0.0f;

};

/// <CGameObject - CDrawerObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CElevatorDoorObject>

class CServerElevatorDoorObject : public CServerGameObject
{
public:
	CServerElevatorDoorObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb);
	virtual ~CServerElevatorDoorObject() {};

	virtual void Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager) override;
	virtual void UpdatePicking(INT8 nClientId) override;
	void EscapeDoorOpen(INT8 nClientId);

	void SetEscapeDoor(bool val) { m_bEscapeDoor = val; }
	bool IsEscape() { return m_bEscapeDoor; }
private:
	bool m_bOpened = false;
	bool m_bAnimate = false;
	XMFLOAT3 m_xmf3OriginPosition;
	XMFLOAT3 m_xmf3Right;

	bool m_bEscapeDoor = false;
};

/// <CGameObject - CElevatorDoorObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CItemObject - CTeleportObject>

class CServerTeleportObject : public CServerItemObject
{
public:
	CServerTeleportObject();
	virtual ~CServerTeleportObject();

	virtual void Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager) override;
	virtual void UpdatePicking(INT8 nClientId) override;
	virtual void UpdateUsing(const shared_ptr<CServerGameObject>& pGameObject, shared_ptr<CServerCollisionManager>& pCollisionManager) override;

	virtual void SetRandomPosition(shared_ptr<CServerCollisionManager>& pCollisionManager) override;

	virtual void SetWorldMatrix(const XMFLOAT4X4& xmf4x4World) override;
};

/// <CGameObject - CTeleportObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CItemObject - CMineObject>

class CServerMineObject : public CServerTeleportObject
{
public:
	CServerMineObject();
	virtual ~CServerMineObject();

	virtual void Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager);
	virtual void UpdateUsing(const shared_ptr<CServerGameObject>& pGameObject, shared_ptr<CServerCollisionManager>& pCollisionManager) override;

	virtual void SetRandomPosition(shared_ptr<CServerCollisionManager>& pCollisionManager) override;

	bool IsInstall() { return m_bInstall; }
	void SetInstall(bool val) { m_bInstall = val; }
	void SetExplosionTime(float val) { m_fExplosionTime = val; }
private:
	bool m_bInstall;
	float m_fExplosionTime = 0.0f;

};

/// <CGameObject - CMineObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CItemObject - CFuseObject>

class CServerFuseObject : public CServerItemObject
{
public:
	CServerFuseObject();
	virtual ~CServerFuseObject();

	virtual void Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager) override;
	virtual void UpdatePicking(INT8 nClientId) override;
	virtual void UpdateUsing(const shared_ptr<CServerGameObject>& pGameObject, shared_ptr<CServerCollisionManager>& pCollisionManager) override;

	virtual void SetRandomPosition(shared_ptr<CServerCollisionManager>& pCollisionManager) override;

	virtual void SetWorldMatrix(const XMFLOAT4X4& xmf4x4World) override;
};

/// <CGameObject - CFuseObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CItemObject - CRadarObject>

class CServerRadarObject : public CServerMineObject
{
public:
	CServerRadarObject();
	virtual ~CServerRadarObject();

	XMFLOAT4X4 m_xmf4x4Scalematrix;
	virtual void Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager) override;
	virtual void UpdateUsing(const shared_ptr<CServerGameObject>& pGameObject, shared_ptr<CServerCollisionManager>& pCollisionManager) override;
};

/// <CGameObject - CRadarObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CStairObject>

class CServerStairTriggerObject : public CServerGameObject
{
public:
	CServerStairTriggerObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb);

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