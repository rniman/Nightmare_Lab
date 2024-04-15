#pragma once
#include "Timer.h"
#include "Scene.h"
#include "TCPClient.h"

//constexpr size_t SWAPCHAIN_BUFFER_NUM = 2;
//class TextObject;

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

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
	void ProcessCollide();
	void PreRenderTasks();
	void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	
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
	shared_ptr<CPlayer>					m_pPlayer;
	weak_ptr<CCamera>					m_pCamera;

	CPostProcessingShader*				m_pPostProcessingShader = NULL;

	POINT								m_ptOldCursorPos;
	_TCHAR								m_pszFrameRate[70];
	//TCPClient
	TCPClient* m_pClientNetwork = NULL;

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
};

