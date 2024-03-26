#include "stdafx.h"
#include "TCPClient.h"
#include "GameFramework.h"

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
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock == INVALID_SOCKET)
	{
		err_quit("socket()");
	}

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);

	nRetval = connect(m_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (nRetval == SOCKET_ERROR) 
	{
		err_quit("connect()");
		return;
	}

	nRetval = WSAAsyncSelect(m_sock, hWnd, WM_SOCKET, FD_CLOSE | FD_READ | FD_WRITE);	// FD_WRITE가 발생할것이다.
}

/*
서버에 연결이 되면 현재 클라이언트 플레이어를 서버에서 생성 후 
서버에서 클라ID, 좌표(float(x,y,z)) 정보를 클라로 넘김.
*/

void CTcpClient::OnDestroy()
{

}

XMFLOAT3 CTcpClient::GetPostion(int id)
{
	XMFLOAT3 position = { 0.0f,0.0f, 0.0f };

	return position;
}

unordered_map<int, Client_INFO>& CTcpClient::GetUMapClientInfo()
{
	return m_umapClientInfo;
}

void CTcpClient::OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_WRITE:	// 소켓이 데이터를 전송할 준비가 되었다.
		OnProcessingWriteMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case FD_READ:	// 소켓이 데이터를 읽을 준비가 되었다.
		OnProcessingReadMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case FD_CLOSE:
		closesocket(m_sock);
		WSACleanup();	
		break;
	default:
		break;
	}
}

void CTcpClient::OnProcessingReadMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	int nRetval;
	if (m_nMainClientID == -1)
	{
		nRetval = recv(m_sock, (char*)&m_pCurrentBuffer + m_nCurrentRecvByte, sizeof(int) - m_nCurrentRecvByte, 0);
		m_nCurrentRecvByte += nRetval;
		if (nRetval == SOCKET_ERROR || nRetval == 0)
		{
			// error
		}
		else if (nRetval < sizeof(int))
		{
			m_bRecvDelayed = true;
			return;
		}
		else
		{
			m_socketState = SOCKET_STATE::SEND_KEY_BUFFER;
			memcpy(&m_nMainClientID, m_pCurrentBuffer, sizeof(int));
			//memcpy(&m_nClient, m_pCurrentBuffer + sizeof(int), sizeof(int));

			m_nCurrentRecvByte = 0;
			memset(m_pCurrentBuffer, 0, BUFSIZE);
			// 딜레이 되었었다면 다시 메시지를 보내주자. -> 여기서는 그럴필요가없다
			if (m_bSendDelayed || m_bRecvDelayed)
			{
				m_bSendDelayed = false;
				m_bRecvDelayed = false;
				PostMessage(hWnd, WM_SOCKET, (WPARAM)m_sock, MAKELPARAM(FD_WRITE, 0));
			}
			return;
		}
	}

	if (m_socketState == SOCKET_STATE::RECV_SERVER_ACK)
	{
		nRetval = recv(m_sock, (char*)&m_pCurrentBuffer + m_nCurrentRecvByte, sizeof(int) - m_nCurrentRecvByte, 0);
		m_nCurrentRecvByte += nRetval;
		if (nRetval == SOCKET_ERROR || nRetval == 0)
		{
			// error
		}
		else if (nRetval < sizeof(int))
		{
			m_bRecvDelayed = true;
			return;
		}
		else
		{
			int nComplete = -1;
			m_socketState = SOCKET_STATE::SEND_KEY_BUFFER;
			memcpy(&nComplete, m_pCurrentBuffer, sizeof(int));
			if (nComplete != 0)
			{
				//error
				exit(-1);
			}

			m_nCurrentRecvByte = 0;
			memset(m_pCurrentBuffer, 0, BUFSIZE);
			if (m_bSendDelayed || m_bRecvDelayed)
			{
				m_bSendDelayed = false;
				m_bRecvDelayed = false;
			}
			PostMessage(hWnd, WM_SOCKET, (WPARAM)m_sock, MAKELPARAM(FD_WRITE, 0));

			return;
		}
	}
	return;
}

void CTcpClient::OnProcessingWriteMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	int nRetval;
	if (m_nMainClientID == -1)	// 아직 ID를 넘겨 받지 못했다.
	{
		m_bSendDelayed = true;
		return;
	}

	if (m_bRecvDelayed == true)	// recv가 딜레이되었다
	{
		m_bSendDelayed = true;
		return;
	}
	
	switch (m_socketState)
	{
	case SOCKET_STATE::SEND_KEY_BUFFER:
	{
		UCHAR keysBuffer[257];
		int nBufferSize = sizeof(keysBuffer);
		UCHAR* pKeysBuffer = CGameFramework::GetKeysBuffer();
		if (pKeysBuffer != nullptr)
		{
			memcpy(keysBuffer, pKeysBuffer, 256);
		}
		keysBuffer[256] = '\0';
		unsigned long bytesInBuffer;
		int result = ioctlsocket(m_sock, FIONREAD, &bytesInBuffer);
		nRetval = send(wParam, (char*)keysBuffer, nBufferSize, 0);

		if (nRetval == SOCKET_ERROR)
		{
			err_display("send()");
		}
		m_socketState = SOCKET_STATE::RECV_SERVER_ACK;
	}
		break;
	default:
		m_bSendDelayed = true;
		break;
	}
}
