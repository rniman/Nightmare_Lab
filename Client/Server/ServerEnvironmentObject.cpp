#include "stdafx.h"
#include "ServerEnvironmentObject.h"
#include "ServerPlayer.h"
#include "TCPServer.h"
#include "ServerCollision.h"

vector<pair<int, int>> CServerItemObject::m_vDrawerId;

// Drawer1 2에 따라 다르게 offset을 준다
float CServerItemObject::m_fDrawer1OffsetY{ 0.5f };
float CServerItemObject::m_fDrawer2OffsetY{ 0.2f };

CServerItemObject::CServerItemObject()
	: CServerGameObject()
{
	m_nCollisionType = Picking;
}

void CServerItemObject::Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	//CServerGameObject::Update(fElapsedTime);
	m_xmf4x4World = m_pDrawerObject->GetWorldMatrix();
	m_xmf4x4ToParent = m_pDrawerObject->GetWorldMatrix();
}

void CServerItemObject::SetDrawerIdContainer(vector<pair<int, int>> vDrawerId)
{
	m_vDrawerId = vDrawerId;
}


void CServerItemObject::SetWorldMatrix(const XMFLOAT4X4& xmf4x4World)
{
	XMFLOAT4X4 m_xmf4x4FinalWorld = xmf4x4World;
	if (m_nDrawerType == 1) // 1 == Drawer1
	{
		m_xmf4x4FinalWorld._42 += m_fDrawer1OffsetY;
	}
	else
	{
		m_xmf4x4FinalWorld._42 += m_fDrawer2OffsetY;
	}

	// 왜 Z축 회전인지는 모르겠;;
	XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::Rotate(m_xmf3Rotation.x, m_xmf3Rotation.y, m_xmf3Rotation.z);
	m_xmf4x4FinalWorld = Matrix4x4::Multiply(xmf4x4Rotate, m_xmf4x4FinalWorld);

	m_xmf4x4World = m_xmf4x4FinalWorld;
	m_xmf4x4ToParent = m_xmf4x4FinalWorld;
}

void CServerItemObject::SetRandomOffset(const XMFLOAT3& xmf3Offset)
{
	m_xmf3PositionOffset = xmf3Offset;
}

/// <CGameObject - CItemObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CEnvironmentObject>

CServerEnvironmentObject::CServerEnvironmentObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb)
	: CServerGameObject(pstrFrameName, xmf4x4World, voobb)
{
	m_nCollisionType = Standard;
	m_bStatic = true;
}

/// <CGameObject - CEnvironmentObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CDoorObject>

CServerDrawerObject::CServerDrawerObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb)
	: CServerGameObject(pstrFrameName, xmf4x4World, voobb)
{
	m_nCollisionType = Picking;

	m_xmf3OriginPosition = XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43);
	m_xmf3Forward = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMMATRIX mtxWorld = XMLoadFloat4x4(&m_xmf4x4World);
	m_xmf3Forward = Vector3::TransformNormal(m_xmf3Forward, mtxWorld);
}

CServerDrawerObject::~CServerDrawerObject()
{
}

void CServerDrawerObject::Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	XMFLOAT3 xmf3Position = XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
	float fDistance = Vector3::Distance(xmf3Position, m_xmf3OriginPosition);

	if (m_bAnimate)
	{
		if (m_bOpened)
		{
			if (fDistance < 0.6f)
			{
				XMFLOAT3 xmf3Offset = Vector3::ScalarProduct(m_xmf3Forward, fElapsedTime * 2.0f);
				Move(xmf3Offset);
			}
			else
			{
				m_bAnimate = false;
			}
		}
		else
		{
			if (fDistance >= 0.0f)
			{
				XMFLOAT3 xmf3Offset = Vector3::ScalarProduct(m_xmf3Forward, -fElapsedTime * 2.0f);
				Move(xmf3Offset);
				xmf3Position = XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
				XMFLOAT3 xmf3ToPosition = Vector3::Subtract(xmf3Position, m_xmf3OriginPosition);
				if (Vector3::DotProduct(m_xmf3Forward, xmf3ToPosition) < 0.0f)
				{
					m_xmf4x4World._41 = m_xmf3OriginPosition.x;
					m_xmf4x4World._42 = m_xmf3OriginPosition.y;
					m_xmf4x4World._43 = m_xmf3OriginPosition.z;
					m_bAnimate = false;
				}
			}
		}
	}
}

