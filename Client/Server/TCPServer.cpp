#include "stdafx.h"
#include "TCPServer.h"
#include "ServerObject.h"
#include "ServerEnvironmentObject.h"
#include "ServerPlayer.h"
#include "ServerCollision.h"

default_random_engine TCPServer::m_mt19937Gen;
INT8 TCPServer::m_nClient = 0;

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

TCPServer::TCPServer()
{
	m_axmf3Positions = {
		XMFLOAT3(10.0f, 0.0f, 13.5),
		XMFLOAT3(10.0f, 0.0f, -13.5),
		XMFLOAT3(-10.0f, 0.0f, 13.5),
		XMFLOAT3(-10.0f, 0.0f, -13.5),

		XMFLOAT3(10.0f, 4.5f, 13.5),
		//XMFLOAT3(10.0f, 4.5f, -13.5),
		//XMFLOAT3(-10.0f, 4.5f, 13.5),
		//XMFLOAT3(-10.0f, 4.5f, -13.5),

		//XMFLOAT3(10.0f, 9.0f, 13.5),
		//XMFLOAT3(10.0f, 9.0f, -13.5),
		//XMFLOAT3(-10.0f, 9.0f, 13.5),
		//XMFLOAT3(-10.0f, 9.0f, -13.5),

		//XMFLOAT3(10.0f, 13.5f, 13.5),
		//XMFLOAT3(10.0f, 13.5f, -13.5),
		//XMFLOAT3(-10.0f, 13.5f, 13.5),
		//XMFLOAT3(-10.0f, 13.5f, -13.5)
	};

	m_anPlayerStartPosNum = { -1, -1, -1, -1, -1 };
}

TCPServer::~TCPServer()
{
}

void TCPServer::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_CREATE:
		m_timer.Start();
		break;
	default:
		break;
	}
}

void TCPServer::OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_ACCEPT:
		OnProcessingAcceptMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case FD_READ:
		OnProcessingReadMessage(hWnd, nMessageID, wParam, lParam);
	case FD_WRITE:
		OnProcessingWriteMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case FD_CLOSE:
		OnProcessingCloseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	default:
		break;
	}

	return;
}

void TCPServer::OnProcessingAcceptMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	
	SOCKET sockClient;
	struct sockaddr_in addrClient;
	int nAddrlen = sizeof(sockaddr_in);
	sockClient = accept(wParam, (struct sockaddr*)&addrClient, &nAddrlen);

	if (sockClient == INVALID_SOCKET)
	{
		err_display("accept()");
		return;
	}

	// 추가된 클라이언트의 정보를 추가한다.
	INT8 nSocketIndex = AddSocketInfo(sockClient, addrClient, nAddrlen);
	
	// MAX_CLIENT보다 더 많은 접속 요구
	if (nSocketIndex == -1)	
	{
		closesocket(sockClient); // 클라이언트 소켓 종료
		err_display("Maximum number of clients reached. Connection refused."); // 연결 거부 메시지 표시
		return;
	}
	printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n", m_vSocketInfoList[nSocketIndex].m_pAddr, ntohs(m_vSocketInfoList[nSocketIndex].m_addrClient.sin_port));

	int retval = WSAAsyncSelect(sockClient, hWnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
	if (retval == SOCKET_ERROR)
	{
		err_display("WSAAsyncSelect()");
		RemoveSocketInfo(sockClient);
	}
	WCHAR pszList[256];
	WCHAR pszIP[16];
	ConvertCharToLPWSTR(m_vSocketInfoList[nSocketIndex].m_pAddr, pszIP, 16);
	wsprintf(pszList, L"CLIENT[%d], IP: %s, 포트 번호: %d\n", nSocketIndex, pszIP, ntohs(m_vSocketInfoList[nSocketIndex].m_addrClient.sin_port));
	SendMessage(m_hClientListBox, LB_ADDSTRING, 0, (LPARAM)pszList);

	// 임시로 CBlueSuitPlayer만 생성
	if (nSocketIndex == ZOMBIEPLAYER)	// Socket Index가 0이면 Zombie
	{
		m_apPlayers[nSocketIndex] = make_shared<CServerZombiePlayer>();
		m_apPlayers[nSocketIndex]->SetPlayerId(nSocketIndex);
	}
	else
	{
		m_apPlayers[nSocketIndex] = make_shared<CServerBlueSuitPlayer>();
		m_apPlayers[nSocketIndex]->SetPlayerId(nSocketIndex);
	}

	InitPlayerPosition(m_apPlayers[nSocketIndex], nSocketIndex);

	m_pCollisionManager->AddCollisionPlayer(m_apPlayers[nSocketIndex], nSocketIndex);

	for (auto& sockInfo : m_vSocketInfoList)
	{
		if (!sockInfo.m_bUsed || sockInfo.m_sock == sockClient)
		{
			continue;
		}
		sockInfo.m_socketState = SOCKET_STATE::SEND_NUM_OF_CLIENT;
	}

	return;
}

