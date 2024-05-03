#pragma once
constexpr UINT WM_SOCKET{ WM_USER + 1 };
constexpr char SERVERIP[16]{ "127.0.0.1" };
constexpr UINT SERVERPORT{ 9000 };
constexpr UINT BUFSIZE{ 10000 };

constexpr size_t MAX_CLIENT{ 5 };
constexpr size_t MAX_RECV_OBJECT_INFO{ 60 };

class CPlayer;

enum class SOCKET_STATE
{
	SEND_KEY_BUFFER,
};

enum RECV_HEAD
{
	HEAD_INIT = 0,
	HEAD_UPDATE_DATA,
	HEAD_NUM_OF_CLIENT
};

struct CS_ANIMATION_INFO {
	float pitch = 0.0f;
};

struct CS_PLAYER_INFO {
	RightItem m_selectItem;
	bool m_bRightClick = false;
};

struct CS_CLIENTS_INFO
{
	int m_nClientId = -1;
	bool m_bAlive = true;
	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3Velocity;
	XMFLOAT3 m_xmf3Look;
	XMFLOAT3 m_xmf3Right;

	int m_nSlotObjectNum[3];	// 각 슬롯에 포함된 오브젝트 번호(없으면 -1)
	int m_nFuseObjectNum[3];	// 퓨즈 오브젝트 번호(없으면 -1)

	int m_nNumOfObject = -1;
	std::array<int, MAX_RECV_OBJECT_INFO> m_anObjectNum;
	std::array<XMFLOAT4X4, MAX_RECV_OBJECT_INFO> m_axmf4x4World;

	CS_ANIMATION_INFO m_animationInfo;
	CS_PLAYER_INFO m_playerInfo;
};

class CTcpClient
{
private:
	int m_nMainClientId = -1;
	int m_nClient = -1;				// 클라이언트 수

	bool m_bRecvDelayed = false;	// 오는 데이터를 전부 받지 못했다
	bool m_bRecvHead = false;

	int m_nCurrentRecvByte = 0;		// 현재까지 받은 데이터의 길이
	char m_pCurrentBuffer[BUFSIZE];

	SOCKET_STATE m_socketState = SOCKET_STATE::SEND_KEY_BUFFER;
	SOCKET_STATE m_prevSocketState = SOCKET_STATE::SEND_KEY_BUFFER;

	// 서버 접속 소켓

	// 서버에 접속한 클라이언트의 정보 <아이디,정보>
	std::array<CS_CLIENTS_INFO, MAX_CLIENT> m_aClientInfo;
	std::array<shared_ptr<CPlayer>, MAX_CLIENT> m_apPlayers;

public:
	SOCKET m_sock;
	bool m_bSend = true;

	int SendNum = 0;
	int RecvNum = 0;
	CTcpClient(HWND hWnd);
	~CTcpClient();

	void CreateSocket(HWND hWnd);
	void OnDestroy();
	void SetPlayer(const shared_ptr<CPlayer>& pPlayer, int nIndex = 0) { m_apPlayers[nIndex] = pPlayer; };

	// 이벤트를 처리한다.
	void OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingReadMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingWriteMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	template<class... Args>
	void CreateSendDataBuffer(char* pBuffer, Args&&... args);
	template<class... Args>
	int SendData(SOCKET socket, size_t nBufferSize, Args&&... args);
	int RecvData(SOCKET socket, size_t nBufferSize);

	void UpdateZombiePlayer();
	void UpdatePlayerItem(int nIndex);

	//Interface
	int GetClientId() const { return m_nMainClientId; }
	int GetNumOfClient() const { return m_nClient; }
	XMFLOAT3 GetPostion(int id);
	std::array<CS_CLIENTS_INFO, 5>& GetArrayClientsInfo();
	//SOCKET GetSocket() { return m_sock; }
};