void CServerDrawerObject::UpdatePicking(INT8 nClientId)
{
	if (m_bOpened)
	{
		m_bOpened = false;
		m_bAnimate = true;
		PostMessage(TCPServer::m_hWnd, WM_SOUND, (WPARAM)SOUND_MESSAGE::CLOSE_DRAWER, (LPARAM)nClientId);
	}
	else
	{
		m_bOpened = true;
		m_bAnimate = true;
		PostMessage(TCPServer::m_hWnd, WM_SOUND, (WPARAM)SOUND_MESSAGE::OPEN_DRAWER, (LPARAM)nClientId);
	}
	
	//소리를 알려야함
	
}

/// <CGameObject - CDrawerObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CDoorObject>

CServerDoorObject::CServerDoorObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb)
	: CServerGameObject(pstrFrameName, xmf4x4World, voobb)
{
	m_nCollisionType = Standard;
}

CServerDoorObject::~CServerDoorObject()
{
}

void CServerDoorObject::Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	if ((m_bOpened && m_fRotationAngle < m_fDoorAngle) || (!m_bOpened && m_fRotationAngle > m_fDoorAngle))
	{
		m_fRotationAngle += m_bOpened ? fElapsedTime * 120.0f : -fElapsedTime * 120.0f;
		float fRotationAngle = m_bOpened ? XMConvertToRadians(fElapsedTime * 120.0f) : -XMConvertToRadians(fElapsedTime * 120.0f);
		if ((m_bOpened && m_fRotationAngle > m_fDoorAngle) || (!m_bOpened && m_fRotationAngle < m_fDoorAngle))
		{
			m_fRotationAngle = m_fDoorAngle;
		}
		else
		{
			//Z-up 모델임
			XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.0f), XMConvertToRadians(0.0f), fRotationAngle);
			m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);

			// Instancing 객체는 world행렬만 수정해주면됨
			//UpdateTransform(NULL);
		}
	}

}

void CServerDoorObject::UpdatePicking(INT8 nClientId)
{
	if (m_bOpened)
	{
		m_bOpened = false;
		m_fDoorAngle = 0.0f;
		PostMessage(TCPServer::m_hWnd, WM_SOUND, (WPARAM)SOUND_MESSAGE::CLOSE_DOOR, (LPARAM)nClientId);
	}
	else
	{
		m_bOpened = true;
		m_fDoorAngle = 150.0f;
		PostMessage(TCPServer::m_hWnd, WM_SOUND, (WPARAM)SOUND_MESSAGE::OPEN_DOOR, (LPARAM)nClientId);
	}

}

/// <CGameObject - CDrawerObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CElevatorDoorObject>

CServerElevatorDoorObject::CServerElevatorDoorObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb)
	: CServerGameObject(pstrFrameName, xmf4x4World, voobb)
{
	m_nCollisionType = Standard;

	m_xmf3OriginPosition = XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43);
	m_xmf3Right = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMMATRIX mtxWorld = XMLoadFloat4x4(&m_xmf4x4World);
	m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, mtxWorld);
}

void CServerElevatorDoorObject::Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	if (!IsEscape()) 
	{
		return;
	}

	XMFLOAT3 xmf3Position = XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
	float fDistance = Vector3::Distance(xmf3Position, m_xmf3OriginPosition);

	if (m_bAnimate)
	{
		if (m_bOpened)
		{
			if (fDistance < 3.0f)
			{
				XMFLOAT3 xmf3Offset = Vector3::ScalarProduct(m_xmf3Right, fElapsedTime * 2.0f);
				Move(xmf3Offset);
			}
			else
			{
				m_bAnimate = false;
			}
		}
		else
		{
			if (fDistance >= 0.0f)
			{
				XMFLOAT3 xmf3Offset = Vector3::ScalarProduct(m_xmf3Right, -fElapsedTime * 2.0f);
				Move(xmf3Offset);
				xmf3Position = XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
				XMFLOAT3 xmf3ToPosition = Vector3::Subtract(xmf3Position, m_xmf3OriginPosition);
				if (Vector3::DotProduct(m_xmf3Right, xmf3ToPosition) < 0.0f)
				{
					m_xmf4x4World._41 = m_xmf3OriginPosition.x;
					m_xmf4x4World._42 = m_xmf3OriginPosition.y;
					m_xmf4x4World._43 = m_xmf3OriginPosition.z;
					m_bAnimate = false;
				}
			}
		}
	}

}

void CServerElevatorDoorObject::UpdatePicking(INT8 nClientId)
{
}