void TCPServer::OnProcessingReadMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	int nRetval = 1;
	size_t nBufferSize;
	int nSocketIndex = GetSocketIndex(wParam);
	if (!m_vSocketInfoList[nSocketIndex].m_bUsed)
	{
		//error
		exit(-1);
	}
	std::shared_ptr<CServerPlayer> pPlayer = m_apPlayers[nSocketIndex];

	if(!m_vSocketInfoList[nSocketIndex].m_bRecvHead)
	{
		nBufferSize = sizeof(INT8);

		nRetval = RecvData(nSocketIndex, nBufferSize);
		if (nRetval == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				m_vSocketInfoList[nSocketIndex].m_bRecvHead = false;
				m_vSocketInfoList[nSocketIndex].m_nHead = -1;
				memset(m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer, 0, BUFSIZE);
			}
			return;
		}
		m_vSocketInfoList[nSocketIndex].m_bRecvHead = true;
		memcpy(&m_vSocketInfoList[nSocketIndex].m_nHead, m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer, sizeof(INT8));
		memset(m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer, 0, BUFSIZE);
	}

	switch (m_vSocketInfoList[nSocketIndex].m_nHead)
	{
	case HEAD_KEYS_BUFFER:
	{
		if (!pPlayer->IsRecvData())
		{
			pPlayer->SetRecvData(true);
		}
		 
		std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
		std::chrono::time_point<std::chrono::steady_clock> client;

		// Time, KeysBuffer(WORD), viewMatrix, vecLook, vecRight
		nBufferSize = sizeof(__int64) + sizeof(WORD) + sizeof(XMFLOAT4X4) + sizeof(XMFLOAT3) * 3 + sizeof(SC_ANIMATION_INFO) + sizeof(SC_PLAYER_INFO);
		m_vSocketInfoList[nSocketIndex].RecvNum++;
		nRetval = RecvData(nSocketIndex, nBufferSize);
		if (nRetval == SOCKET_ERROR)
		{
			break;
		}

		int sizeOffset = 0;
		memcpy(&client, m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer + sizeOffset, sizeof(std::chrono::time_point<std::chrono::steady_clock>));
		std::chrono::duration<double> deltaTime = now - client;
		sizeOffset += sizeof(__int64);

		WORD wKeyBuffer = 0;
		memcpy(&wKeyBuffer, m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer + sizeOffset, sizeof(WORD));
		pPlayer->SetKeyBuffer(wKeyBuffer);

		sizeOffset += sizeof(WORD);

		XMFLOAT4X4 xmf4x4View;
		memcpy(&xmf4x4View, m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer + sizeOffset, sizeof(XMFLOAT4X4));
		pPlayer->SetViewMatrix(xmf4x4View);
		sizeOffset += sizeof(XMFLOAT4X4);

		XMFLOAT3 xmf3Look, xmf3Right, xmf3Up;
		memcpy(&xmf3Look, m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer + sizeOffset, sizeof(XMFLOAT3));
		sizeOffset += sizeof(XMFLOAT3);
		memcpy(&xmf3Right, m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer + sizeOffset, sizeof(XMFLOAT3));
		sizeOffset += sizeof(XMFLOAT3);
		memcpy(&xmf3Up, m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer + sizeOffset, sizeof(XMFLOAT3));
		sizeOffset += sizeof(XMFLOAT3);

		pPlayer->SetLook(xmf3Look);
		pPlayer->SetRight(xmf3Right);
		pPlayer->SetUp(xmf3Up);

		SC_ANIMATION_INFO animationInfo;
		memcpy(&animationInfo, m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer + sizeOffset, sizeof(SC_ANIMATION_INFO)); 
		m_aUpdateInfo[nSocketIndex].m_animationInfo = animationInfo;
		sizeOffset += sizeof(SC_ANIMATION_INFO);
		
		SC_PLAYER_INFO playerInfo;
		memcpy(&playerInfo, m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer + sizeOffset, sizeof(SC_PLAYER_INFO));
		pPlayer->SetRightClick(playerInfo.m_bRightClick);
	}
		break;
	default:
		break;
	}

	if (nRetval != 0)
	{
		//if (WSAGetLastError() == WSAEWOULDBLOCK)
		//{
		//	m_vSocketInfoList[nSocketIndex].m_bRecvHead = true;
		//	memset(m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer, 0, BUFSIZE);
		//}
		return;
	}
	m_vSocketInfoList[nSocketIndex].m_nHead = -1;
	m_vSocketInfoList[nSocketIndex].m_bRecvHead = false;
	m_vSocketInfoList[nSocketIndex].m_bRecvDelayed = false;
	memset(m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer, 0, BUFSIZE);
	return;
}

