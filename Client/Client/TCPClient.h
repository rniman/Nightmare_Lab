#pragma once
struct Client_INFO {
	int Id;
	float x, y, z;
};

class TCPClient {
private:
	// 서버 접속 소켓
	SOCKET sock;
	// 데이터 통신에 사용할 버퍼
	vector<BYTE> buffer;

	// 서버에 접속한 클라이언트의 정보 <아이디,정보>
	unordered_map<int, Client_INFO> client_infos;

	int my_Id;
public:
	TCPClient();
	~TCPClient();

	void Init();
	bool Logic();
	void Exit();

	bool SendData(char* data, int size);
	bool RecvData(char* data, int size);

	XMFLOAT3 GetPostion(int id);

	unordered_map<int, Client_INFO>& GetClientInfos();
	int GetMyId();
};