void CServerElevatorDoorObject::EscapeDoorOpen(INT8 nClientId)
{
	if (!IsEscape()) {
		return;
	}
	if (m_bOpened)
	{
		m_bOpened = false;
		m_bAnimate = true;
	}
	else
	{
		m_bOpened = true;
		m_bAnimate = true;
	}
}


/// <CGameObject - CElevatorDoorObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CItemObject - CTeleportObject>

CServerTeleportObject::CServerTeleportObject()
	: CServerItemObject()
{
	m_xmf3PositionOffset = XMFLOAT3(0.0f, 0.0f, 0.0f);

	BoundingOrientedBox oobb;
	oobb.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	oobb.Extents = XMFLOAT3(0.1370034f / 2.0f, 0.1370034f / 2.0f, 0.1370034f / 2.0f);
	XMStoreFloat4(&oobb.Orientation, XMQuaternionIdentity());
	m_voobbOrigin.push_back(oobb);

	strcpy(m_pstrFrameName, "TeleportObject");
}

CServerTeleportObject::~CServerTeleportObject()
{
}

//void CTeleportObject::Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager)
//{
//	CGameObject::Animate(fElapsedTime);
//}

void CServerTeleportObject::Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	if (m_bObtained)
	{
		return;
	}

	XMFLOAT4X4 xmf4x4FuseWorld = m_pDrawerObject->GetWorldMatrix();
	xmf4x4FuseWorld._41 += m_xmf3PositionOffset.x;
	xmf4x4FuseWorld._42 += m_xmf3PositionOffset.y;
	xmf4x4FuseWorld._43 += m_xmf3PositionOffset.z;

	CServerItemObject::SetWorldMatrix(xmf4x4FuseWorld);
}

void CServerTeleportObject::UpdatePicking(INT8 nClientId)
{
	m_bObtained = true;
	m_bCollision = false;
}

void CServerTeleportObject::UpdateUsing(const shared_ptr<CServerGameObject>& pGameObject, shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	shared_ptr<CServerBlueSuitPlayer> pBlueSuitPlayer = dynamic_pointer_cast<CServerBlueSuitPlayer>(pGameObject);
	if (!pBlueSuitPlayer)
	{
		return;
	}
	pBlueSuitPlayer->TeleportItemUse();

	// 아이템 리셋하는 함수 작성
	m_bObtained = false;
	m_bCollision = true;
	SetRandomPosition(pCollisionManager);
}

void CServerTeleportObject::SetRandomPosition(shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	dynamic_pointer_cast<CServerDrawerObject>(pCollisionManager->GetCollisionObjectWithNumber(m_nDrawerNumber))->m_pStoredItem.reset();

	uniform_int_distribution<int> dis(0, m_vDrawerId.size() - 1);
	uniform_int_distribution<int> rotation_dis(1, 360);
	uniform_real_distribution<float> pos_dis(-0.2f, 0.2f);
	while (true)
	{
		int rd_num = dis(TCPServer::m_mt19937Gen);
		int nDrawerNum = m_vDrawerId[rd_num].first;
		shared_ptr<CServerDrawerObject> pDrawerObject = dynamic_pointer_cast<CServerDrawerObject>(pCollisionManager->GetCollisionObjectWithNumber(nDrawerNum));

		if (!pDrawerObject) //error
			assert(0);
			//exit(1);

		if (pDrawerObject->m_pStoredItem)	// 이미 다른 아이템이 들어왔음
		{
			continue;
		}

		XMFLOAT4X4 xmf4x4World = pCollisionManager->GetCollisionObjectWithNumber(nDrawerNum)->GetWorldMatrix();

		XMFLOAT3 xmf3RandOffset = XMFLOAT3(pos_dis(TCPServer::m_mt19937Gen), 0.0f, pos_dis(TCPServer::m_mt19937Gen));
		XMFLOAT3 xmf3RandRotation = XMFLOAT3(0.0f, 0.0f, (float)rotation_dis(TCPServer::m_mt19937Gen));

		SetDrawerNumber(nDrawerNum);
		SetDrawer(pDrawerObject);
		SetDrawerType(m_vDrawerId[rd_num].second);
		pDrawerObject->m_pStoredItem = dynamic_pointer_cast<CServerTeleportObject>(shared_from_this());

		m_bObtained = false;
		m_bCollision = true;

		SetRandomRotation(xmf3RandRotation);
		SetRandomOffset(xmf3RandOffset);

		SetWorldMatrix(xmf4x4World);

		// 공간의 위치를 다시 놓는다.
		auto iter = pCollisionManager->GetSpaceGameObjects(m_nFloor, m_nWidth, m_nDepth).begin();
		int i = 0;
		for (auto pGameObject : pCollisionManager->GetSpaceGameObjects(m_nFloor, m_nWidth, m_nDepth))
		{
			if (pGameObject->GetCollisionNum() == m_nCollisionNum)	// 동일한 
			{
				pCollisionManager->GetSpaceGameObjects(m_nFloor, m_nWidth, m_nDepth).erase(iter + i);
				break;
			}
			++i;
		}
		pCollisionManager->ReplaceCollisionObject(shared_from_this());

		//printf("%d New Pos: %f %f %f\n", m_nCollisionNum, m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
		break;
	}
}

