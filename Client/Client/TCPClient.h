#pragma once
constexpr UINT WM_SOCKET{ WM_USER + 1 };
constexpr char SERVERIP[16]{ "127.0.0.1" };
constexpr UINT SERVERPORT{ 9000 };
constexpr UINT BUFSIZE{ 500 };

enum class SOCKET_STATE
{
	RECV_SERVER_ACK = 0, // 서버에서 데이터를 받았다는 응답
	RECV_ID,
	SEND_KEY_BUFFER,
};

struct Client_INFO
{
	int m_nClientId;
	float x, y, z;
};

class CTcpClient
{
private:
	int m_nMainClientID = -1;
	int m_nClient = -1;

	bool m_bRecvDelayed = false;	// 오는 데이터를 전부 받지 못했다
	bool m_bSendDelayed = false;

	int m_nCurrentRecvByte = 0;		// 현재까지 받은 데이터의 길이
	char m_pCurrentBuffer[BUFSIZE];

	SOCKET_STATE m_socketState = SOCKET_STATE::RECV_ID;
	SOCKET_STATE m_prevSocketState = SOCKET_STATE::RECV_ID;
	//SEND_STATE m_sendState = SEND_STATE::SEND_DATA_LENGTH;

	// 서버 접속 소켓
	SOCKET m_sock;
	// 데이터 통신에 사용할 버퍼
	vector<BYTE> m_vbuffer;

	// 서버에 접속한 클라이언트의 정보 <아이디,정보>
	unordered_map<int, Client_INFO> m_umapClientInfo;
public:
	CTcpClient(HWND hWnd);
	~CTcpClient();

	void CreateSocket(HWND hWnd);
	void OnDestroy();

	// 이벤트를 처리한다.
	void OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingReadMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingWriteMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	//Interface
	int GetClientId() const { return m_nMainClientID; }
	int GetNumOfClient() const { return m_nClient; }
	XMFLOAT3 GetPostion(int id);
	unordered_map<int, Client_INFO>& GetUMapClientInfo();
	SOCKET GetSocket() const { return m_sock; }
};