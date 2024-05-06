#pragma once
enum COLLISION_TYPE
{
	None = 0,
	Standard,
	Picking,
	StairTrigger
};

class CServerCollisionManager;

class CServerGameObject : public std::enable_shared_from_this<CServerGameObject>
{
public:
	CServerGameObject() {};
	CServerGameObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb);
	virtual ~CServerGameObject() {};

	virtual void Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager);
	virtual void Collide(const shared_ptr<CServerCollisionManager>& pCollisionManager, float fElapsedTime, shared_ptr<CServerGameObject> pCollided) {};

	void Move(XMFLOAT3 xmf3Offset);
	virtual void UpdatePicking() {};
	virtual void UpdateUsing(const shared_ptr<CServerGameObject>& pGameObject, shared_ptr<CServerCollisionManager>& pCollisionManager) {};

	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);

	static bool CheckPicking(const shared_ptr<CServerGameObject>& pCollisionGameObject, const XMFLOAT3& xmf3PickPosition, const XMFLOAT4X4& xmf4x4ViewMatrix, float& fDistance);

	// Interface
	void SetCollision(bool bCollision) { m_bCollision = bCollision; }
	void SetCollisionNum(int nCollisionNum) { m_nCollisionNum = nCollisionNum; }

	void SetSpaceObject(int nFloor, int nWidth, int nDepth) { m_nFloor = nFloor; m_nWidth = nWidth; m_nDepth = nDepth; }

	XMFLOAT4X4 GetWorldMatrix()const { return m_xmf4x4World; }
	vector<BoundingOrientedBox> GetVectorOOBB() const { return m_voobbOrigin; };
	BoundingOrientedBox GetOOBB(int nIndex) { return m_voobbOrigin[nIndex]; }
	
	bool IsAlive() const { return m_bAlive; }
	bool IsCollision() const { return m_bCollision; }
	int GetCollisionType() const { return m_nCollisionType; }
	int GetCollisionNum() const { return m_nCollisionNum; }

	char* GetFrameName() { return m_pstrFrameName; }

	int GetWidth()const { return m_nWidth; }
	int GetDepth()const { return m_nDepth; }
	int GetFloor()const { return m_nFloor; }

	void SetPosition(XMFLOAT3 position);

	char m_pstrFrameName[64];
protected:
	XMFLOAT4X4 m_xmf4x4ToParent;
	XMFLOAT4X4 m_xmf4x4World;

	std::weak_ptr<CServerGameObject> m_pParent;
	std::shared_ptr<CServerGameObject> m_pChild;
	std::shared_ptr<CServerGameObject> m_pSibling;

	vector<BoundingOrientedBox> m_voobbOrigin;

	bool m_bAlive = true;
	bool m_bCollision = true;
	int m_nCollisionType = None; // 0:None, 1:Standard, 2:Picking
	int m_nCollisionNum = -1;

	int m_nFloor = 0;
	int m_nWidth = 0;
	int m_nDepth = 0;
};