void CServerTeleportObject::SetWorldMatrix(const XMFLOAT4X4& xmf4x4World)
{
	XMFLOAT4X4 xmf4x4TeleportWorld = xmf4x4World;
	xmf4x4TeleportWorld._41 += m_xmf3PositionOffset.x;
	xmf4x4TeleportWorld._42 += m_xmf3PositionOffset.y;
	xmf4x4TeleportWorld._43 += m_xmf3PositionOffset.z;

	CServerItemObject::SetWorldMatrix(xmf4x4TeleportWorld);
}

/// <CGameObject - CTeleportObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CMineObject>

CServerMineObject::CServerMineObject()
{
	m_xmf3PositionOffset = XMFLOAT3(0.0f, 0.0f, 0.0f);

	BoundingOrientedBox oobb;
	oobb.Center = XMFLOAT3(0.003596, -0.00012212, -0.01279);
	oobb.Extents = XMFLOAT3(0.26312/2, 0.066229/2, 0.27357/2);
	XMStoreFloat4(&oobb.Orientation, XMQuaternionIdentity());

	m_voobbOrigin.clear();
	m_voobbOrigin.push_back(oobb);

	strcpy(m_pstrFrameName, "MineObject");

	m_bInstall = false;

}

CServerMineObject::~CServerMineObject()
{
}

void CServerMineObject::Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	if (m_bInstall) {
		if (m_fExplosionTime > 0.0f) {
			m_fExplosionTime -= fElapsedTime;

			if (m_fExplosionTime < 0.0f) { // 폭발이 끝난 후에 오브젝트를 재배치함
				m_bInstall = false;
				SetRandomPosition(pCollisionManager);
			}
		}

	}
	else {
		if (m_bObtained)
		{
			return;
		}

		XMFLOAT4X4 xmf4x4FuseWorld = m_pDrawerObject->GetWorldMatrix();
		xmf4x4FuseWorld._41 += m_xmf3PositionOffset.x;
		xmf4x4FuseWorld._42 += m_xmf3PositionOffset.y;
		xmf4x4FuseWorld._43 += m_xmf3PositionOffset.z;

		CServerItemObject::SetWorldMatrix(xmf4x4FuseWorld);
	}
}

void CServerMineObject::UpdateUsing(const shared_ptr<CServerGameObject>& pGameObject, shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	shared_ptr<CServerBlueSuitPlayer> pBlueSuitPlayer = dynamic_pointer_cast<CServerBlueSuitPlayer>(pGameObject);
	if (!pBlueSuitPlayer)
	{
		return;
	}

	//// 아이템 리셋하는 함수 작성

	m_bObtained = false;
	m_bCollision = true;

	m_bInstall = true;
	XMFLOAT3 position = pBlueSuitPlayer->GetPosition();

	auto iter = pCollisionManager->GetSpaceGameObjects(m_nFloor, m_nWidth, m_nDepth).begin();
	int i = 0;
	for (auto pGameObject : pCollisionManager->GetSpaceGameObjects(m_nFloor, m_nWidth, m_nDepth))
	{
		if (pGameObject->GetCollisionNum() == m_nCollisionNum)	// 동일한 
		{
			pCollisionManager->GetSpaceGameObjects(m_nFloor, m_nWidth, m_nDepth).erase(iter + i);
			break;
		}
		++i;
	}
	SetPosition(position);
	pCollisionManager->ReplaceCollisionObject(shared_from_this());
}


/// <CGameObject - CMineObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CFuseObject>

