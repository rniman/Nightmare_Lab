#pragma once

constexpr float SPACE_SIZE_XZ{ 6.0f };
constexpr float SPACE_SIZE_Y{ 4.5f };
constexpr float GRID_START_X{ -30.0f };
constexpr float GRID_START_Y{ -0.5f };
constexpr float GRID_START_Z{ -30.0f };

class CGameObject;
class CPlayer;

typedef vector<weak_ptr<CGameObject>> vpObjects_t;
typedef vector<vector<vector<vpObjects_t>>> collisionGrid_t;

class CCollisionManager
{
public:
	CCollisionManager() {};
	~CCollisionManager() {}

	void CreateCollision(int nHeight, int nWidth, int nDepth);

	void AddCollisionObject(const shared_ptr<CGameObject>& pGameObject);
	
	vpObjects_t& GetSpaceGameObjects(int nHeight, int nWidth, int nDepth);

	//void Collide(float fElapsedTime);

	collisionGrid_t m_collisionGridGameObjects;
	weak_ptr<CPlayer> m_pPlayer;

	weak_ptr<CGameObject> GetCollisionObjectWithNumber(int nCollisionNum)
	{
		return m_pCollisionObject[nCollisionNum];
	}

	int GetHeight() const { return m_nHeight; }
	int GetWidth() const { return m_nWidth; }
	int GetDepth() const { return m_nDepth; }
private:
	static int m_nCollisionObject;

	vector<weak_ptr<CGameObject>> m_pCollisionObject;	// 번호를 이용해서 충돌 객체를 빠르게 가져오기위함

	int m_nHeight = -1; // == Floor
	int m_nWidth = -1; 
	int m_nDepth = -1;
};

extern CCollisionManager g_collisionManager;