void TCPServer::OnProcessingWriteMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	size_t nBufferSize = sizeof(INT8);
	INT8 nHead;
	int nRetval;
	int nSocketIndex = GetSocketIndex(wParam);
	if (!m_vSocketInfoList[nSocketIndex].m_bUsed)
	{
		//error
	}
	std::shared_ptr<CServerPlayer> pPlayer = m_apPlayers[nSocketIndex];

	switch (m_vSocketInfoList[nSocketIndex].m_socketState)
	{
	case SOCKET_STATE::SEND_ID:
		nHead = 0;
		nBufferSize += sizeof(INT8) * 2;

		m_vSocketInfoList[nSocketIndex].SendNum++;
		nRetval = SendData(m_vSocketInfoList[nSocketIndex].m_sock, nBufferSize, nHead, m_aUpdateInfo[nSocketIndex].m_nClientId, m_nClient);

		if (nRetval == -1 && WSAGetLastError() == WSAEWOULDBLOCK)
		{
		}
		m_vSocketInfoList[nSocketIndex].m_socketState = SOCKET_STATE::SEND_UPDATE_DATA;

		break;
	case SOCKET_STATE::SEND_UPDATE_DATA:
	{
		nHead = 1;
		nBufferSize += sizeof(m_aUpdateInfo);

		m_vSocketInfoList[nSocketIndex].SendNum++;
		nRetval = SendData(m_vSocketInfoList[nSocketIndex].m_sock, nBufferSize, nHead, m_aUpdateInfo);

		m_bDataSend[nSocketIndex] = true;
		if (nRetval == -1 && WSAGetLastError() == WSAEWOULDBLOCK)
		{
		}
		}
		break;
	case SOCKET_STATE::SEND_NUM_OF_CLIENT:
		nHead = 2;
		nBufferSize += sizeof(INT8) + sizeof(m_aUpdateInfo);

		nRetval = SendData(m_vSocketInfoList[nSocketIndex].m_sock, nBufferSize, nHead, m_nClient, m_aUpdateInfo);
		if (nRetval == -1 && WSAGetLastError() == WSAEWOULDBLOCK)
		{
		}
		
		m_vSocketInfoList[nSocketIndex].m_socketState = SOCKET_STATE::SEND_UPDATE_DATA;
		break;
	default:
		break;
	}
	return;
}

void TCPServer::OnProcessingCloseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	INT8 nIndex = RemoveSocketInfo((SOCKET)wParam);
	m_apPlayers[nIndex].reset();
	m_anPlayerStartPosNum[nIndex] = -1;
	//m_apPlayers[nIndex]->SetPlayerId(-1);
}

bool TCPServer::Init(HWND hWnd)
{
	m_mt19937Gen = default_random_engine(random_device()());

	m_hWnd = hWnd;
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		return false;
	}

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	int retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		err_quit("listen()");
	}

	// WSAAsyncSelect()
	retval = WSAAsyncSelect(listen_sock, hWnd, WM_SOCKET, FD_ACCEPT | FD_CLOSE);
	if (retval == SOCKET_ERROR)
	{
		err_quit("WSAAsyncSelect()");
	}

	m_pCollisionManager = make_shared<CServerCollisionManager>();
	m_pCollisionManager->CreateCollision(SPACE_FLOOR, SPACE_WIDTH, SPACE_DEPTH);

	// 씬 생성
	LoadScene();
	int ELEVATORDOORCOUNT = 16;
	int start_door{ -1 };
	for (int i = 0; i < m_pCollisionManager->GetNumberOfCollisionObject();++i) {
		shared_ptr<CServerGameObject> object = m_pCollisionManager->GetCollisionObjectWithNumber(i);
		auto pElevaterDoor = dynamic_pointer_cast<CServerElevatorDoorObject>(object);
		if (!pElevaterDoor) {
			continue;
		}
		if (strcmp(pElevaterDoor->m_pstrFrameName, "Door1")) {
			continue;
		}
		if (pElevaterDoor) {
			start_door = i;
			break;
		}
	}

	int random_escape_index = rand() % ELEVATORDOORCOUNT;
	for (int i = 0; i < ELEVATORDOORCOUNT;++i) {
		shared_ptr<CServerGameObject> object = m_pCollisionManager->GetCollisionObjectWithNumber(start_door + i);
		auto pElevaterDoor = dynamic_pointer_cast<CServerElevatorDoorObject>(object);
		if (!pElevaterDoor) {
			//std::cout << "엘리베이터 문이 아닙니다.!" << std::endl;
			assert(0); //반드시 CServerElevatorDoorObject 일것임. 아니면 시스템 종료 씬 오브젝트 정렬의 문제 발생
		}
		
		if (i == random_escape_index) {
			pElevaterDoor->SetEscapeDoor(true);
			for (int pi = 0; pi < MAX_CLIENT;++pi) {
				m_aUpdateInfo[pi].m_playerInfo.m_iEscapeDoor = start_door + i;
			}
		}
		//pElevaterDoor->SetEscapeDoor(false); // 디버그를 위해서 모든 문을 잠금
	}

	// 아이템 생성
	CreateItemObject();

	std::cout << "생성된 충돌객체 = " << m_pCollisionManager->GetNumberOfCollisionObject() << std::endl;
	//shared_ptr<CServerGameObject> object = m_pCollisionManager->GetCollisionObjectWithNumber(932);

	return true;
}

