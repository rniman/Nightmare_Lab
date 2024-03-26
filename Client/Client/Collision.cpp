#include "stdafx.h"
#include "Collision.h"
#include "Object.h"
#include "Player.h"

CCollisionManager g_collisonManager;

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
}

void CCollisionManager::AddCollisionObject(const shared_ptr<CGameObject>& pGameObject)
{
	XMFLOAT3 xmf3Position = XMFLOAT3(pGameObject->m_xmf4x4World._41, pGameObject->m_xmf4x4World._42, pGameObject->m_xmf4x4World._43);

	int nWidth = static_cast<int>((xmf3Position.x - GRID_START_X) / SPACE_SIZE_XZ);
	int nHeight = static_cast<int>((xmf3Position.y - GRID_START_Y) / SPACE_SIZE_Y);
	int nDepth = static_cast<int>((xmf3Position.z - GRID_START_Z) / SPACE_SIZE_XZ);

	m_collisionGridGameObjects[nHeight][nWidth][nDepth].push_back(pGameObject);
}

//size_t GetSizeGameObjects(CollisonLayer nLayer) { return m_v3dpCollisionGameObjects[static_cast<int>(nLayer)].size(); }

vpObjects_t& CCollisionManager::GetSpaceGameObjects(int nHeight, int nWidth, int nDepth)
{
	
	return m_collisionGridGameObjects[nHeight][nWidth][nDepth];

}

void CCollisionManager::Collide(float fElapsedTime)
{
	shared_ptr<CPlayer> pPlayer = m_pPlayer.lock();
	if (!pPlayer)
	{
		return;
	}

	BoundingOrientedBox oobbPlayer;

	BoundingBox aabbPlayer;
	aabbPlayer.Center = pPlayer->GetOOBB(0).Center;
	aabbPlayer.Extents = pPlayer->GetOOBB(0).Extents;
	XMVECTOR xmvTranslation = XMVectorSet(pPlayer->GetPosition().x, pPlayer->GetPosition().y, pPlayer->GetPosition().z, 1.0f);
	aabbPlayer.Transform(aabbPlayer, 1.0f, XMQuaternionIdentity(), xmvTranslation);
	
	for (int i = pPlayer->GetWidth() - 1; i <= pPlayer->GetWidth() + 1; ++i)
	{
		for (int j = pPlayer->GetDepth() - 1; j <= pPlayer->GetDepth() + 1; ++j)
		{
			if (i < 0 || i >= g_collisonManager.GetWidth() || j < 0 || j >= g_collisonManager.GetDepth())
			{
				continue;
			}

			for (const auto& object : GetSpaceGameObjects(pPlayer->GetFloor(), i, j))	// 플레이어의 그리드를 나타내야함
			{
				shared_ptr<CGameObject> pGameObject = object.lock();
				if (!pGameObject)
				{
					continue;
				}
				if (pGameObject->GetCollisionType() == 2) // 임시로 2면 넘김
				{
					continue;
				}

				for (const auto& oobbOrigin : pGameObject->GetVectorOOBB())
				{
					BoundingOrientedBox oobb;
					oobbOrigin.Transform(oobb, XMLoadFloat4x4(&pGameObject->m_xmf4x4World));
					XMStoreFloat4(&oobb.Orientation, XMQuaternionNormalize(XMLoadFloat4(&oobb.Orientation)));

					if (!oobb.Intersects(aabbPlayer))
					{
						continue;
					}

					pPlayer->Collide(fElapsedTime, pGameObject);					
					break;
				}
			}
		}
	}
}

