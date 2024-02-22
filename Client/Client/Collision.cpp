#include "stdafx.h"
#include "Collision.h"

CCollisionManager g_collisonManager;

void CCollisionManager::AddCollisionObject(int nIndices, CGameObject* pGameObject)
{
	m_vvpCollisionGameObjects[nIndices].push_back(pGameObject);
}
