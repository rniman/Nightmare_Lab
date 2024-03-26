#include "stdafx.h"
#include "TCPServer.h"

size_t TCPServer::m_nClient = 0;

TCPServer::TCPServer()
{
}

TCPServer::~TCPServer()
{
}

void TCPServer::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
			m_timer.Stop();
		else
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
		return OnProcessingAcceptMessage(hWnd, nMessageID, wParam, lParam);
	case FD_WRITE:
		return OnProcessingWriteMessage(hWnd, nMessageID, wParam, lParam);
	case FD_READ:
		return OnProcessingReadMessage(hWnd, nMessageID, wParam, lParam);
	case FD_CLOSE:
		RemoveSocketInfo((SOCKET)wParam);
		break;
	default:
		break;
	}

	return;
}

void TCPServer::OnProcessingAcceptMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	// 최대 클라이언트 수를 넘길수 없습니다.
	if (m_nClient >= MAX_CLIENT)
	{
		return;
	}

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
	int nClientIndex = AddSocketInfo(sockClient, addrClient, nAddrlen);
	printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n", m_vSocketInfoList[nClientIndex].m_pAddr, ntohs(m_vSocketInfoList[nClientIndex].m_addrClient.sin_port));

	int retval = WSAAsyncSelect(sockClient, hWnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
	if (retval == SOCKET_ERROR)
	{
		err_display("WSAAsyncSelect()");
		RemoveSocketInfo(sockClient);
	}

	return;
}

void TCPServer::OnProcessingReadMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	int nSocketIndex = GetSocketIndex(wParam);
	if (!m_vSocketInfoList[nSocketIndex].m_bUsed)
	{
		//error
	}

	int nRetval;
	switch (m_vSocketInfoList[nSocketIndex].m_socketState)
	{
	case SOCKET_STATE::RECV_KEY_BUFFER:
	{
		UCHAR keysBuffer[257];
		int nBufferSize = sizeof(keysBuffer);
		nRetval = recv(m_vSocketInfoList[nSocketIndex].m_sock
			, (char*)&m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer + m_vSocketInfoList[nSocketIndex].m_nCurrentRecvByte, nBufferSize - m_vSocketInfoList[nSocketIndex].m_nCurrentRecvByte, 0);
		m_vSocketInfoList[nSocketIndex].m_nCurrentRecvByte += nRetval;
		if (nRetval == SOCKET_ERROR || nRetval == 0)
		{
			// error
		}
		else if (nRetval < nBufferSize)
		{
			m_vSocketInfoList[nSocketIndex].m_bRecvDelayed = true;
		}
		else
		{
			m_vSocketInfoList[nSocketIndex].m_socketState = SOCKET_STATE::SEND_ACK;	// 데이터의 길이를 받을 상태로 바뀐다.
			m_vSocketInfoList[nSocketIndex].m_nCurrentRecvByte = 0;
			memcpy(&m_pKeysBuffer, m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer, nBufferSize - 1);
			
			if(m_vSocketInfoList[nSocketIndex].m_clientInfo.m_nClientId == 0)
			{
				DWORD press = 0;
				if (m_pKeysBuffer['W'] & 0xF0) press |= 0x01;
				if (m_pKeysBuffer['S'] & 0xF0) press |= 0x02;
				if (m_pKeysBuffer['A'] & 0xF0) press |= 0x04;
				if (m_pKeysBuffer['D'] & 0xF0) press |= 0x08;
				for (int i = 7; i >= 0; --i)
				{
					int result = press >> i & 1;
					printf("%d", result);
				}
				printf("\n");
			}

			memset(m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer, 0, BUFSIZE);
			//m_vSocketInfoList[nSocketIndex].m_nRecvDataLength = 0;
			if (m_vSocketInfoList[nSocketIndex].m_bRecvDelayed)
			{
				m_vSocketInfoList[nSocketIndex].m_bRecvDelayed = false;
			}
			PostMessage(hWnd, WM_SOCKET, (WPARAM)m_vSocketInfoList[nSocketIndex].m_sock, MAKELPARAM(FD_WRITE, 0));
		}
		break; 
	}
	default:
		break;
	}

	return;
}

