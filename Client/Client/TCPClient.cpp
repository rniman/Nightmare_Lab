#include "stdafx.h"
#include "TCPClient.h"
#include "GameFramework.h"
#include "Player.h"


CTcpClient::CTcpClient()
{
}

CTcpClient::~CTcpClient()
{
}

bool CTcpClient::CreateSocket(HWND hWnd, TCHAR* pszIPAddress)
{
	int nRetval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		err_quit("WSAStartup");
		return false;
	}

	// 소켓 생성
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	m_sock = s;
	//m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET)
	{
		err_quit("socket()");
		return false;	
	}

	char pIPAddress[20];
	ConvertLPWSTRToChar(pszIPAddress, pIPAddress, 20);

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	//inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	inet_pton(AF_INET, pIPAddress, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);

	//nRetval = connect(m_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	nRetval = connect(s, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
 	if (nRetval == SOCKET_ERROR) 
	{
		err_display("connect()");
		return false;
	}

	//nRetval = WSAAsyncSelect(m_sock, hWnd, WM_SOCKET, FD_CLOSE | FD_READ | FD_WRITE);	// FD_WRITE가 발생할것이다.
	nRetval = WSAAsyncSelect(s, hWnd, WM_SOCKET, FD_CLOSE | FD_READ | FD_WRITE);	// FD_WRITE가 발생할것이다.
	if (nRetval == SOCKET_ERROR)
	{
		return false;
	}

	return true;
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
	static INT8 nHead;
	int nRetval = 1;
	size_t nBufferSize;

	if(!m_bRecvHead)
	{
		nBufferSize = sizeof(INT8);
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
		memcpy(&nHead, m_pCurrentBuffer, sizeof(INT8));
		memset(m_pCurrentBuffer, 0, BUFSIZE);
	}

	switch (nHead)
	{
	case HEAD_GAME_START:
		PostMessage(hWnd, WM_START_GAME, 0, 0);
		m_socketState = SOCKET_STATE::SEND_KEY_BUFFER;
		break;
	case HEAD_INIT:
		//nBufferSize = sizeof(INT8) * 2;
		nBufferSize = sizeof(INT8) * 2 + sizeof(m_aClientInfo);
		RecvNum++;
		nRetval = RecvData(wParam, nBufferSize);
		if (nRetval != 0)
		{
			break;
		}

		memcpy(&m_nMainClientId, m_pCurrentBuffer, sizeof(INT8));
		memcpy(&m_nClient, m_pCurrentBuffer + sizeof(INT8), sizeof(INT8));
		memcpy(&m_aClientInfo, m_pCurrentBuffer + sizeof(INT8) * 2, sizeof(m_aClientInfo));

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

		UpdateDataFromServer();
	}
	break;
	case HEAD_NUM_OF_CLIENT:
		nBufferSize = sizeof(INT8) + sizeof(m_aClientInfo);
		RecvNum++;
		nRetval = RecvData(wParam, nBufferSize);
		if (nRetval != 0)
		{
			break;
		}

		memcpy(&m_nClient, m_pCurrentBuffer, sizeof(INT8));
		memcpy(&m_aClientInfo, m_pCurrentBuffer + sizeof(INT8), sizeof(m_aClientInfo));
		for (int i = 0;i < MAX_CLIENT;++i)
		{
			if (m_apPlayers[i])
			{
				m_apPlayers[i]->SetClientId(m_aClientInfo[i].m_nClientId);
			}
		}
		break;
	case HEAD_BLUE_SUIT_WIN:
		// 메시지?
		PostMessage(hWnd, WM_END_GAME, 0, 0);
		break;
	case HEAD_ZOMBIE_WIN:
		PostMessage(hWnd, WM_END_GAME, 1, 0);
		break;
	default:
		break;
	}

	if (nRetval != 0)
	{
		/*if (nRetval == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
		{
			m_bRecvHead = true;
			memset(m_pCurrentBuffer, 0, BUFSIZE);
		}*/
		return;
	}
	nHead = -1;
	m_bRecvHead = false;
	m_bRecvDelayed = false;
	memset(m_pCurrentBuffer, 0, BUFSIZE);

	return;
}

