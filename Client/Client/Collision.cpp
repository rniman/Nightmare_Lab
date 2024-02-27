#include "stdafx.h"
#include "Collision.h"

CCollisionManager g_collisonManager;

void CCollisionManager::AddCollisionObject(int nIndices, const shared_ptr<CGameObject>& pGameObject)
{
	m_vvpCollisionGameObjects[nIndices].push_back(pGameObject);
}