void TCPServer::OnProcessingWriteMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	int nRetval;
	int nSocketIndex = GetSocketIndex(wParam);
	if (!m_vSocketInfoList[nSocketIndex].m_bUsed)
	{
		//error
	}

	if (m_vSocketInfoList[nSocketIndex].m_socketState == SOCKET_STATE::SEND_ID)
	{
		//char* buf = new char[sizeof(int)];
		//memcpy(buf, (char*)&m_vSocketInfoList[nSocketIndex].m_clientInfo.m_nClientId, sizeof(int));
		//memcpy(buf, (char*)&m_vSocketInfoList[nSocketIndex].m_clientInfo.m_nClientId, sizeof(int));
		//memcpy(buf + sizeof(int), (char*)&m_nClient, sizeof(int));

		//int a, b;
		//memcpy(&a, buf, sizeof(int));
		//memcpy(&b, buf + sizeof(int), sizeof(int));

   		//nRetval = send(m_vSocketInfoList[nSocketIndex].m_sock, (char*)&buf, sizeof(int), 0);
   		nRetval = send(m_vSocketInfoList[nSocketIndex].m_sock, (char*)&m_vSocketInfoList[nSocketIndex].m_clientInfo.m_nClientId, sizeof(int), 0);
		if (nRetval == SOCKET_ERROR)
		{
			err_display("send()");
		}

		m_vSocketInfoList[nSocketIndex].m_socketState = SOCKET_STATE::RECV_KEY_BUFFER;
		//delete[] buf;
		return;
	}

	if (m_vSocketInfoList[nSocketIndex].m_socketState == SOCKET_STATE::SEND_ACK)
	{
		int nComplete = 0;
		nRetval = send(m_vSocketInfoList[nSocketIndex].m_sock, (char*)&nComplete, sizeof(int), 0);
		if (nRetval == SOCKET_ERROR)
		{
			err_display("send()");
		}

		m_vSocketInfoList[nSocketIndex].m_socketState = SOCKET_STATE::RECV_KEY_BUFFER;
		return;
	}
	return;
}

bool TCPServer::Init(HWND hWnd)
{
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

	return true;
}

void TCPServer::Update()
{
	// 실제 시뮬레이션이 일어날곳

}

// 소켓 정보 추가
int TCPServer::AddSocketInfo(SOCKET sockClient, struct sockaddr_in addrClient, int nAddrLen)
{
	int nSocketIndex = -1;
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
	sockInfo.m_bSendDelayed = false;
	sockInfo.m_socketState = SOCKET_STATE::SEND_ID;
	sockInfo.m_prevSocketState = SOCKET_STATE::SEND_ID;
	
	// 배열에 정보 추가 
	for (int i = 0; i < m_nClient + 1; ++i)
	{
		if (m_vSocketInfoList[i].m_bUsed)
		{
			continue;
		}

		// 클라이언트 정보 초기화
		sockInfo.m_clientInfo.m_nClientId = i;
		sockInfo.m_clientInfo.x = 0.0f;
		sockInfo.m_clientInfo.y = 0.0f;
		sockInfo.m_clientInfo.z = 0.0f;

		m_vSocketInfoList[i] = sockInfo;
		nSocketIndex = i;

		m_nClient++;
		break;
	}
	
	return nSocketIndex;
}

// 소켓 정보 얻기
int TCPServer::GetSocketIndex(SOCKET sock)
{
	int nIndex = -1;
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
void TCPServer::RemoveSocketInfo(SOCKET sock)
{
	// 리스트에서 정보 제거
	for (auto& sockInfo : m_vSocketInfoList)
	{
		if (!sockInfo.m_bUsed)
		{
			continue;
		}

		if (sockInfo.m_sock == sock)
		{
			printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n", sockInfo.m_pAddr, ntohs(sockInfo.m_addrClient.sin_port));
			closesocket(sockInfo.m_sock); // 소켓 닫기
			sockInfo.m_bUsed = false;
			m_nClient--;
			return;
		}
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
