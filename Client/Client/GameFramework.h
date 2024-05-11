#pragma once
#include "Timer.h"
#include "Scene.h"
#include "TCPClient.h"

//constexpr size_t SWAPCHAIN_BUFFER_NUM = 2;
//class TextObject;

constexpr UINT WM_CREATE_TCP{ WM_USER + 2 };
constexpr UINT BUTTON_CREATE_TCP_ID{ 1 };
constexpr UINT EDIT_INPUT_ADDRESS_ID{ 2 };

struct FrameTimeInfo {
	float time = 0.0f;
	float localTime = 0.0f;
	float usePattern = -1.0f; // shaders에서 패턴텍스처를 사용하는가? 0보다 큰값이면 사용하는 것. 최적화 필요. 쉐이더를 나누면 분기문 줄일수있음.

	float fTrackingTime = 0.0f;
};

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();
	void OnDestroyLobby();

	void CreateLobby(HWND hWnd);

	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();

	void CreateRtvAndDsvDescriptorHeaps();

	void CreateRenderTargetViews();
	void CreateDepthStencilView();

	void ChangeSwapChainState();

	void BuildObjects();
	void ReleaseObjects();

	void ProcessInput();
	void AnimateObjects();
	//void ProcessCollide();
	void PreRenderTasks();
	void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingCommandMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	INT8 GetClientIdFromTcpClient() const { return m_pTcpClient->GetClientId(); }

	static UCHAR* GetKeysBuffer();
	static int GetMainClientId() { return m_nMainClientId; }
	void SetPlayerObjectOfClient(int nClientId);

	void SetConnected(bool bConnected) { m_bConnected = bConnected; }
	bool IsConnected() const { return m_bConnected; }

	bool IsTcpClient() const { return m_bTcpClient; }
	void OnButtonClick(HWND hWnd);

	void SetMousePoint(POINT ptMouse) { m_ptOldCursorPos = ptMouse; }
private:

	D3D12_VIEWPORT m_d3dViewport;
	D3D12_RECT m_d3dScissorRect;
	//뷰포트와 씨저 사각형이다.

	HINSTANCE							m_hInstance;
	HWND								m_hWnd;

	int									m_nWndClientWidth;
	int									m_nWndClientHeight;

	ComPtr<IDXGIFactory4>				m_dxgiFactory;
	ComPtr<IDXGISwapChain3>				m_dxgiSwapChain;
	ComPtr<ID3D12Device>				m_d3d12Device;
	bool								m_bMsaa4xEnable = false;
	UINT								m_nMsaa4xQualityLevels = 0;

	static const UINT					m_nSwapChainBuffers = 2;
	UINT								m_nSwapChainBufferIndex;

	std::array<ComPtr<ID3D12Resource>, m_nSwapChainBuffers>			m_d3dSwapChainBackBuffers;
	ComPtr<ID3D12DescriptorHeap>									m_d3dRtvDescriptorHeap;
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, m_nSwapChainBuffers>	m_pd3dSwapChainBackBufferRTVCPUHandles;

	ComPtr<ID3D12Resource>				m_d3dDepthStencilBuffer;
	ComPtr<ID3D12DescriptorHeap>		m_d3dDsvDescriptorHeap;

	ComPtr<ID3D12CommandAllocator>		m_d3dCommandAllocator[m_nSwapChainBuffers];
	ComPtr<ID3D12CommandQueue>			m_d3dCommandQueue;
	ComPtr<ID3D12GraphicsCommandList>	m_d3dCommandList;

	ComPtr<ID3D12Fence>						m_d3dFence;
	std::array<UINT64, m_nSwapChainBuffers>	m_nFenceValues;
	HANDLE									m_hFenceEvent;

#if defined(_DEBUG)
	ID3D12Debug*						m_pd3dDebugController;
#endif

	CGameTimer							m_GameTimer;
	
	shared_ptr<CScene>					m_pScene;

	std::shared_ptr<CPlayer>					m_pMainPlayer;	// 클라이언트ID에 해당하는 인덱스가 해당 클라이언트의 Main플레이어로 설정된다
	std::array<shared_ptr<CPlayer>, MAX_CLIENT>	m_apPlayer;		// 클라이언트ID와 인덱스는 동일하다.
	weak_ptr<CCamera>							m_pCamera;

	CPostProcessingShader*				m_pPostProcessingShader = NULL;

	POINT								m_ptOldCursorPos;
	_TCHAR								m_pszFrameRate[200];
	
	static UCHAR						m_pKeysBuffer[256];

	//TCPClient
	unique_ptr<CTcpClient>				m_pTcpClient;
	static int							m_nMainClientId;	// TcpClient에서 받게 된다. -> 플레이어 1인칭으로 그릴때 비교해서 그려주게 하기위해
public:
	void PrepareDrawText();
	void RenderUI();

private:
	//DrawText
	ComPtr<ID3D11DeviceContext> m_d3d11DeviceContext;
	ComPtr<ID3D11On12Device> m_d3d11On12Device;
	ComPtr<IDWriteFactory> m_dWriteFactory;
	ComPtr<ID3D11Resource> m_wrappedBackBuffers[m_nSwapChainBuffers];
	ComPtr<ID2D1Factory3> m_d2dFactory;
	ComPtr<ID2D1Device2> m_d2dDevice;
	ComPtr<ID2D1Bitmap1> m_d2dRenderTargets[m_nSwapChainBuffers];
	ComPtr<ID2D1DeviceContext2> m_d2dDeviceContext;

	ComPtr<ID2D1SolidColorBrush> m_textBrush;
	ComPtr<IDWriteTextFormat> m_textFormat;

	//unique_ptr<TextObject> m_pTextobject;
	bool m_bPrevRender = false;
public:
	// Time 
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dTimeCbvGPUDescriptorHandle;
	ComPtr<ID3D12Resource>		m_pd3dcbTime;
	FrameTimeInfo* m_pcbMappedTime;

//[0507]
private:
	bool m_bConnected = false;
	HWND m_hConnectButton;

	bool m_bTcpClient = false;
	UINT m_nEventCreateTcpClient;

	HWND m_hIPAddressEdit;

	_TCHAR m_pszIPAddress[16];
};

