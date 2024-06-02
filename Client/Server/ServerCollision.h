#pragma once
#include "TCPServer.h"

constexpr int SPACE_FLOOR{ 4 };
constexpr int SPACE_WIDTH{ 30 };
constexpr int SPACE_DEPTH{ 30 };

constexpr float SPACE_SIZE_XZ{ 6.0f };
constexpr float SPACE_SIZE_Y{ 4.5f };
constexpr float GRID_START_X{ -90.0f };
constexpr float GRID_START_Y{ -0.01f };
constexpr float GRID_START_Z{ -90.0f };

class CServerGameObject;
class CServerPlayer;

typedef vector<shared_ptr<CServerGameObject>> vpObjects_t;
typedef vector<vector<vector<vpObjects_t>>> collisionGrid_t;

class CServerCollisionManager : public std::enable_shared_from_this<CServerCollisionManager>
{
public:
	CServerCollisionManager() {};
	~CServerCollisionManager() {}

	void CreateCollision(int nHeight, int nWidth, int nDepth);
	void AddCollisionObject(const shared_ptr<CServerGameObject>& pGameObject);
	void ReplaceCollisionObject(const shared_ptr<CServerGameObject>& pGameObject);
	void AddCollisionPlayer(const shared_ptr<CServerPlayer>& pPlayer, int nIndex);

	vpObjects_t& GetSpaceGameObjects(int nHeight, int nWidth, int nDepth);

	void Update(float fElapsedTime);
	void Collide(float fElapsedTime, const shared_ptr<CServerPlayer>& pPlayer);
	void PlayerInStair(const std::shared_ptr<CServerPlayer>& pPlayer);
	void CheckStairTrigger(const std::shared_ptr<CServerGameObject>& pGameObject, DirectX::BoundingSphere& aabbPlayer, const std::shared_ptr<CServerPlayer>& pPlayer);
	void CollideWithPlayer(float fElapsedTime);
	void CollideWithMine(shared_ptr<CServerGameObject> pGameObject, shared_ptr<CServerGameObject> pPlayer, BoundingBox& aabbPlayer);

	collisionGrid_t m_collisionGridGameObjects;
	std::array<weak_ptr<CServerPlayer>, MAX_CLIENT> m_apPlayer;

	static int GetNumberOfCollisionObject() { return CServerCollisionManager::m_nCollisionObject; }
	shared_ptr<CServerGameObject> GetCollisionObjectWithNumber(int nCollisionNum) { return m_vpCollisionObject[nCollisionNum]; }
	vector<shared_ptr<CServerGameObject>> GetOutSpaceObject() { return m_vpOutSpaceObject; }

	int GetHeight() const { return m_nHeight; }
	int GetWidth() const { return m_nWidth; }
	int GetDepth() const { return m_nDepth; }

private:
	static int m_nCollisionObject;

	vector<shared_ptr<CServerGameObject>> m_vpCollisionObject;	// 번호를 이용해서 충돌 객체를 빠르게 가져오기위함
	vector<shared_ptr<CServerGameObject>> m_vpOutSpaceObject;	// 공간 분할의 업데이트가 일어나면 어떤 오브젝트는 한번 공간외에서의 업데이트가 필요하다

	int m_nHeight; // == Floor
	int m_nWidth;
	int m_nDepth;
};
