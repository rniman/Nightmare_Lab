#include "stdafx.h"
#include "TCPClient.h"
#include "GameFramework.h"
#include "Player.h"

CTcpClient::CTcpClient(HWND hWnd)
{
	CreateSocket(hWnd);
}

CTcpClient::~CTcpClient()
{
}

void CTcpClient::CreateSocket(HWND hWnd)
{
	int nRetval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		return;
	}

	// 소켓 생성
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	m_sock = s;
	//m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET)
	{
		err_quit("socket()");
	}

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);

	//nRetval = connect(m_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	nRetval = connect(s, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (nRetval == SOCKET_ERROR) 
	{
		err_quit("connect()");
		return;
	}

	//nRetval = WSAAsyncSelect(m_sock, hWnd, WM_SOCKET, FD_CLOSE | FD_READ | FD_WRITE);	// FD_WRITE가 발생할것이다.
	nRetval = WSAAsyncSelect(s, hWnd, WM_SOCKET, FD_CLOSE | FD_READ | FD_WRITE);	// FD_WRITE가 발생할것이다.
}

void CTcpClient::OnDestroy()
{

}


XMFLOAT3 CTcpClient::GetPostion(int id)
{
	XMFLOAT3 position = { 0.0f,0.0f, 0.0f };

	return position;
}

std::array<CS_CLIENTS_INFO, 5>& CTcpClient::GetArrayClientsInfo()
{
	return m_aClientInfo;
}

void CTcpClient::OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_READ:	// 소켓이 데이터를 읽을 준비가 되었다.
		OnProcessingReadMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case FD_WRITE:	// 소켓이 데이터를 전송할 준비가 되었다.
		OnProcessingWriteMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case FD_CLOSE:
		closesocket(wParam);
		WSACleanup();	
		break;
	default:
		break;
	}
}