void CTcpClient::UpdateDataFromServer()
{
	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		if (m_apPlayers[i])
		{
			m_apPlayers[i]->SetAlive(m_aClientInfo[i].m_bAlive);
			m_apPlayers[i]->SetRunning(m_aClientInfo[i].m_bRunning);
			m_apPlayers[i]->SetClientId(m_aClientInfo[i].m_nClientId);
			m_apPlayers[i]->SetPosition(m_aClientInfo[i].m_xmf3Position);
			m_apPlayers[i]->SetVelocity(m_aClientInfo[i].m_xmf3Velocity);
			if (i != m_nMainClientId) {
				m_apPlayers[i]->SetPitch(m_aClientInfo[i].m_animationInfo.pitch);
			}


			if (i != m_nMainClientId)
			{
				m_apPlayers[i]->SetLook(m_aClientInfo[i].m_xmf3Look);
				XMFLOAT3 xmf3Right = XMFLOAT3(0.0f, 1.0f, 0.0f);
				xmf3Right = Vector3::CrossProduct(xmf3Right, m_aClientInfo[i].m_xmf3Look, true);
				m_apPlayers[i]->SetRight(xmf3Right);
				//m_apPlayers[i]->SetRight(m_aClientInfo[i].m_xmf3Right);
			}

			//[0523] 피킹 오브젝트 설정(외곽선 작업에 필요)
			UpdatePickedObject(i);

				// 지뢰 충돌
				int nObjectNum = m_aClientInfo[i].m_playerInfo.m_iMineobjectNum;
				if (nObjectNum >= 0) {
					shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(nObjectNum).lock();
					auto mine = dynamic_pointer_cast<CMineObject>(pGameObject);
					if (mine)
					{
						mine->SetCollide(true);
						shared_ptr<CZombiePlayer> pZombiePlayer = dynamic_pointer_cast<CZombiePlayer>(m_apPlayers[i]);
						if (pZombiePlayer) 
						{
							pZombiePlayer->SetEectricShock();
						}
					}
				}
			}

		if (i == ZOMBIEPLAYER)
		{
			UpdateZombiePlayer();
		}
		else
		{
			UpdatePlayer(i);
		}

		int nNumOfGameObject = m_aClientInfo[i].m_nNumOfObject;
		for (int j = 0; j < nNumOfGameObject; ++j)
		{
			int nObjectNum = m_aClientInfo[i].m_anObjectNum[j];


				if (nObjectNum <= -1 || nObjectNum >= g_collisionManager.GetNumOfCollisionObject())
				{
					continue;
				}
#ifdef LOADSCENE
			shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(nObjectNum).lock();
			if (pGameObject)
			{
				pGameObject->m_xmf4x4World = m_aClientInfo[i].m_axmf4x4World[j];
				pGameObject->m_xmf4x4ToParent = m_aClientInfo[i].m_axmf4x4World[j];
			}
#endif LOADSCENE
		}
	}
}

void CTcpClient::UpdatePickedObject(int i)
{
	if (i == m_nMainClientId)
	{
		if (m_aClientInfo[i].m_nPickedObjectNum == -1)
		{
			m_apPlayers[i]->SetPickedObject(nullptr);
		}
		else
		{
			int nObjectNum = m_aClientInfo[i].m_nPickedObjectNum;
			shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(nObjectNum).lock();
			if (pGameObject)
			{
				m_apPlayers[i]->SetPickedObject(pGameObject);
			}
		}
	}
}