CServerFuseObject::CServerFuseObject()
	: CServerItemObject()
{
	m_xmf3PositionOffset = XMFLOAT3(0.0f, 0.0f, 0.0f);

	BoundingOrientedBox oobb;
	oobb.Center = XMFLOAT3(-0.0148698f, 0.0f, 0.0f);
	oobb.Extents = XMFLOAT3(0.1315422f / 2.0f, 0.0457827f / 2.0f, 0.0599868f / 2.0f);
	XMStoreFloat4(&oobb.Orientation, XMQuaternionIdentity());
	m_voobbOrigin.push_back(oobb);

	strcpy(m_pstrFrameName, "FuseObject");
}

CServerFuseObject::~CServerFuseObject()
{
}

void CServerFuseObject::Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	if (m_bObtained)
	{
		return;
	}

	XMFLOAT4X4 xmf4x4FuseWorld = m_pDrawerObject->GetWorldMatrix();
	xmf4x4FuseWorld._41 += m_xmf3PositionOffset.x;
	xmf4x4FuseWorld._42 += m_xmf3PositionOffset.y;
	xmf4x4FuseWorld._43 += m_xmf3PositionOffset.z;

	CServerItemObject::SetWorldMatrix(xmf4x4FuseWorld);
}

void CServerFuseObject::UpdatePicking(INT8 nClientId)
{
	m_bObtained = true;
	m_bCollision = false;
}

void CServerFuseObject::UpdateUsing(const shared_ptr<CServerGameObject>& pGameObject, shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	shared_ptr<CServerBlueSuitPlayer> pBlueSuitPlayer = dynamic_pointer_cast<CServerBlueSuitPlayer>(pGameObject);
	if (!pBlueSuitPlayer)
	{
		return;
	}
	m_bObtained = false;
	m_bCollision = true;

	SetRandomPosition(pCollisionManager);
}

void CServerFuseObject::SetRandomPosition(shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	dynamic_pointer_cast<CServerDrawerObject>(pCollisionManager->GetCollisionObjectWithNumber(m_nDrawerNumber))->m_pStoredItem.reset();

	uniform_int_distribution<int> dis(0, m_vDrawerId.size() - 1);
	uniform_int_distribution<int> rotation_dis(1, 360);
	uniform_real_distribution<float> pos_dis(-0.2f, 0.2f);
	while (true)
	{
		int rd_num = dis(TCPServer::m_mt19937Gen);
		int nDrawerNum = m_vDrawerId[rd_num].first;
		shared_ptr<CServerDrawerObject> pDrawerObject = dynamic_pointer_cast<CServerDrawerObject>(pCollisionManager->GetCollisionObjectWithNumber(nDrawerNum));

		if (!pDrawerObject) //error
		{
			//exit(1);
			assert(10);
		}

		if (pDrawerObject->m_pStoredItem)	// 이미 다른 아이템이 들어왔음
		{
			continue;
		}

		XMFLOAT4X4 xmf4x4World = pCollisionManager->GetCollisionObjectWithNumber(nDrawerNum)->GetWorldMatrix();

		XMFLOAT3 xmf3RandOffset = XMFLOAT3(pos_dis(TCPServer::m_mt19937Gen), 0.0f, pos_dis(TCPServer::m_mt19937Gen));
		XMFLOAT3 xmf3RandRotation = XMFLOAT3(0.0f, 0.0f, (float)rotation_dis(TCPServer::m_mt19937Gen));

		SetDrawerNumber(nDrawerNum);
		SetDrawer(pDrawerObject);
		SetDrawerType(m_vDrawerId[rd_num].second);
		pDrawerObject->m_pStoredItem = dynamic_pointer_cast<CServerFuseObject>(shared_from_this());

		SetRandomRotation(xmf3RandRotation);
		SetRandomOffset(xmf3RandOffset);

		SetWorldMatrix(xmf4x4World);

		// 공간의 위치를 다시 놓는다.
		auto iter = pCollisionManager->GetSpaceGameObjects(m_nFloor, m_nWidth, m_nDepth).begin();
		int i = 0;
		for (auto pGameObject : pCollisionManager->GetSpaceGameObjects(m_nFloor, m_nWidth, m_nDepth))
		{
			if (pGameObject->GetCollisionNum() == m_nCollisionNum)	// 동일한 
			{
				pCollisionManager->GetSpaceGameObjects(m_nFloor, m_nWidth, m_nDepth).erase(iter + i);
				break;
			}
			++i;
		}
		pCollisionManager->ReplaceCollisionObject(shared_from_this());

		//printf("%d New Pos: %f %f %f\n", m_nCollisionNum, m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
		break;
	}
}