void TCPServer::SimulationLoop()
{
	m_timer.Tick();
	// 실제 시뮬레이션이 일어날곳
	float fElapsedTime = m_timer.GetTimeElapsed();
	for (auto& pPlayer : m_apPlayers)
	{
		if (!pPlayer || pPlayer->GetPlayerId() == -1)
		{
			continue;
		}

		pPlayer->SetPickedObject(m_pCollisionManager);	
		//if (pPlayer->GetPickedObject().lock())
		//{
		//	printf("%s\n", pPlayer->GetPickedObject().lock()->GetFrameName());
		//}

		pPlayer->RightClickProcess(m_pCollisionManager);
		pPlayer->UseItem(m_pCollisionManager);
		pPlayer->Update(fElapsedTime, m_pCollisionManager);
		pPlayer->UpdatePicking();
		//UpdateInformation(pPlayer);
		m_pCollisionManager->Collide(fElapsedTime, pPlayer);

		pPlayer->OnUpdateToParent();
	}

	m_pCollisionManager->Update(fElapsedTime);

	UpdateInformation();
	CreateSendObject();
		}

// 소켓 정보 추가
INT8 TCPServer::AddSocketInfo(SOCKET sockClient, struct sockaddr_in addrClient, int nAddrLen)
{
	INT8 nSocketIndex = -1;
	if (m_nClient >= MAX_CLIENT)
	{
		return nSocketIndex;
	}
	SOCKETINFO sockInfo;

	sockInfo.m_bUsed = true;
	sockInfo.m_sock = sockClient;
	sockInfo.m_addrClient = addrClient;
	sockInfo.m_nAddrlen = nAddrLen;

	getpeername(sockInfo.m_sock, (struct sockaddr*)&sockInfo.m_addrClient, &sockInfo.m_nAddrlen);
	inet_ntop(AF_INET, &sockInfo.m_addrClient.sin_addr, sockInfo.m_pAddr, sizeof(sockInfo.m_pAddr));

	sockInfo.m_nCurrentRecvByte = 0;
	sockInfo.m_bRecvDelayed = false;
	sockInfo.m_bRecvHead = false;
	sockInfo.m_socketState = SOCKET_STATE::SEND_ID;
	//sockInfo.m_prevSocketState = SOCKET_STATE::SEND_ID;
	
	// 배열에 정보 추가 
	for (int i = 0; i < m_nClient + 1; ++i)
	{
		if (m_vSocketInfoList[i].m_bUsed)
		{
			continue;
		}
		m_nClient++;

		// 클라이언트 정보 초기화
		m_aUpdateInfo[i].m_nClientId = i;
		m_vSocketInfoList[i] = sockInfo;
		nSocketIndex = i;
		break;
	}

	return nSocketIndex;
}

// 소켓 정보 얻기
INT8 TCPServer::GetSocketIndex(SOCKET sock)
{
	INT8 nIndex = -1;
	for (auto& sockInfo : m_vSocketInfoList)
	{
		nIndex++;
		if (!sockInfo.m_bUsed)
		{
			continue;
		}

		if (sockInfo.m_sock == sock)
		{
			return nIndex;
		}
	}
	return nIndex;
}

// 소켓 정보 제거
INT8 TCPServer::RemoveSocketInfo(SOCKET sock)
{
	INT8 nIndex = -1;
	INT8 nListBoxIndex = -1;
	// 리스트에서 정보 제거
	for (auto& sockInfo : m_vSocketInfoList)
	{
		nIndex++;
		if (!sockInfo.m_bUsed)
		{
			continue;
		}
		else
		{
			nListBoxIndex++;
		}

		if (sockInfo.m_sock == sock)
		{
			printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n", sockInfo.m_pAddr, ntohs(sockInfo.m_addrClient.sin_port));

			SendMessage(m_hClientListBox, LB_DELETESTRING, (WPARAM)nListBoxIndex, 0);

			closesocket(sockInfo.m_sock); // 소켓 닫기
			sockInfo.m_bUsed = false;
			
			m_aUpdateInfo[nIndex].m_nClientId = -1;
			m_nClient--;
			for(auto& otherSocketInfo : m_vSocketInfoList)
			{
				if (!otherSocketInfo.m_bUsed)
				{
					continue;
				}

				//otherSocketInfo.m_prevSocketState = otherSocketInfo.m_socketState;
				otherSocketInfo.m_socketState = SOCKET_STATE::SEND_NUM_OF_CLIENT;
			}

			return nIndex;
		}
	}
	return -1;
}

int TCPServer::CheckAllClientsSentData(int cur_nPlayer)
{
	int sendClientCount{};
	for (int i = 0; i < cur_nPlayer;++i) {
		if (m_bDataSend[i]) {
			sendClientCount++;
		}
	}
	return sendClientCount;
}

void TCPServer::SetAllClientsSendStatus(int cur_nPlayer,bool val)
{
	for (int i = 0; i < cur_nPlayer;++i) {
		m_bDataSend[i] = val;
	}
}

