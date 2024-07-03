#pragma once
constexpr UINT WM_SOCKET{ WM_USER + 1 };
constexpr char SERVERIP[16]{ "127.0.0.1" };
constexpr UINT SERVERPORT{ 9000 };
constexpr UINT BUFSIZE{ 12000 };

constexpr size_t MAX_CLIENT{ 5 };
constexpr size_t MAX_SURVIVOR{ 4 };
constexpr size_t MAX_ZOMBIE{ 1 };
constexpr size_t MAX_RECV_OBJECT_INFO{ 30 };

constexpr WORD KEY_W{ 0x01 };
constexpr WORD KEY_S{ 0x02 };
constexpr WORD KEY_A{ 0x04 };
constexpr WORD KEY_D{ 0x08 };
constexpr WORD KEY_1{ 0x10 };
constexpr WORD KEY_2{ 0x20 };
constexpr WORD KEY_3{ 0x40 };
constexpr WORD KEY_4{ 0x80 };
constexpr WORD KEY_E{ 0x100 };
constexpr WORD KEY_LSHIFT{ 0x200 };
constexpr WORD KEY_LBUTTON{ 0x400 };
constexpr WORD KEY_RBUTTON{ 0x800 };

class CPlayer;

enum class SOCKET_STATE
{
	SEND_KEY_BUFFER,
	SEND_GAME_START,
	SEND_CHANGE_SLOT
};

enum RECV_HEAD
{
	HEAD_INIT = 0,
	HEAD_UPDATE_DATA,
	HEAD_NUM_OF_CLIENT,
	HEAD_BLUE_SUIT_WIN,
	HEAD_ZOMBIE_WIN,
	HEAD_GAME_START,
	HEAD_CHANGE_SLOT
};

struct CS_ANIMATION_INFO {
	float pitch = 0.0f;
};

struct CS_PLAYER_INFO {
	RightItem m_selectItem;
	bool m_bRightClick = false;

	int m_iMineobjectNum = -1;
	bool m_bAttacked = false;

	int m_iEscapeDoor = -1;
};

struct CS_CLIENTS_INFO
{
	INT8 m_nClientId = -1;
	bool m_bAlive = true;
	bool m_bRunning = false;
	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3Velocity;
	XMFLOAT3 m_xmf3Look;
	int m_nPickedObjectNum = -1;

	int m_nSlotObjectNum[3];	// 각 슬롯에 포함된 오브젝트 번호(없으면 -1)
	int m_nFuseObjectNum[3];	// 퓨즈 오브젝트 번호(없으면 -1)

	int m_nNumOfObject = -1;
	std::array<int, MAX_RECV_OBJECT_INFO> m_anObjectNum;
	std::array<XMFLOAT4X4, MAX_RECV_OBJECT_INFO> m_axmf4x4World;

	CS_ANIMATION_INFO m_animationInfo;
	CS_PLAYER_INFO m_playerInfo;
};

void ConvertLPWSTRToChar(LPWSTR lpwstr, char* dest, int destSize);
void ConvertCharToLPWSTR(const char* pstr, LPWSTR dest, int destSize);

class CTcpClient
{
private:
	INT8 m_nMainClientId = -1;
	INT8 m_nClient = -1;				// 클라이언트 수

	INT8 m_nSelectedSlot = -1;

	bool m_bRecvDelayed = false;	// 오는 데이터를 전부 받지 못했다
	bool m_bRecvHead = false;

	int m_nCurrentRecvByte = 0;		// 현재까지 받은 데이터의 길이
	char m_pCurrentBuffer[BUFSIZE];

	SOCKET_STATE m_socketState = SOCKET_STATE::SEND_GAME_START;
	SOCKET_STATE m_prevSocketState = SOCKET_STATE::SEND_GAME_START;

	// 서버 접속 소켓

	// 서버에 접속한 클라이언트의 정보 <아이디,정보>
	std::array<CS_CLIENTS_INFO, MAX_CLIENT> m_aClientInfo;
	std::array<shared_ptr<CPlayer>, MAX_CLIENT> m_apPlayers;

	int m_nEscapeDoor = -1;
public:
	SOCKET m_sock;
	bool m_bSend = true;

	int SendNum = 0;
	int RecvNum = 0;
	CTcpClient();
	~CTcpClient();

	bool CreateSocket(HWND hWnd, TCHAR* pszIPAddress);
	void OnDestroy();
	void SetPlayer(const shared_ptr<CPlayer>& pPlayer, int nIndex = 0) { m_apPlayers[nIndex] = pPlayer; };

	// 이벤트를 처리한다.
	void OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingReadMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void UpdateDataFromServer();
	void UpdatePickedObject(int i);
	void OnProcessingWriteMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	template<class... Args>
	void CreateSendDataBuffer(char* pBuffer, Args&&... args);
	template<class... Args>
	int SendData(SOCKET socket, size_t nBufferSize, Args&&... args);
	int RecvData(SOCKET socket, size_t nBufferSize);

	void UpdateKeyBitMask(UCHAR* pKeysBuffer, WORD& wKeyBuffer);

	void UpdateZombiePlayer();
	void UpdatePlayer(int nIndex);

	//Interface
	INT8 GetMainClientId() const { return m_nMainClientId; }
	INT8 GetClientID(int nIndex) { return m_aClientInfo[nIndex].m_nClientId; }
	INT8 GetNumOfClient() const { return m_nClient; }
	XMFLOAT3 GetPostion(int id);
	std::array<CS_CLIENTS_INFO, 5>& GetArrayClientsInfo();
	//SOCKET GetSocket() { return m_sock; }

	int GetEscapeDoor() const { return m_nEscapeDoor; }

	void SetSelectedSlot(INT8 nSelectedSlot) { m_nSelectedSlot = nSelectedSlot; }
	void SetSocketState(SOCKET_STATE sockState) { m_socketState = sockState; }
};

