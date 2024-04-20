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

	virtual void Update(float fElapsedTime);
	virtual void Collide(const shared_ptr<CServerCollisionManager>& pCollisionManager, float fElapsedTime, shared_ptr<CServerGameObject> pCollided) {};

	void Move(XMFLOAT3 xmf3Offset);
	virtual void UpdatePicking() {};
	virtual void UpdateUsing(const shared_ptr<CServerGameObject>& pGameObject) {};

	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);

	static bool CheckPicking(const shared_ptr<CServerGameObject>& pCollisionGameObject, const XMFLOAT3& xmf3PickPosition, const XMFLOAT4X4& xmf4x4ViewMatrix, float& fDistance);

	// Interface
	void SetCollisionNum(int nCollisionNum) { m_nCollisionNum = nCollisionNum; }

	XMFLOAT4X4 GetWorldMatrix()const { return m_xmf4x4World; }
	vector<BoundingOrientedBox> GetVectorOOBB() const { return m_voobbOrigin; };
	BoundingOrientedBox GetOOBB(int nIndex) { return m_voobbOrigin[nIndex]; }
	int GetCollisionType() const { return m_nCollisionType; }
	int GetCollisionNum() const { return m_nCollisionNum; }

	char* GetFrameName() { return m_pstrFrameName; }
protected:
	char m_pstrFrameName[64];
	XMFLOAT4X4 m_xmf4x4ToParent;
	XMFLOAT4X4 m_xmf4x4World;

	std::weak_ptr<CServerGameObject> m_pParent;
	std::shared_ptr<CServerGameObject> m_pChild;
	std::shared_ptr<CServerGameObject> m_pSibling;

	vector<BoundingOrientedBox> m_voobbOrigin;
	// 투명 오브젝트 분류
	bool m_bCollision = true;
	int m_nCollisionType = None; // 0:None, 1:Standard, 2:Picking
	int m_nCollisionNum = -1;
};