void TCPServer::UpdateInformation()
{
	int cur_nPlayer{};
	for (const auto& pPlayer : m_apPlayers)
	{
		if (!pPlayer || pPlayer->GetPlayerId() == -1)
		{
			continue;
		}
		++cur_nPlayer;
	}

	for (const auto& pPlayer : m_apPlayers)
	{
		INT8 nPlayerId;
		if (!pPlayer || pPlayer->GetPlayerId() == -1)
		{
			continue;
		}
		nPlayerId = pPlayer->GetPlayerId();

		m_aUpdateInfo[nPlayerId].m_bAlive = pPlayer->IsAlive();
		m_aUpdateInfo[nPlayerId].m_bRunning = pPlayer->IsRunning();
		m_aUpdateInfo[nPlayerId].m_xmf3Position = pPlayer->GetPosition();
		m_aUpdateInfo[nPlayerId].m_xmf3Velocity = pPlayer->GetVelocity();
		m_aUpdateInfo[nPlayerId].m_xmf3Look = pPlayer->GetLook();
		// 지금은 일단 이렇게 해뒀지만 나중에는 0번이 Enemy고정일듯
		if (nPlayerId == ZOMBIEPLAYER)	//Enemy
		{
			shared_ptr<CServerZombiePlayer> pZombiePlayer = dynamic_pointer_cast<CServerZombiePlayer>(pPlayer);
			if (pZombiePlayer) {
				m_aUpdateInfo[nPlayerId].m_nSlotObjectNum[0] = pZombiePlayer->IsTracking() ? 1 : -1;		// 추적
				m_aUpdateInfo[nPlayerId].m_nSlotObjectNum[1] = pZombiePlayer->IsInterruption() ? 1 : -1;	// 시야방해
				m_aUpdateInfo[nPlayerId].m_nSlotObjectNum[2] = pZombiePlayer->IsAttack() ? 1 : -1;			// 공격

				m_aUpdateInfo[nPlayerId].m_playerInfo.m_iMineobjectNum = pZombiePlayer->GetCollideMineRef();
				// 지뢰충돌에 대한 데이터 로직
				if (m_aUpdateInfo[nPlayerId].m_playerInfo.m_iMineobjectNum == -1) {
					m_aUpdateInfo[nPlayerId].m_playerInfo.m_iMineobjectNum = pZombiePlayer->GetCollideMineRef();
					pZombiePlayer->expDelay = 0.0f;
				} 
				else {
					if (pZombiePlayer->expDelay > 0.05f) {
						pZombiePlayer->SetCollideMineRef(-1);
						m_aUpdateInfo[nPlayerId].m_playerInfo.m_iMineobjectNum = pZombiePlayer->GetCollideMineRef();
					}
				}
			}
		}
		else
		{
			shared_ptr<CServerBlueSuitPlayer> pBlueSuitPlayer = dynamic_pointer_cast<CServerBlueSuitPlayer>(pPlayer);
			if (pBlueSuitPlayer)
			{
				for (int i = 0; i < 3; ++i)
				{
					m_aUpdateInfo[nPlayerId].m_nSlotObjectNum[i] = pBlueSuitPlayer->GetReferenceSlotItemNum(i);
					m_aUpdateInfo[nPlayerId].m_nFuseObjectNum[i] = pBlueSuitPlayer->GetReferenceFuseItemNum(i);
				}
				m_aUpdateInfo[nPlayerId].m_playerInfo.m_bAttacked = pBlueSuitPlayer->IsAttacked();
				m_aUpdateInfo[nPlayerId].m_playerInfo.m_selectItem = pBlueSuitPlayer->GetRightItem();
			}
			
		}
		// 업데이트 오브젝트는 리셋
		m_aUpdateInfo[nPlayerId].m_nNumOfObject = 0;
		for (int i = 0; i < MAX_SEND_OBJECT_INFO; ++i)
		{
			m_aUpdateInfo[nPlayerId].m_anObjectNum[i] = -1;
		}
	}
}

