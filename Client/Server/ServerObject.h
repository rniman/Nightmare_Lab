#pragma once
enum COLLISION_TYPE
{
	None = 0,
	Standard,
	Picking
};

class CCollisionManager;

class CGameObject
{
public:
	CGameObject() {};
	CGameObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb);
	virtual ~CGameObject() {};

	virtual void Update(float fElapsedTime);
	virtual void Collide(const shared_ptr<CCollisionManager>& pCollisionManager, float fElapsedTime, shared_ptr<CGameObject> pCollided) {};

	void Move(XMFLOAT3 xmf3Offset);
	virtual void UpdatePicking() {};
	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);

	XMFLOAT4X4 GetWorldMatrix()const { return m_xmf4x4World; }

	vector<BoundingOrientedBox> GetVectorOOBB() const { return m_voobbOrigin; };
	BoundingOrientedBox GetOOBB(int nIndex) { return m_voobbOrigin[nIndex]; }
	int GetCollisionType() const { return m_nCollisionType; }
protected:
	char m_pstrFrameName[64];
	XMFLOAT4X4 m_xmf4x4ToParent;
	XMFLOAT4X4 m_xmf4x4World;

	std::weak_ptr<CGameObject> m_pParent;
	std::shared_ptr<CGameObject> m_pChild;
	std::shared_ptr<CGameObject> m_pSibling;

	vector<BoundingOrientedBox> m_voobbOrigin;
	// 투명 오브젝트 분류
	bool m_bCollsion = true;
	int m_nCollisionType = 0; // 0:None, 1:Standard, 2:Picking
};

