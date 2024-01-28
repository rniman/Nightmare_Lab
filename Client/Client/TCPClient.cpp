#include "stdafx.h"
#include "TCPClient.h"

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    50

TCPClient::TCPClient()
{
	Init();
}

TCPClient::~TCPClient()
{
}

void TCPClient::Init()
{
	int retval;

	// 명령행 인수가 있으면 IP 주소로 사용
	//if (argc > 1) SERVERIP = argv[1];

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return;

	// 소켓 생성
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);

	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) {
		err_quit("connect()");
		return;
	}

	// 1. 접속한 클라이언트 개수
	int client_count;
	RecvData((char*)&client_count, sizeof(int));
	// 2. 접속한 클라이언트 정보
	for (int i = 0; i < client_count;++i) {
		Client_INFO info;
		RecvData((char*)&info, sizeof(Client_INFO));

		client_infos[info.Id] = info;
	}
	// 3. 클라이언트 ID 
	RecvData((char*)&my_Id, sizeof(int));
}

/*
서버에 연결이 되면 현재 클라이언트 플레이어를 서버에서 생성 후 
서버에서 클라ID, 좌표(float(x,y,z)) 정보를 클라로 넘김.

*/

bool TCPClient::Logic()
{
	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE];
	const char* str = {
		"클라이언트_"
	};
	string str_id = to_string(my_Id);
	char* sendData = new char[(int)strlen(str) + (int)str_id.size()];
	int ss = strlen(sendData);
	std::strcpy(sendData, str);
	std::strcat(sendData, str_id.c_str());

	int len;
	len = (int)strlen(sendData);
	strncpy(buf, sendData, len);

	// str 클라이언트_N 데이터 보내기
	if (!SendData(buf, len)) {
		return false;
	}

	// 서버에 접속한 현재 인원
	int client_count;
	if (!RecvData((char*)&client_count, sizeof(int))) {
		return false;
	}
	// 접속한 클라이언트의 플레이어 정보
	BYTE* byte =new BYTE[client_count * sizeof(Client_INFO)];
	if (!RecvData((char*)byte, client_count * sizeof(Client_INFO))) {
		return false;
	}

	for (int i = 0;i < client_count;++i) {
		Client_INFO info;
		memcpy(&info.Id, &byte[0 + i * sizeof(Client_INFO)], sizeof(int));
		memcpy(&info.x, &byte[4 + i * sizeof(Client_INFO)], sizeof(float));
		memcpy(&info.y, &byte[8 + i * sizeof(Client_INFO)], sizeof(float));
		memcpy(&info.z, &byte[12 + i * sizeof(Client_INFO)], sizeof(float));

		client_infos[info.Id] = info;
	}

	delete[] byte;

	return true;
}

void TCPClient::Exit()
{
	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
}

XMFLOAT3& TCPClient::GetPostion(int id)
{
#ifndef SINGLE_PLAY
	XMFLOAT3 position = { client_infos[id].x,client_infos[id].y,client_infos[id].z };
#else
	XMFLOAT3 position = { 0.0f,0.0f,100.0f };
#endif
	return position;
}

unordered_map<int, Client_INFO>& TCPClient::GetClientInfos()
{
	return client_infos;
}

int TCPClient::GetMyId()
{
	return my_Id;
}

bool TCPClient::SendData(char* data, int size)
{
	int retval = send(sock, data, size, 0);

	if (retval == SOCKET_ERROR) {
		err_display("send()");
		Exit();
		return false;
	}

	return true;
}

bool TCPClient::RecvData(char* data, int size)
{
	int retval = recv(sock, data, size, MSG_WAITALL);

	if (retval == SOCKET_ERROR) {
		err_display("recv()");
		Exit();
		return false;
	}
	else if (retval == 0) {
		Exit();
		return false;
	}

	return true;
}