void TCPServer::LoadScene()
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, (char*)"ServerScene.bin", "rb");
	::rewind(pInFile);
	int fileEnd{};
	int nReads;
	while (true)
	{
		char pstrToken[128] = { '\0' };
		for (; ; )
		{
			if (::ReadStringFromFile(pInFile, pstrToken))
			{
				if (!strcmp(pstrToken, "<Hierarchy>:"))
				{
					char pStrFrameName[64];
					int nChild, nBoxCollider;
					XMFLOAT3 xmf3AABBCenter, xmf3AABBExtents;
					std::vector<BoundingOrientedBox> voobb;
					for (;;)
					{
						if (::ReadStringFromFile(pInFile, pstrToken))
						{
							if (!strcmp(pstrToken, "<Frame>:"))
							{
								::ReadIntegerFromFile(pInFile);
								::ReadStringFromFile(pInFile, pStrFrameName);
							}
							else if (!strcmp(pstrToken, "<Children>:"))
							{
								nChild = ::ReadIntegerFromFile(pInFile);
							}
							else if (!strcmp(pstrToken, "<BoxColliders>:"))
							{
								nBoxCollider = ::ReadIntegerFromFile(pInFile);
								voobb.reserve(nBoxCollider);
								for (int i = 0; i < nBoxCollider; ++i)
								{
									::ReadStringFromFile(pInFile, pstrToken);	// <Bound>
									int nIndex = 0;
									nReads = fread(&nIndex, sizeof(int), 1, pInFile);
									nReads = (UINT)::fread(&xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
									nReads = (UINT)::fread(&xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
									XMFLOAT4 xmf4Orientation;
									XMStoreFloat4(&xmf4Orientation, XMQuaternionIdentity());
									voobb.emplace_back(xmf3AABBCenter, xmf3AABBExtents, xmf4Orientation);
								}
							}
							else if (!strcmp(pstrToken, "<Matrix>:"))
							{
								nChild = ::ReadIntegerFromFile(pInFile);
								XMFLOAT4X4* xmf4x4World = new XMFLOAT4X4[nChild];
								nReads = (UINT)::fread(xmf4x4World, sizeof(XMFLOAT4X4), nChild, pInFile);
								for (int i = 0; i < nChild; ++i)
								{
									// 오브젝트 생성
									CreateSceneObject(pStrFrameName, Matrix4x4::Transpose(xmf4x4World[i]), voobb);
								}
								delete[] xmf4x4World;
							}
							else if (!strcmp(pstrToken, "</Frame>"))
							{
								break;
							}
						}
					}
				}
				else if (!strcmp(pstrToken, "</Hierarchy>"))
				{
					break;
				}
				else if (!strcmp(pstrToken, "</Scene>:"))
				{
					fileEnd = 1;
					break;
				}
			}
			else
			{
				break;
			}
		}
		if (fileEnd) 
		{
			break;
		}
	}


}

void TCPServer::CreateSceneObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb)
{
	static int nServerObjectNum = 0;
	shared_ptr<CServerGameObject> pGameObject;

	if (!strcmp(pstrFrameName, "Door_1"))
	{
		pGameObject = make_shared<CServerDoorObject>(pstrFrameName, xmf4x4World, voobb);
	}
	else if (!strcmp(pstrFrameName, "Drawer_1"))
	{
		if (m_nStartDrawer1 == -1)
		{
			m_nStartDrawer1 = nServerObjectNum;
			m_nEndDrawer1 = nServerObjectNum - 1;
		}
		m_nEndDrawer1++;
		pGameObject = make_shared<CServerDrawerObject>(pstrFrameName, xmf4x4World, voobb);
	}
	else if (!strcmp(pstrFrameName, "Drawer_2"))
	{
		if (m_nStartDrawer2 == -1)
		{
			m_nStartDrawer2 = nServerObjectNum;
			m_nEndDrawer2 = nServerObjectNum - 1;
		}
		m_nEndDrawer2++;
		pGameObject = make_shared<CServerDrawerObject>(pstrFrameName, xmf4x4World, voobb);
	}
	else if (!strcmp(pstrFrameName, "Door1"))
	{
		pGameObject = make_shared<CServerElevatorDoorObject>(pstrFrameName, xmf4x4World, voobb);
	}
	else if (!strcmp(pstrFrameName, "Emergency_Handle"))
	{
		pGameObject = make_shared<CServerElevatorDoorObject>(pstrFrameName, xmf4x4World, voobb);
	}
	else if (!strcmp(pstrFrameName, "Laboratory_Wall_1_Corner_1") || !strcmp(pstrFrameName, "Laboratory_Wall_1_Corner_2"))
	{
		pGameObject = make_shared<CServerEnvironmentObject>(pstrFrameName, xmf4x4World, voobb);
	}
	else if (!strcmp(pstrFrameName, "BoxCollide_Wall"))
	{
		pGameObject = make_shared<CServerEnvironmentObject>(pstrFrameName, xmf4x4World, voobb);
	}
	else if (!strcmp(pstrFrameName, "Laboratory_Wall_1_Corner") || !strcmp(pstrFrameName, "Laboratory_Wall_1_Corner2") || !strcmp(pstrFrameName, "Laboratory_Wall_1"))
	{
		pGameObject = make_shared<CServerEnvironmentObject>(pstrFrameName, xmf4x4World, voobb);
	}
	else if (!strcmp(pstrFrameName, "Laboratory_Wall_Door_1") || !strcmp(pstrFrameName, "Laboratory_Wall_Door_1_2"))
	{
		pGameObject = make_shared<CServerEnvironmentObject>(pstrFrameName, xmf4x4World, voobb);
	}
	else if(!strcmp(pstrFrameName, "Biological_Capsule_1") || !strcmp(pstrFrameName, "Laboratory_Table_1"))
	{
		pGameObject = make_shared<CServerEnvironmentObject>(pstrFrameName, xmf4x4World, voobb);
	}
	else if (!strcmp(pstrFrameName, "Laboratory_Tunnel_1_Stairs") || !strcmp(pstrFrameName, "Laboratory_Tunnel_1"))
	{
		pGameObject = make_shared<CServerEnvironmentObject>(pstrFrameName, xmf4x4World, voobb);
	}
	else if (!strcmp(pstrFrameName, "BoxCollider_Stair_Start"))
	{
		pGameObject = make_shared<CServerStairTriggerObject>(pstrFrameName, xmf4x4World, voobb);
	}
	else
	{
		pGameObject = make_shared<CServerGameObject>(pstrFrameName, xmf4x4World, voobb);
		pGameObject->SetStatic(true);
	}

	strcpy(pGameObject->m_pstrFrameName, pstrFrameName);

	nServerObjectNum++;
	m_pCollisionManager->AddCollisionObject(pGameObject);
}

