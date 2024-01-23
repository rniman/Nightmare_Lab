#pragma once
#include "../Common.h"
// 소켓 정보 저장을 위한 구조체와 변수
struct Client_INFO {
	int Id;
	float x, y, z;
};

struct SOCKETINFO
{
	SOCKET sock;
	Client_INFO client_info;
	char buf[BUFSIZE + 1];
	int recvbytes;
	int sendbytes;
	bool recvdelayed;
	//SOCKETINFO *next;
};

class TCPServer
{
public:
	TCPServer();
	~TCPServer();

	int AcceptProcess(HWND hWnd, SOCKET& client_sock, int addrlen, sockaddr_in& clientaddr, WPARAM wParam);
	int ReadProcess(HWND hWnd, int addrlen, sockaddr_in& clientaddr, WPARAM wParam);
	int WriteProcess(HWND hWnd, int addrlen, sockaddr_in& clientaddr, WPARAM wParam);

	// 접속한 클라이언트들의 정보를 저장.
	vector<SOCKETINFO> socketInfoList;
	// 서버에서 시뮬레이션되는 데이터를 저장하고 클라이언트에게 프레임마다 보내는 데이터이다.
	vector<BYTE> packedData;

	bool Init(HWND hWnd);
	// 데이터 송수신 함수
	bool SendData(SOCKET& sock, char* data, int size);
	bool RecvData(SOCKET& sock, char* data, int size);

	void InsertData(void* data, size_t size);
	void ScenePackageData();

	// 소켓 정보 관리 함수
	bool AddSocketInfo(SOCKET sock);
	void SocketInfoInit(SOCKETINFO* socketinfo);
	SOCKETINFO* GetSocketInfo(SOCKET sock);
	void RemoveSocketInfo(SOCKET sock);
};

extern void err_quit(const char* msg);
extern void err_display(const char* msg);
extern void err_display(int errcode);