void CTcpClient::OnProcessingWriteMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	size_t nBufferSize = sizeof(INT8);
	INT8 nHead;
	int nRetval;
	if (m_nMainClientId == -1 || m_bRecvDelayed == true || !m_apPlayers[m_nMainClientId])	// 아직 ID를 넘겨 받지 못했거나 딜레이 되었다.
	{
		return;
	}
	
	//데이터 갱신 후 전송
	m_aClientInfo[m_nMainClientId].m_animationInfo.pitch = m_apPlayers[m_nMainClientId]->GetPitch();
	m_aClientInfo[m_nMainClientId].m_playerInfo.m_bRightClick = m_apPlayers[m_nMainClientId]->IsRightClick();
	m_apPlayers[m_nMainClientId]->SetRightClick(false);

	switch (m_socketState)
	{
	case SOCKET_STATE::SEND_GAME_START:
		nHead = 1;
		nRetval = SendData(m_sock, nBufferSize, nHead);

		if (nRetval == -1 && WSAGetLastError() == WSAEWOULDBLOCK)
		{
		}
		break;
	case SOCKET_STATE::SEND_KEY_BUFFER:
	{
		nHead = 0;
		UCHAR keysBuffer[256];

		UCHAR* pKeysBuffer = CGameFramework::GetKeysBuffer();
		WORD wKeyBuffer = 0;
		UpdateKeyBitMask(pKeysBuffer, wKeyBuffer);
		
		std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();

		nBufferSize += sizeof(WORD);

		//nBufferSize += sizeof(keysBuffer);
		//if (pKeysBuffer != nullptr)
		//{
		//	memcpy(keysBuffer, pKeysBuffer, nBufferSize - sizeof(int));
		//}
		nBufferSize += sizeof(std::chrono::time_point<std::chrono::steady_clock>);
		nBufferSize += sizeof(XMFLOAT4X4);
		nBufferSize += sizeof(XMFLOAT3) * 3;
		nBufferSize += sizeof(CS_ANIMATION_INFO);
		nBufferSize += sizeof(CS_PLAYER_INFO);

		SendNum++;
		// 키버퍼, 카메라Matrix, LOOK,RIGHT 같이 보내주기
		if(m_apPlayers[m_nMainClientId]->m_pSkinnedAnimationController->IsAnimation())
		{
			nRetval = SendData(wParam, nBufferSize,
				nHead,
				now,
				wKeyBuffer,
				m_apPlayers[m_nMainClientId]->GetCamera()->GetViewMatrix(),
				m_apPlayers[m_nMainClientId]->GetLook(),
				m_apPlayers[m_nMainClientId]->GetRight(),
				m_apPlayers[m_nMainClientId]->GetUp(),
				m_aClientInfo[m_nMainClientId].m_animationInfo,
				m_aClientInfo[m_nMainClientId].m_playerInfo
			);
		}
		else
		{
			nRetval = SendData(wParam, nBufferSize,
				nHead,
				now,
				wKeyBuffer,
				m_apPlayers[m_nMainClientId]->GetCamera()->GetViewMatrix(),
				m_apPlayers[m_nMainClientId]->GetCamera()->GetLookVector(),
				m_apPlayers[m_nMainClientId]->GetCamera()->GetRightVector(),
				m_apPlayers[m_nMainClientId]->GetCamera()->GetUpVector(),
				m_aClientInfo[m_nMainClientId].m_animationInfo,
				m_aClientInfo[m_nMainClientId].m_playerInfo
			);
		}

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

void CTcpClient::UpdateKeyBitMask(UCHAR* pKeysBuffer, WORD& wKeyBuffer)	// 보낼 키 버퍼를 업데이트
{
	if (pKeysBuffer['W'] & 0xF0)wKeyBuffer |= KEY_W;
	if (pKeysBuffer['S'] & 0xF0)wKeyBuffer |= KEY_S;
	if (pKeysBuffer['A'] & 0xF0)wKeyBuffer |= KEY_A;
	if (pKeysBuffer['D'] & 0xF0)wKeyBuffer |= KEY_D;
	if (pKeysBuffer['1'] & 0xF0)wKeyBuffer |= KEY_1;
	if (pKeysBuffer['2'] & 0xF0)wKeyBuffer |= KEY_2;
	if (pKeysBuffer['3'] & 0xF0)wKeyBuffer |= KEY_3;
	if (pKeysBuffer['4'] & 0xF0)wKeyBuffer |= KEY_4;
	if (pKeysBuffer['E'] & 0xF0)wKeyBuffer |= KEY_E;
	if (pKeysBuffer[VK_LSHIFT] & 0xF0)wKeyBuffer |= KEY_LSHIFT;
	if (pKeysBuffer[VK_LBUTTON] & 0xF0)wKeyBuffer |= KEY_LBUTTON;
	if (pKeysBuffer[VK_RBUTTON] & 0xF0)wKeyBuffer |= KEY_RBUTTON;
}

void CTcpClient::UpdateZombiePlayer()
{
	shared_ptr<CZombiePlayer> pZombiePlayer = dynamic_pointer_cast<CZombiePlayer>(m_apPlayers[0]);
	if (!pZombiePlayer)
	{
		return;
	}

	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		if (m_nMainClientId == ZOMBIEPLAYER)	// 추적
		{
			if (m_aClientInfo[0].m_nSlotObjectNum[0] == 1)
			{
				m_apPlayers[i]->SetTracking(true);
			}
			else
			{
				m_apPlayers[i]->SetTracking(false);
			}
		}

		if (m_apPlayers[i]->GetClientId() != m_nMainClientId || i == ZOMBIEPLAYER)
		{
			continue;
		}
		if (m_aClientInfo[0].m_nSlotObjectNum[1] == 1)
		{
			m_apPlayers[i]->SetInterruption(true);
		}
		else
		{
			m_apPlayers[i]->SetInterruption(false);
		}
	}

	// 시야 방해(zombie 플레이어)
	if (m_nMainClientId == ZOMBIEPLAYER)
	{
		if (m_aClientInfo[0].m_nSlotObjectNum[1] == 1)
		{
			m_apPlayers[0]->SetInterruption(true);
		}
		else
		{
			m_apPlayers[0]->SetInterruption(false);
		}
	}

	if (m_aClientInfo[0].m_nSlotObjectNum[2] == 1)	// 공격을 시도
	{
		pZombiePlayer->m_pSkinnedAnimationController->SetTrackEnable(2, true);
	}
}

void CTcpClient::UpdatePlayer(int nIndex)
{
	shared_ptr<CBlueSuitPlayer> pBlueSuitPlayer = dynamic_pointer_cast<CBlueSuitPlayer>(m_apPlayers[nIndex]);

	if (!pBlueSuitPlayer) // 생존자가 아니면 수행 x
	{
		return;
	}

	if (m_nEscapeDoor == -1) m_nEscapeDoor = m_aClientInfo[nIndex].m_playerInfo.m_iEscapeDoor;
	if (m_nEscapeDoor != -1) {
		shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(m_nEscapeDoor).lock();
		pBlueSuitPlayer->SetEscapePos(pGameObject->GetPosition());
	}

	pBlueSuitPlayer->SelectItem(m_aClientInfo[nIndex].m_playerInfo.m_selectItem);
	for (int j = 0; j < 3; ++j)
	{
		if (m_aClientInfo[nIndex].m_nSlotObjectNum[j] != -1)
		{
			shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(m_aClientInfo[nIndex].m_nSlotObjectNum[j]).lock();
			shared_ptr<CItemObject> pItemObject = dynamic_pointer_cast<CItemObject>(pGameObject);
			if (pItemObject) {
				pItemObject->SetObtain(true);
				pBlueSuitPlayer->SetSlotItem(j, m_aClientInfo[nIndex].m_nSlotObjectNum[j]);
			}
		}
		else // -1을 받았는데 플레이어가 가진 Reference값이 -1이 아닌 경우를 생각해야함
		{
			if (pBlueSuitPlayer->GetReferenceSlotItemNum(j) != -1)
			{
				shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(pBlueSuitPlayer->GetReferenceSlotItemNum(j)).lock();
				shared_ptr<CItemObject> pItemObject = dynamic_pointer_cast<CItemObject>(pGameObject);
				if (pItemObject) {
					pItemObject->SetObtain(false);
					pBlueSuitPlayer->SetSlotItemEmpty(j);
				}
			}
		}
	}

	for (int j = 0; j < 3; ++j)
	{
		if (m_aClientInfo[nIndex].m_nFuseObjectNum[j] != -1)
		{
			shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(m_aClientInfo[nIndex].m_nFuseObjectNum[j]).lock();
			shared_ptr<CItemObject> pItemObject = dynamic_pointer_cast<CItemObject>(pGameObject);
			if (pItemObject) {
				pItemObject->SetObtain(true);
				pBlueSuitPlayer->SetFuseItem(j, m_aClientInfo[nIndex].m_nFuseObjectNum[j]);
			}
		}
		else // -1을 받았는데 플레이어가 가진 Reference값이 -1이 아닌 경우를 생각해야함
		{
			if (pBlueSuitPlayer->GetReferenceFuseItemNum(j) != -1)
			{
				shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(pBlueSuitPlayer->GetReferenceFuseItemNum(j)).lock();
				shared_ptr<CItemObject> pItemObject = dynamic_pointer_cast<CItemObject>(pGameObject);
				if (pItemObject) {
					pItemObject->SetObtain(false);
					pBlueSuitPlayer->SetFuseItemEmpty(j);
				}
			}
		}
	}

	if (m_aClientInfo[nIndex].m_playerInfo.m_bAttacked) {
		pBlueSuitPlayer->SetHitEvent();
	}
}

void ConvertLPWSTRToChar(LPWSTR lpwstr, char* dest, int destSize)
{
	// WideCharToMultiByte 함수를 사용하여 LPWSTR을 char*로 변환
	WideCharToMultiByte(
		CP_UTF8,
		0,                   // 변환 옵션
		lpwstr,              // 변환할 유니코드 문자열
		-1,                  // 자동으로 문자열 길이 계산
		dest,                // 대상 버퍼
		destSize,            // 대상 버퍼의 크기
		NULL,                // 기본 문자 사용 안 함
		NULL                 // 기본 문자 사용 여부를 저장할 변수의 주소
	);
}

void ConvertCharToLPWSTR(const char* pstr, LPWSTR dest, int destSize)
{
	// MultiByteToWideChar 함수를 사용하여 char*을 LPWSTR로 변환
	MultiByteToWideChar(
		CP_UTF8,
		0,                   // 변환 옵션
		pstr,                 // 변환할 문자열
		-1,                  // 자동으로 문자열 길이 계산
		dest,                // 대상 버퍼
		destSize             // 대상 버퍼의 크기
	);
}

