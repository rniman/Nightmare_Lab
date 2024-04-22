#include "stdafx.h"
#include "Collision.h"
#include "Object.h"
#include "Player.h"

int CCollisionManager::m_nCollisionObject = 0;

CCollisionManager g_collisionManager;

void CCollisionManager::CreateCollision(int nHeight, int nWidth, int nDepth)
{
	m_nHeight = nHeight;
	m_nWidth = nWidth;
	m_nDepth = nDepth;	

	m_collisionGridGameObjects.resize(m_nHeight);
	for (int i = 0; i < m_nHeight; ++i)
	{
		m_collisionGridGameObjects[i].resize(m_nWidth);
		for (int j = 0; j < m_nWidth; ++j)
		{
			m_collisionGridGameObjects[i][j].resize(m_nDepth);
		}
	}
	m_pCollisionObject.reserve(1000);
}

void CCollisionManager::AddCollisionObject(const shared_ptr<CGameObject>& pGameObject)
{
	//XMFLOAT3 xmf3Position = XMFLOAT3(pGameObject->m_xmf4x4World._41, pGameObject->m_xmf4x4World._42, pGameObject->m_xmf4x4World._43);

	//int nWidth = static_cast<int>((xmf3Position.x - GRID_START_X) / SPACE_SIZE_XZ);
	//int nHeight = static_cast<int>((xmf3Position.y - GRID_START_Y) / SPACE_SIZE_Y);
	//int nDepth = static_cast<int>((xmf3Position.z - GRID_START_Z) / SPACE_SIZE_XZ);

	//m_collisionGridGameObjects[nHeight][nWidth][nDepth].push_back(pGameObject);
	pGameObject->m_nCollisionNum = m_nCollisionObject++;
	m_pCollisionObject.emplace_back(pGameObject);
}

//size_t GetSizeGameObjects(CollisonLayer nLayer) { return m_v3dpCollisionGameObjects[static_cast<int>(nLayer)].size(); }

vpObjects_t& CCollisionManager::GetSpaceGameObjects(int nHeight, int nWidth, int nDepth)
{
	return m_collisionGridGameObjects[nHeight][nWidth][nDepth];
}

