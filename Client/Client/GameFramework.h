#pragma once

#include "Timer.h"
#include "Scene.h"
#include "TCPClient.h"

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

	ComPtr<ID3D12Resource>				m_d3dSwapChainBackBuffers[m_nSwapChainBuffers];
	ComPtr<ID3D12DescriptorHeap>		m_d3dRtvDescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBuffers];


	ComPtr<ID3D12Resource>				m_d3dDepthStencilBuffer;
	ComPtr<ID3D12DescriptorHeap>		m_d3dDsvDescriptorHeap;

	ComPtr<ID3D12CommandAllocator>		m_d3dCommandAllocator;
	ComPtr<ID3D12CommandQueue>			m_d3dCommandQueue;
	ComPtr<ID3D12GraphicsCommandList>	m_d3dCommandList;

	ComPtr<ID3D12Fence>					m_d3dFence;
	UINT64								m_nFenceValues[m_nSwapChainBuffers];
	HANDLE								m_hFenceEvent;

#if defined(_DEBUG)
	ID3D12Debug*						m_pd3dDebugController;
#endif

	CGameTimer							m_GameTimer;

	CScene*								m_pScene = NULL;
	CCamera*							m_pCamera = NULL;
	CPlayer*							m_pPlayer = NULL;

	CPostProcessingShader*				m_pPostProcessingShader = NULL;

	POINT								m_ptOldCursorPos;

	_TCHAR								m_pszFrameRate[70];

	//TCPClient
	TCPClient* m_pClientNetwork = NULL;
};