void CServerFuseObject::SetWorldMatrix(const XMFLOAT4X4& xmf4x4World)
{
	XMFLOAT4X4 xmf4x4FuseWorld = xmf4x4World;
	xmf4x4FuseWorld._41 += m_xmf3PositionOffset.x;
	xmf4x4FuseWorld._42 += m_xmf3PositionOffset.y;
	xmf4x4FuseWorld._43 += m_xmf3PositionOffset.z;

	CServerItemObject::SetWorldMatrix(xmf4x4FuseWorld);
}

/// <CGameObject - CFuseObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CRadarObject>

CServerRadarObject::CServerRadarObject()
{
	m_xmf3PositionOffset = XMFLOAT3(0.0f, 0.0f, 0.0f);

	BoundingOrientedBox oobb;
	oobb.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	oobb.Extents = XMFLOAT3(0.16935, 0.034772, 0.195917);
	XMStoreFloat4(&oobb.Orientation, XMQuaternionIdentity());

	m_voobbOrigin.clear();
	m_voobbOrigin.push_back(oobb);

	strcpy(m_pstrFrameName, "RaderObject");

}

CServerRadarObject::~CServerRadarObject()
{
}

void CServerRadarObject::Update(float fElapsedTime, shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	if (m_bObtained)
	{
		return;
	}
	
	XMFLOAT4X4 xmf4x4RadarWorld = m_pDrawerObject->GetWorldMatrix();
	
	xmf4x4RadarWorld._41 += m_xmf3PositionOffset.x;
	xmf4x4RadarWorld._42 += m_xmf3PositionOffset.y;
	xmf4x4RadarWorld._43 += m_xmf3PositionOffset.z;

	CServerItemObject::SetWorldMatrix(xmf4x4RadarWorld);
}

void CServerRadarObject::UpdateUsing(const shared_ptr<CServerGameObject>& pGameObject, shared_ptr<CServerCollisionManager>& pCollisionManager)
{
	shared_ptr<CServerBlueSuitPlayer> pBlueSuitPlayer = dynamic_pointer_cast<CServerBlueSuitPlayer>(pGameObject);
	if (!pBlueSuitPlayer)
	{
		return;
	}

	//// 아이템 리셋하는 함수 작성
	//m_bObtained = false;
	//m_bCollision = true;
	//SetRandomPosition(pCollisionManager);
}

/// <CGameObject - CRadarObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CStairTriggerObject>

CServerStairTriggerObject::CServerStairTriggerObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb)
	: CServerGameObject(pstrFrameName, xmf4x4World, voobb)
{
	m_nCollisionType = COLLISION_TYPE::StairTrigger;
	XMFLOAT3 xmf3Position = XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
	XMFLOAT3 xmf3Point1, xmf3Point2, xmf3Point3;

	/*float fx, fy, fz, fOffsetY;*/
	if (xmf3Position.y < 1.0f) // 1층
	{
		m_fy = 0.2f;
	}
	else if (xmf3Position.y < 5.5f) // 2층
	{
		m_fy = 4.7f;
	}
	else if (xmf3Position.y < 10.0f) // 3층
	{
		m_fy = 9.2f;
	}
	else if (xmf3Position.y < 14.5f) // 4층
	{
		m_fy = 13.7f;
	}

	if (xmf3Position.z > 0.0f) 
	{
		if (xmf3Position.x > 0.0f) // 계단 아래
		{
			m_fOffsetY = 4.5f;
			m_fx = 8.0f;
		}
		else
		{
			m_fOffsetY = -4.5f;
			m_fx = -8.0f;
		}
		m_fz = 13.9;
	}
	else
	{
		if (xmf3Position.x > 0.0f) // 계단 위
		{
			m_fOffsetY = -4.5f;
			m_fx = 8.0f;
		}
		else
		{
			m_fOffsetY = 4.5f;
			m_fx = -8.0f;
		}
		m_fz = -13.9;
	}
	xmf3Point1 = XMFLOAT3(m_fx, m_fy, m_fz + 3.5f / 2.f);
	xmf3Point2 = XMFLOAT3(m_fx, m_fy, m_fz - 3.5f / 2.f);
	xmf3Point3 = XMFLOAT3(-m_fx, m_fy + m_fOffsetY, m_fz + 3.5f / 2.f);
	m_xmf4Plane = Plane::CreateFromPoints(xmf3Point1, xmf3Point2, xmf3Point3);
}
