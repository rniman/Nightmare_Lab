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
	CCollisionManager() {/* m_vvpCollisionGameObjects.resize(MaxCollisionLayer);*/};
	~CCollisionManager() {}

	void CreateCollision(int nHeight, int nWidth, int nDepth);

	void AddCollisionObject(const shared_ptr<CGameObject>& pGameObject);
	
	vpObjects_t& GetSpaceGameObjects(int nHeight, int nWidth, int nDepth);

	void Collide(float fElapsedTime);

	collisionGrid_t m_collisionGridGameObjects;
	weak_ptr<CPlayer> m_pPlayer;

	int GetHeight() const { return m_nHeight; }
	int GetWidth() const { return m_nWidth; }
	int GetDepth() const { return m_nDepth; }
private:
	int m_nHeight; // == Floor
	int m_nWidth; 
	int m_nDepth;
};

extern CCollisionManager g_collisonManager;

class CCollision
{
public:


private:

};

