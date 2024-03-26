#pragma once
#include "Timer.h"
constexpr size_t MAX_CLIENT{ 5 };

// 소켓 정보 저장을 위한 구조체와 변수
struct Client_INFO
{
	int m_nClientId;
	float x, y, z;
};

enum class SOCKET_STATE
{
	RECV_CLIENT_ACK = 0, // 클라에서 데이터를 받았다는 응답
	RECV_KEY_BUFFER,
	SEND_ACK,			 // 클라이언트에 보내는 응답
	SEND_ID,
	SEND_KEY_BUFFER,
};

struct SOCKETINFO
{
	bool m_bUsed = false;
	SOCKET m_sock;

	struct sockaddr_in m_addrClient;
	int m_nAddrlen;
	char m_pAddr[INET_ADDRSTRLEN];

	Client_INFO m_clientInfo;
	bool m_bRecvDelayed = false;	// 오는 데이터를 전부 받지 못했다
	bool m_bSendDelayed = false;	// 오는 데이터를 전부 받지 못했다
	int m_nCurrentRecvByte = 0;		// 현재까지 받은 데이터의 길이
	char m_pCurrentBuffer[BUFSIZE + 1];

	SOCKET_STATE m_socketState = SOCKET_STATE::SEND_ID;
	SOCKET_STATE m_prevSocketState = SOCKET_STATE::SEND_ID;
};

class TCPServer
{
public:
	TCPServer();
	~TCPServer();

	// 이벤트를 처리한다.
	void OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingAcceptMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingReadMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingWriteMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	bool Init(HWND hWnd);

	void SimulationLoop();

	// 소켓 정보 관리 함수
	int AddSocketInfo(SOCKET sockClient, struct sockaddr_in addrClient, int nAddrLen);
	int GetSocketIndex(SOCKET sockClient);
	void RemoveSocketInfo(SOCKET sock);

	UCHAR* GetKeysBuffer() { return m_pKeysBuffer; }
private:
	CTimer m_timer;
	static size_t m_nClient;

	// 접속한 클라이언트들의 정보를 저장.
	std::array<SOCKETINFO, MAX_CLIENT> m_vSocketInfoList;
	// 서버에서 시뮬레이션되는 데이터를 저장하고 클라이언트에게 프레임마다 보내는 데이터이다.
	vector<BYTE> packedData;

	// 임시 키버퍼
	UCHAR m_pKeysBuffer[256];
};

extern void err_quit(const char* msg);
extern void err_display(const char* msg);
extern void err_display(int errcode);