void TCPServer::CreateItemObject()
{
	CServerItemObject::SetDrawerStartEnd(m_nStartDrawer1, m_nEndDrawer1, m_nStartDrawer2, m_nEndDrawer2);
	// 확률: fus 30, mine 30, tp 30, radar 10
	uniform_int_distribution<int> dis(m_nStartDrawer1, m_nEndDrawer2);
	uniform_int_distribution<int> item_dis(0, 99);
	uniform_int_distribution<int> rotation_dis(1, 360);
	uniform_real_distribution<float> pos_dis(-0.2f, 0.2f);
	for(int i = 0; i < 80;++i)
	{
		int nDrawerNum = dis(m_mt19937Gen);
		shared_ptr<CServerDrawerObject> pDrawerObject = dynamic_pointer_cast<CServerDrawerObject>(m_pCollisionManager->GetCollisionObjectWithNumber(nDrawerNum));
		if (!pDrawerObject) //error
			exit(1);

		if (pDrawerObject->m_pStoredItem)	// 이미 다른 아이템이 들어왔음
		{
			--i;
			continue;
		}
		XMFLOAT4X4 xmf4x4World = m_pCollisionManager->GetCollisionObjectWithNumber(nDrawerNum)->GetWorldMatrix();

		int nCreateItem = item_dis(m_mt19937Gen);
		shared_ptr<CServerItemObject> pItemObject;

		XMFLOAT3 xmf3RandOffset = XMFLOAT3(pos_dis(m_mt19937Gen), 0.0f, pos_dis(m_mt19937Gen));
		XMFLOAT3 xmf3RandRotation = XMFLOAT3(0.0f, 0.0f, (float)rotation_dis(m_mt19937Gen));
		if(i < 10)		// Fuse
		{
			pItemObject = make_shared<CServerFuseObject>();
			pItemObject->SetDrawerNumber(nDrawerNum);
			pItemObject->SetDrawer(pDrawerObject);
			pDrawerObject->m_pStoredItem = pItemObject;

			pItemObject->SetRandomRotation(xmf3RandRotation);
			pItemObject->SetRandomOffset(xmf3RandOffset);

			pItemObject->SetWorldMatrix(xmf4x4World);
			m_pCollisionManager->AddCollisionObject(pItemObject);
		}
		else if (i < 20)	// tp
		{
			pItemObject = make_shared<CServerTeleportObject>();
			pItemObject->SetDrawerNumber(nDrawerNum);
			pItemObject->SetDrawer(pDrawerObject);
			pDrawerObject->m_pStoredItem = pItemObject;

			pItemObject->SetRandomRotation(xmf3RandRotation);
			pItemObject->SetRandomOffset(xmf3RandOffset);
			pItemObject->SetWorldMatrix(xmf4x4World);
			m_pCollisionManager->AddCollisionObject(pItemObject);
		}
		else if (i < 30)	// Rader
		{
			xmf3RandRotation = XMFLOAT3(90.0f, 0.0f, 0.0f);
			pItemObject = make_shared<CServerRadarObject>();
			pItemObject->SetDrawerNumber(nDrawerNum);
			pItemObject->SetDrawer(pDrawerObject);
			pDrawerObject->m_pStoredItem = pItemObject;

			pItemObject->SetRandomRotation(xmf3RandRotation);
			pItemObject->SetRandomOffset(xmf3RandOffset);
			pItemObject->SetWorldMatrix(xmf4x4World);
			m_pCollisionManager->AddCollisionObject(pItemObject);
		}
		else if (i < 80)	// Mine
		{
			xmf3RandRotation = XMFLOAT3(90.0f, 0.0f, 0.0f);

			pItemObject = make_shared<CServerMineObject>();
			pItemObject->SetDrawerNumber(nDrawerNum);
			pItemObject->SetDrawer(pDrawerObject);
			pDrawerObject->m_pStoredItem = pItemObject;

			pItemObject->SetRandomRotation(xmf3RandRotation);
			pItemObject->SetRandomOffset(xmf3RandOffset);
			pItemObject->SetWorldMatrix(xmf4x4World);
			m_pCollisionManager->AddCollisionObject(pItemObject);
		}

	}
}