void CTcpClient::OnProcessingReadMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	static int nHead;
	int nRetval = 1;
	size_t nBufferSize;

	if(!m_bRecvHead)
	{
		nBufferSize = sizeof(int);
		nRetval = RecvData(wParam, nBufferSize);
		if (nRetval != 0)
		{
			if (nRetval == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
			{
				m_bRecvHead = false;
				nHead = -1;
				memset(m_pCurrentBuffer, 0, BUFSIZE);
			}
			return;
		}
		m_bRecvHead = true;
		memcpy(&nHead, m_pCurrentBuffer, sizeof(int));
		memset(m_pCurrentBuffer, 0, BUFSIZE);
	}

	switch (nHead)
	{
	case HEAD_INIT:
		nBufferSize = sizeof(int) * 2;
		RecvNum++;
		nRetval = RecvData(wParam, nBufferSize);
		if (nRetval != 0)
		{
			break;
		}

		memcpy(&m_nMainClientId, m_pCurrentBuffer, sizeof(int));
		memcpy(&m_nClient, m_pCurrentBuffer + sizeof(int), sizeof(int));
		break;
	case HEAD_UPDATE_DATA:
	{
		nBufferSize = sizeof(m_aClientInfo);
		RecvNum++;
		nRetval = RecvData(wParam, nBufferSize);
		if (nRetval != 0)
		{
			break;
		}
		
		memcpy(m_aClientInfo.data(), m_pCurrentBuffer, sizeof(m_aClientInfo));

		for (int i = 0; i < MAX_CLIENT; ++i)
		{
			if (m_apPlayers[i])
			{
				m_apPlayers[i]->SetAlive(m_aClientInfo[i].m_bAlive);
				m_apPlayers[i]->SetClientId(m_aClientInfo[i].m_nClientId);
				m_apPlayers[i]->SetPosition(m_aClientInfo[i].m_xmf3Position);
				m_apPlayers[i]->SetVelocity(m_aClientInfo[i].m_xmf3Velocity);
				
				if(i != m_nMainClientId)
				{
					m_apPlayers[i]->SetLook(m_aClientInfo[i].m_xmf3Look);
					m_apPlayers[i]->SetRight(m_aClientInfo[i].m_xmf3Right);
				}
			}

			if (i == 0)
			{
				UpdateZombiePlayer();
			}
			else
			{
				UpdatePlayerItem(i);
			}

			int nNumOfGameObject = m_aClientInfo[i].m_nNumOfObject;
			for (int j = 0; j < nNumOfGameObject; ++j)
			{
				int nObjectNum = m_aClientInfo[i].m_anObjectNum[j];


				if (nObjectNum == -1)
				{
					continue;
				}
				shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(nObjectNum).lock();
				if (pGameObject)
				{
					pGameObject->m_xmf4x4World = m_aClientInfo[i].m_axmf4x4World[j];
					pGameObject->m_xmf4x4ToParent = m_aClientInfo[i].m_axmf4x4World[j];
				}
			}
		}
	}
		break;
	case HEAD_NUM_OF_CLIENT:
		nBufferSize = sizeof(int) + sizeof(m_aClientInfo);
		RecvNum++;
		nRetval = RecvData(wParam, nBufferSize);
		if (nRetval != 0)
		{
			break;
		}

		memcpy(&m_nClient, m_pCurrentBuffer, sizeof(int));
		memcpy(&m_aClientInfo, m_pCurrentBuffer + sizeof(int), sizeof(m_aClientInfo));
		for(int i=0;i<MAX_CLIENT;++i)
		{
			if (m_apPlayers[i])
			{
				m_apPlayers[i]->SetClientId(m_aClientInfo[i].m_nClientId);
			}
		}
		break;
	default:
		break;
	}

	if (nRetval != 0)
	{
		if (nRetval == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
		{
			m_bRecvHead = true;
			memset(m_pCurrentBuffer, 0, BUFSIZE);
		}
		return;
	}
	nHead = -1;
	m_bRecvHead = false;
	m_bRecvDelayed = false;
	memset(m_pCurrentBuffer, 0, BUFSIZE);

	return;
}

void CTcpClient::OnProcessingWriteMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	size_t nBufferSize = sizeof(int);
	int nHead;
	int nRetval;
	if (m_nMainClientId == -1  || m_bRecvDelayed == true)	// 아직 ID를 넘겨 받지 못했거나 딜레이 되었다.
	{
		return;
	}

	switch (m_socketState)
	{
	case SOCKET_STATE::SEND_KEY_BUFFER:
	{
		nHead = 0;
		UCHAR keysBuffer[256];
	
		std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();

		nBufferSize += sizeof(keysBuffer);
		UCHAR* pKeysBuffer = CGameFramework::GetKeysBuffer();
		if (pKeysBuffer != nullptr)
		{
			memcpy(keysBuffer, pKeysBuffer, nBufferSize - sizeof(int));
		}
		nBufferSize += sizeof(std::chrono::time_point<std::chrono::steady_clock>);
		nBufferSize += sizeof(XMFLOAT4X4);
		nBufferSize += sizeof(XMFLOAT3) * 2;		

		SendNum++;
		// 키버퍼, 카메라Matrix, LOOK,RIGHT 같이 보내주기
		nRetval = SendData(wParam, nBufferSize, 
							nHead,
							now,
							keysBuffer,
							m_apPlayers[m_nMainClientId]->GetCamera()->GetViewMatrix(),
							m_apPlayers[m_nMainClientId]->GetLook(), 
							m_apPlayers[m_nMainClientId]->GetRight());

		if (nRetval == -1 && WSAGetLastError() == WSAEWOULDBLOCK)
		{
		}
	}
		break;
	default:
		break;
	}
}

template<class... Args>
void CTcpClient::CreateSendDataBuffer(char* pBuffer, Args&&... args)
{
	size_t nOffset = 0;
	((memcpy(pBuffer + nOffset, &args, sizeof(args)), nOffset += sizeof(args)), ...);
}

template<class... Args>
int CTcpClient::SendData(SOCKET socket, size_t nBufferSize, Args&&... args)
{
	int nRetval;
	char* pBuffer = new char[nBufferSize];
	(CreateSendDataBuffer(pBuffer, args...));

	nRetval = send(socket, (char*)pBuffer, nBufferSize, 0);
	delete[] pBuffer;

	if (nRetval == SOCKET_ERROR)
	{
		return -1;
	}
	return 0;
}

int CTcpClient::RecvData(SOCKET socket, size_t nBufferSize)
{
	int nRetval;
	int nRemainRecvByte = nBufferSize - m_nCurrentRecvByte;

	nRetval = recv(socket, (char*)&m_pCurrentBuffer + m_nCurrentRecvByte, nRemainRecvByte, 0);
	if(nRetval > 0) m_nCurrentRecvByte += nRetval;
	if (nRetval == SOCKET_ERROR)
	{
		return SOCKET_ERROR;
	}
	else if (nRetval == 0)
	{
		return -2;
	}
	else if (m_nCurrentRecvByte < nBufferSize)
	{
		m_bRecvDelayed = true;
		return 1;
	}
	else
	{
		m_nCurrentRecvByte = 0;
		m_bRecvDelayed = false;
		return 0;
	}
}