void TCPServer::CreateSendObject()
{
	for (const auto& pPlayer : m_apPlayers)
	{
		int nIndex = 0;
		if (!pPlayer || pPlayer->GetPlayerId() == -1)
		{
			continue;
		}

		INT8 nId = pPlayer->GetPlayerId();

		// 공간 외에 업데이트가 필요한 경우의 오브젝트
		// EX) 아이템 사용 후 다시 맵에 리젠 되어야하는 아이템 오브젝트
		for (auto& pGameObject : m_pCollisionManager->GetOutSpaceObject())
		{
			if (!pGameObject)
			{
				continue;
			}
			if (nIndex >= MAX_SEND_OBJECT_INFO) {
				//std::cout << "nIndex 가 Max 치를 넘은 오류\n";
				assert(0);
			}
			m_aUpdateInfo[nId].m_anObjectNum[nIndex] = pGameObject->GetCollisionNum();
			if (m_aUpdateInfo[nId].m_anObjectNum[nIndex] >= m_pCollisionManager->GetNumberOfCollisionObject()) {
				//std::cout << "index 범위를 넘어서는 오브젝트를 담았습니다.\n";
			}
			m_aUpdateInfo[nId].m_axmf4x4World[nIndex] = pGameObject->GetWorldMatrix();
			nIndex++;
		}
		m_pCollisionManager->GetOutSpaceObject().clear();


		// 층은 나중에 계단쪽에서만 추가할수있도록해야할듯
		// if(계단 쪽이면 위층 or 아래층범위 검사)

		for (int j = pPlayer->GetWidth() - 1; j <= pPlayer->GetWidth() + 1 && nIndex < MAX_SEND_OBJECT_INFO; ++j)
		{
			if (j < 0 || j > m_pCollisionManager->GetWidth() - 1)
			{
				continue;
			}

			for (int k = pPlayer->GetDepth() - 1; k <= pPlayer->GetDepth() + 1 && nIndex < MAX_SEND_OBJECT_INFO; ++k)
			{
				if (k < 0 || k > m_pCollisionManager->GetDepth() - 1)
				{
					continue;
				}

				for (const auto& pGameObject : m_pCollisionManager->GetSpaceGameObjects(pPlayer->GetFloor(), j, k))
				{
					if (!pGameObject || pGameObject->IsStatic())
					{
						continue;
					}
					
					m_aUpdateInfo[nId].m_anObjectNum[nIndex] = pGameObject->GetCollisionNum();
					m_aUpdateInfo[nId].m_axmf4x4World[nIndex] = pGameObject->GetWorldMatrix();

					nIndex++;
					//if (m_aUpdateInfo[nId].m_anObjectNum[nIndex] >= m_pCollisionManager->GetNumberOfCollisionObject()) {
					//	std::cout << "index 범위를 넘어서는 오브젝트를 담았습니다.\n";
					//}
					if (nIndex == MAX_SEND_OBJECT_INFO)
					{
						//std::cout << "보내려고 하는 객체가 MAX_SEND_OBJECT_INFO 개수가 되었습니다.\n";
						break;
					}
				}
				m_aUpdateInfo[nId].m_nNumOfObject = nIndex;
			}
		}
	}
}

void TCPServer::InitPlayerPosition(shared_ptr<CServerPlayer>& pServerPlayer, int nIndex)
{
	// 후보지를 두고 int 값에 따라 그곳에 가도록 해야할듯
	uniform_int_distribution<int> disIntPosition(0, 4);

	int nStartPosNum = disIntPosition(m_mt19937Gen);
	bool bEmpty = false;
	while (!bEmpty)
	{
		bEmpty = true;
		nStartPosNum = disIntPosition(m_mt19937Gen);
		for (const auto& nPlayerStartPos : m_anPlayerStartPosNum)
		{
			if (nPlayerStartPos == nStartPosNum)
			{
				bEmpty = false;
				break;
			}
		}
	}

	m_anPlayerStartPosNum[nIndex] = nStartPosNum;
	XMFLOAT3 xmf3Position = m_axmf3Positions[nStartPosNum];
	pServerPlayer->SetPlayerPosition(xmf3Position);
	pServerPlayer->SetPlayerOldPosition(xmf3Position);
}

template<class... Args>
void TCPServer::CreateSendDataBuffer(char* pBuffer, Args&&... args)
{
	size_t nOffset = 0;
	((memcpy(pBuffer + nOffset, &args, sizeof(args)), nOffset += sizeof(args)), ...);
}

// 여러개 데이터를 묶어서 보낼때 사용
template<class... Args>
int TCPServer::SendData(SOCKET socket, size_t nBufferSize, Args&&... args)
{
	int nRetval;
	char* pBuffer = new char[nBufferSize];
	(CreateSendDataBuffer(pBuffer, args...));

	nRetval = send(socket, (char*)pBuffer, nBufferSize, 0);
	delete[] pBuffer;
	
	if (nRetval == SOCKET_ERROR)
	{
		err_display("send()");
		return SOCKET_ERROR;
	}
	return 0;
}

int TCPServer::RecvData(int nSocketIndex, size_t nBufferSize)
{
	int nRetval;
	int nRemainRecvByte = nBufferSize - m_vSocketInfoList[nSocketIndex].m_nCurrentRecvByte;

	
	nRetval = recv(m_vSocketInfoList[nSocketIndex].m_sock, (char*)&m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer + m_vSocketInfoList[nSocketIndex].m_nCurrentRecvByte, nRemainRecvByte, 0);
	
	if (nRetval > 0)m_vSocketInfoList[nSocketIndex].m_nCurrentRecvByte += nRetval;
	if (nRetval == SOCKET_ERROR || nRetval == 0) // error
	{
		return -1;
	}
	else if (m_vSocketInfoList[nSocketIndex].m_nCurrentRecvByte < nBufferSize)
	{
		m_vSocketInfoList[nSocketIndex].m_bRecvDelayed = true;
		return 1;
	}
	else
	{
		m_vSocketInfoList[nSocketIndex].m_nCurrentRecvByte = 0;
		m_vSocketInfoList[nSocketIndex].m_bRecvDelayed = false;
		return 0;
	}
}

// 소켓 함수 오류 출력 후 종료
void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	MessageBoxA(NULL, (const char*)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}
// 소켓 함수 오류 출력
void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[%s] %s\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
// 소켓 함수 오류 출력
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[오류] %s\n", (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