void CTcpClient::UpdateZombiePlayer()
{
	shared_ptr<CZombiePlayer> pZombiePlayer = dynamic_pointer_cast<CZombiePlayer>(m_apPlayers[0]);
	if (!pZombiePlayer)
	{
		return;
	}

	if (m_aClientInfo[0].m_nSlotObjectNum[0] == 1)	// 추적
	{

	}

	if (m_aClientInfo[0].m_nSlotObjectNum[1] == 1)	// 시야방해
	{
		// 모든 플레이어의 안개효과 심화됨 ( 내 플레이어 안개만 조정하면 됨 )
		for (int i = 1; i < MAX_CLIENT; ++i)
		{
			if (m_apPlayers[i]->GetClientId() != m_nMainClientId)
			{
				continue;
			}
			dynamic_pointer_cast<CBlueSuitPlayer>(m_apPlayers[i])->SetInterruption(true);
		}
	}
	else
	{
		for (int i = 1; i < MAX_CLIENT; ++i)
		{
			if (m_apPlayers[i]->GetClientId() != m_nMainClientId)
			{
				continue;
			}
			dynamic_pointer_cast<CBlueSuitPlayer>(m_apPlayers[i])->SetInterruption(false);
		}
	}

	if (m_aClientInfo[0].m_nSlotObjectNum[2] == 1)	// 공격을 시도
	{
		pZombiePlayer->m_pSkinnedAnimationController->SetTrackEnable(2, true);
	}
}

void CTcpClient::UpdatePlayerItem(int nIndex)
{
	shared_ptr<CBlueSuitPlayer> pBlueSuitPlayer = dynamic_pointer_cast<CBlueSuitPlayer>(m_apPlayers[nIndex]);
	if (pBlueSuitPlayer)
	{
		for (int j = 0; j < 3; ++j)
		{
			if (m_aClientInfo[nIndex].m_nSlotObjectNum[j] != -1)
			{
				shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(m_aClientInfo[nIndex].m_nSlotObjectNum[j]).lock();
				shared_ptr<CItemObject> pItemObject = dynamic_pointer_cast<CItemObject>(pGameObject);
				pItemObject->SetObtain(true);
				pBlueSuitPlayer->SetSlotItem(j, m_aClientInfo[nIndex].m_nSlotObjectNum[j]);
			}
			else // -1을 받았는데 플레이어가 가진 Reference값이 -1이 아닌 경우를 생각해야함
			{
				if (pBlueSuitPlayer->GetReferenceSlotItemNum(j) != -1)
				{
					shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(pBlueSuitPlayer->GetReferenceSlotItemNum(j)).lock();
					shared_ptr<CItemObject> pItemObject = dynamic_pointer_cast<CItemObject>(pGameObject);
					pItemObject->SetObtain(false);
					pBlueSuitPlayer->SetSlotItemEmpty(j);
				}
			}
		}

		for (int j = 0; j < 3; ++j)
		{
			if (m_aClientInfo[nIndex].m_nFuseObjectNum[j] != -1)
			{
				shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(m_aClientInfo[nIndex].m_nFuseObjectNum[j]).lock();
				shared_ptr<CItemObject> pItemObject = dynamic_pointer_cast<CItemObject>(pGameObject);
				pItemObject->SetObtain(true);
				pBlueSuitPlayer->SetFuseItem(j, m_aClientInfo[nIndex].m_nFuseObjectNum[j]);
			}
			else // -1을 받았는데 플레이어가 가진 Reference값이 -1이 아닌 경우를 생각해야함
			{
				if (pBlueSuitPlayer->GetReferenceFuseItemNum(j) != -1)
				{
					shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(pBlueSuitPlayer->GetReferenceFuseItemNum(j)).lock();
					shared_ptr<CItemObject> pItemObject = dynamic_pointer_cast<CItemObject>(pGameObject);
					pItemObject->SetObtain(false);
					pBlueSuitPlayer->SetFuseItemEmpty(j);
				}
			}
		}
	}
}
