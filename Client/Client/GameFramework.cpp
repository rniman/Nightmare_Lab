//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "GameFramework.h"
#include "Player.h"
#include "Collision.h"

 extern UINT gnCbvSrvDescriptorIncrementSize;
 extern UINT gnRtvDescriptorIncrementSize;
 extern UINT gnDsvDescriptorIncrementSize;

 UCHAR CGameFramework::m_pKeysBuffer[256] = {};
 int CGameFramework::m_nMainClientId = -1;

 CGameFramework::CGameFramework()
{
	m_nSwapChainBufferIndex = 0;

	m_hFenceEvent = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_d3dViewport = { 0,0,FRAME_BUFFER_WIDTH,FRAME_BUFFER_HEIGHT,0.0f,1.0f };
	m_d3dScissorRect = { 0,0,FRAME_BUFFER_WIDTH,FRAME_BUFFER_HEIGHT };

	m_pScene = NULL;

	m_pTcpClient = make_unique<CTcpClient>();

	_tcscpy_s(m_pszFrameRate, _T("NightMare Lab ("));
}

CGameFramework::~CGameFramework()
{
#ifndef SINGLE_PLAY
	m_pClientNetwork->Exit();
#endif // SINGLE_PLAY
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateDepthStencilView();

#ifdef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	ChangeSwapChainState();
#endif

	CoInitialize(NULL);

	//m_pTcpClient = make_shared<CTcpClient>(hMainWnd);

	//g_collisionManager.CreateCollision(SPACE_FLOOR, SPACE_WIDTH, SPACE_DEPTH);
	BuildObjects();
	//[0514] PrepaerDrawText-> 창모드 전환에 문제있음
	//PrepareDrawText();// Scene이 초기화 되고 나서 수행해야함 SRV를 Scene이 가지고 있음.

	return(true);
}
//#define _WITH_CREATE_SWAPCHAIN_FOR_HWND

void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	//m_nWndClientWidth = rcClient.right - rcClient.left;
	//m_nWndClientHeight = rcClient.bottom - rcClient.top;

#ifdef _WITH_CREATE_SWAPCHAIN_FOR_HWND
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	HRESULT hResult = m_dxgiFactory->CreateSwapChainForHwnd(m_d3dCommandQueue.Get(), m_hWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1**)m_dxgiSwapChain.GetAddressOf());
#else
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hResult = m_dxgiFactory->CreateSwapChain(m_d3dCommandQueue.Get(), &dxgiSwapChainDesc, (IDXGISwapChain**)m_dxgiSwapChain.GetAddressOf());
#endif
	m_nSwapChainBufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();

	hResult = m_dxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}

void CGameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug* pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void**)m_dxgiFactory.GetAddressOf());

	IDXGIAdapter1* pd3dAdapter = NULL;

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_dxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)m_d3d12Device.GetAddressOf()))) break;
	}

	if (!pd3dAdapter)
	{
		m_dxgiFactory->EnumWarpAdapter(_uuidof(IDXGIFactory4), (void**)&pd3dAdapter);
		hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)m_d3d12Device.GetAddressOf());
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	hResult = m_d3d12Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	hResult = m_d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)m_d3dFence.GetAddressOf());
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	::gnCbvSrvDescriptorIncrementSize = m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::gnRtvDescriptorIncrementSize = m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	if (pd3dAdapter) pd3dAdapter->Release();
}

void CGameFramework::CreateCommandQueueAndList()
{
	HRESULT hResult;

	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = m_d3d12Device->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void**)m_d3dCommandQueue.GetAddressOf());

	//hResult = m_d3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)m_d3dCommandAllocator.GetAddressOf());

	for (int i = 0;i < m_nSwapChainBuffers;++i) {
		hResult = m_d3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)m_d3dCommandAllocator[i].GetAddressOf());
		//ThrowIfFailed(m_d3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_d3dCommandAllocator[i])));
	}
	hResult = m_d3d12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,m_d3dCommandAllocator[m_nSwapChainBufferIndex].Get(), NULL, __uuidof(ID3D12GraphicsCommandList), (void**)m_d3dCommandList.GetAddressOf());
	hResult = m_d3dCommandList->Close();
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers + ADD_RENDERTARGET_COUNT + 2 ;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_d3d12Device->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)m_d3dRtvDescriptorHeap.GetAddressOf());
	::gnRtvDescriptorIncrementSize = m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_d3d12Device->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)m_d3dDsvDescriptorHeap.GetAddressOf());
	::gnDsvDescriptorIncrementSize = m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateRenderTargetViews()
{
	HRESULT hResult;

	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		hResult = m_dxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)m_d3dSwapChainBackBuffers[i].GetAddressOf());
		m_d3d12Device->CreateRenderTargetView(m_d3dSwapChainBackBuffers[i].Get(), &d3dRenderTargetViewDesc, d3dRtvCPUDescriptorHandle);
		m_pd3dSwapChainBackBufferRTVCPUHandles[i] = d3dRtvCPUDescriptorHandle;
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
}

void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	m_d3d12Device->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_COMMON, &d3dClearValue, __uuidof(ID3D12Resource), (void**)m_d3dDepthStencilBuffer.GetAddressOf());

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_d3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3d12Device->CreateDepthStencilView(m_d3dDepthStencilBuffer.Get(), &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);
}

void CGameFramework::ChangeSwapChainState()
{
	WaitForGpuComplete();

	HRESULT hResult;
	BOOL bFullScreenState = FALSE;
	hResult = m_dxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	hResult = m_dxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	hResult = m_dxgiSwapChain->GetDesc(&dxgiSwapChainDesc);

	DXGI_MODE_DESC dxgiTargetParameters;
	::ZeroMemory(&dxgiTargetParameters, sizeof(dxgiTargetParameters));
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	hResult = m_dxgiSwapChain->ResizeTarget(&dxgiSwapChainDesc.BufferDesc);

	for (int i = 0; i < m_nSwapChainBuffers; i++)
	{
		if (m_d3dSwapChainBackBuffers[i])
		{
			m_d3dSwapChainBackBuffers[i].Reset();
		}
	}

	//[0514] 백버퍼 참조 없애야함
	if (m_bPrepareDrawText)
	{
		m_d3d11DeviceContext.Reset();
		m_d3d11On12Device.Reset();
		m_dWriteFactory.Reset();
		m_wrappedBackBuffers[0].Reset();
		m_wrappedBackBuffers[1].Reset();
		m_d2dFactory.Reset();
		m_d2dDevice.Reset();
		m_d2dRenderTargets[0].Reset();
		m_d2dRenderTargets[1].Reset();
		m_d2dDeviceContext.Reset();

		m_textBrush.Reset();
		m_textFormat.Reset();
	}

	hResult = m_dxgiSwapChain->ResizeBuffers(2, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);

	m_nSwapChainBufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();

	//[0514] 백버퍼 참조 없애야함
	if (m_bPrepareDrawText)
	{
		PrepareDrawText();
	}
}

void CGameFramework::PrepareDrawText()
{
	D2D1_FACTORY_OPTIONS d2dFactoryOptions = {}; //drawText
	d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;

	//DrawText
	ComPtr<ID3D11Device> d3d11Device;
	D3D11On12CreateDevice(
		m_d3d12Device.Get(),
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		nullptr,
		0,
		reinterpret_cast<IUnknown**>(m_d3dCommandQueue.GetAddressOf()),
		1,
		0,
		d3d11Device.GetAddressOf(),
		m_d3d11DeviceContext.GetAddressOf(),
		nullptr
	);

	ThrowIfFailed(d3d11Device.As(&m_d3d11On12Device));
	{
		D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
		ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, &m_d2dFactory));
		ComPtr<IDXGIDevice> dxgiDevice;
		ThrowIfFailed(m_d3d11On12Device.As(&dxgiDevice));
		ThrowIfFailed(m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice));
		ThrowIfFailed(m_d2dDevice->CreateDeviceContext(deviceOptions, &m_d2dDeviceContext));
		ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_dWriteFactory));
	}

	float dpiX;
	float dpiY;

#pragma warning(push)
#pragma warning(disable : 4996) // GetDesktopDpi is deprecated.
	m_d2dFactory->GetDesktopDpi(&dpiX, &dpiY);
#pragma warning(pop)
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
		dpiX,
		dpiY
	);

	//[CJI 0412] 텍스트를 렌더타겟에 그리고 이를 텍스처로 바꾼이후 다른 사물에 매핑하려고 했으나 실패(시간너무 끌어서 패스).. 나중에 한번 해보자. 
	//m_pTextobject = make_unique<TextObject>(m_d3d12Device.Get(), m_d3dCommandList.Get(), m_d3dSwapChainBackBuffers);

	for (UINT n = 0; n < m_nSwapChainBuffers; n++)
	{
		D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
		ThrowIfFailed(m_d3d11On12Device->CreateWrappedResource(
			m_d3dSwapChainBackBuffers[n].Get(),
			&d3d11Flags,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT,
			IID_PPV_ARGS(&m_wrappedBackBuffers[n])
		));

		ComPtr<IDXGISurface> surface;
		ThrowIfFailed(m_wrappedBackBuffers[n].As(&surface));
		ThrowIfFailed(m_d2dDeviceContext->CreateBitmapFromDxgiSurface(
			surface.Get(),
			&bitmapProperties,
			&m_d2dRenderTargets[n]
		));

	}

	{
		ThrowIfFailed(m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Cyan), &m_textBrush));
		ThrowIfFailed(m_dWriteFactory->CreateTextFormat(
			L"Verdana",
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			50,
			L"en-us",
			&m_textFormat
		));
		ThrowIfFailed(m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
		ThrowIfFailed(m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
	}

	m_bPrepareDrawText = true;
}

//float uiX{}, uiY{};

void CGameFramework::RenderUI()
{
	if (m_nGameState != GAME_STATE::IN_GAME)
	{
		return;
	}

	D2D1_SIZE_F rtSize = m_d2dRenderTargets[m_nSwapChainBufferIndex]->GetSize();
	D2D1_RECT_F textRect = D2D1::RectF(0, 0, rtSize.width, rtSize.height);

	auto player = dynamic_pointer_cast<CBlueSuitPlayer>(m_pMainPlayer);
	if (!player) { //좀비 플레이어라면 레이더 텍스트 렌더링 X
		m_d3d11On12Device->AcquireWrappedResources(m_wrappedBackBuffers[m_nSwapChainBufferIndex].GetAddressOf(), 1);
		m_d3d11On12Device->ReleaseWrappedResources(m_wrappedBackBuffers[m_nSwapChainBufferIndex].GetAddressOf(), 1);
		m_d3d11DeviceContext->Flush();
		return;
	}
	float escapelength = player->GetEscapeLength();

	wchar_t text[20]; // 변환된 유니코드 문자열을 저장할 버퍼

	// 부동 소수점 값을 문자열로 변환 후 유니코드 문자열로 저장
	int len = swprintf(text, 20, L"%d", (int)escapelength);
	text[len] = 'M'; 
	len += 1;
	text[len+1] = '\0';
	
	//static const WCHAR text[] = buffer;
	
	// 현재 백 버퍼에 대한 래핑된 렌더 타겟 자원을 획득합니다.
	m_d3d11On12Device->AcquireWrappedResources(m_wrappedBackBuffers[m_nSwapChainBufferIndex].GetAddressOf(), 1);
	m_d2dDeviceContext->SetTarget(m_d2dRenderTargets[m_nSwapChainBufferIndex].Get());
	m_d2dDeviceContext->BeginDraw();
	if (dynamic_pointer_cast<CBlueSuitPlayer>(m_pMainPlayer)->PlayRaiderUI()) {

		D2D1::Matrix3x2F mat = D2D1::Matrix3x2F::Identity();
		static D2D1_POINT_2F point = { 455.f,180.f };
		mat = mat.Translation(point.x, point.y);
		m_d2dDeviceContext->SetTransform(mat);
		m_d2dDeviceContext->DrawText(
			text,
			/*_countof(text)*/len,
			m_textFormat.Get(),
			&textRect,
			m_textBrush.Get()
		);

	}
	ThrowIfFailed(m_d2dDeviceContext->EndDraw());
	// 래핑된 렌더 타겟 자원을 해제합니다. 해제하면 래핑된 자원이 생성될 때 지정된 OutState로 백 버퍼 자원이 전환됩니다
	m_d3d11On12Device->ReleaseWrappedResources(m_wrappedBackBuffers[m_nSwapChainBufferIndex].GetAddressOf(), 1);

	//명령 목록을 공유 명령 큐에 제출하기 위해 플러시합니다.
	m_d3d11DeviceContext->Flush();
}


void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (!m_bConnected)	// 서버와 연결 X
	{

		return;
	}

	if (m_pScene) m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);

	if (m_nGameState == GAME_STATE::IN_LOBBY)
	{
		return;
	}

	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		SetCursor(NULL);
		SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (!m_bConnected)
	{
		switch (nMessageID)
		{
		case WM_KEYUP:
			switch (wParam)
			{
			case VK_ESCAPE:
				::PostQuitMessage(0);
				break;
			default:
				break;
			}
			break;
		default:
			break;

		}
		return;
	}

	if (m_pScene) m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_CONTROL:	// 캡쳐 해제
			if (GetCapture())
			{
				SetCursor(LoadCursor(NULL, IDC_ARROW));
				::ReleaseCapture();
			}
			else
			{
				SetCursor(NULL);
				SetCapture(hWnd);
				SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
			}
			break;
		case VK_RETURN:
			break;
		case VK_F1:
		case VK_F2:
			if (!m_pMainPlayer)
			{
				break;
			}
			m_pMainPlayer->ChangeCamera((DWORD)(wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
			m_pCamera = m_pMainPlayer->GetCamera();
			break;
		case VK_F3:
			break;
		case VK_F9:
			ChangeSwapChainState();
			break;
		//case VK_PRIOR:
		//{
		//	if (!m_pMainPlayer)
		//	{
		//		break;
		//	}
		//	XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		//	XMFLOAT3 xmf3Up = m_pMainPlayer->GetUpVector();
		//	if (m_pMainPlayer->GetPosition().y < 13.5f + FLT_EPSILON) xmf3Shift = Vector3::Add(xmf3Shift, xmf3Up, 4.5f);

		//	m_pMainPlayer->SetPosition(Vector3::Add(m_pMainPlayer->GetPosition(), xmf3Shift));
		//}
		//	break;
		//case VK_NEXT:
		//{
		//	if (!m_pMainPlayer)
		//	{
		//		break;
		//	}
		//	XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		//	XMFLOAT3 xmf3Up = m_pMainPlayer->GetUpVector();
		//	if (m_pMainPlayer->GetPosition().y > 0.0f + FLT_EPSILON) xmf3Shift = Vector3::Add(xmf3Shift, xmf3Up, -4.5f);

		//	m_pMainPlayer->SetPosition(Vector3::Add(m_pMainPlayer->GetPosition(), xmf3Shift));
		//}
		//	break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void CGameFramework::OnProcessingCommandMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_COMMAND:
		if (LOWORD(wParam) == BUTTON_CREATE_TCP_ID)
		{
			OnButtonClick(hWnd);
		}
		break;
	}
}

void CGameFramework::OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	// 오류 발생 여부 확인
	if (WSAGETSELECTERROR(lParam))
	{
		err_display(WSAGETSELECTERROR(lParam));
		return;
	}

	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_WRITE:	// 소켓이 데이터를 전송할 준비가 되었다.
	case FD_READ:	// 소켓이 데이터를 읽을 준비가 되었다.
	case FD_CLOSE:
		m_pTcpClient->OnProcessingSocketMessage(hWnd, nMessageID, wParam, lParam);
		if (WSAGETSELECTEVENT(lParam) == FD_CLOSE && m_bTcpClient)
		{
			m_bTcpClient = false;
			err_display("Fail Connect", "Client count exceeded or game already started");
		}
		break;
	default:
		break;
	}
}

void CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_COMMAND:
		OnProcessingCommandMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_CREATE_TCP:
		m_bTcpClient = true;
		break;
	case WM_START_GAME:
	{
		m_nGameState = GAME_STATE::IN_GAME;
		BuildObjects();

		::SetCursor(NULL);
		::SetCapture(hWnd);
		// 마우스를 화면 중앙으로 이동시킴 (윈도우 내부로만 이동하도록)
		RECT rect;
		GetClientRect(hWnd, &rect);
		POINT center = { (rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2 };
		ClientToScreen(hWnd, &center);
		SetCursorPos(center.x, center.y);
		SetMousePoint(center);
	}
		break;
	case WM_END_GAME:
		OnProcessingEndGameMessage(wParam);
		break;
	case WM_SOCKET:
		OnProcessingSocketMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
			m_GameTimer.Stop();
		else
			m_GameTimer.Start();
		break;
	}
	case WM_SIZE:
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
}

void CGameFramework::OnProcessingEndGameMessage(WPARAM& wParam)
{
	if (LOWORD(wParam) == 0)	// BLUESUIT WIN
	{
		m_nGameState = GAME_STATE::BLUE_SUIT_WIN;
	}
	else if (LOWORD(wParam) == 1)	// ZOMBIE WIN
	{
		m_nGameState = GAME_STATE::ZOMBIE_WIN;
		for (int i = ZOMBIEPLAYER + 1; i < ZOMBIEPLAYER + 1 + 4; ++i)
		{
			m_apPlayer[i]->SetAlive(false);
		}
	}

	shared_ptr<CMainScene> pMainScene = dynamic_pointer_cast<CMainScene>(m_pScene);
	if (m_nMainClientId != ZOMBIEPLAYER)
	{
		dynamic_cast<CBlueSuitUserInterfaceShader*>(pMainScene->m_vForwardRenderShader[USER_INTERFACE_SHADER].get())->SetGameState(m_nGameState);
	}
	else if (m_nMainClientId == ZOMBIEPLAYER)
	{
		dynamic_cast<CZombieUserInterfaceShader*>(pMainScene->m_vForwardRenderShader[USER_INTERFACE_SHADER].get())->SetGameState(m_nGameState);
	}
}

UCHAR* CGameFramework::GetKeysBuffer()
{
	return m_pKeysBuffer; 
}

void CGameFramework::SetPlayerObjectOfClient(int nClientId)
{
	m_nMainClientId = nClientId;
	m_pMainPlayer = m_apPlayer[nClientId];
	m_pMainPlayer->GetCamera()->SetPlayer(m_pMainPlayer);
	m_pCamera = m_pMainPlayer->GetCamera();
	g_collisionManager.m_pPlayer = m_pMainPlayer;

	m_pScene->SetMainPlayer(m_pMainPlayer);
}

void CGameFramework::OnButtonClick(HWND hWnd)
{
	GetWindowText(m_hIPAddressEdit, m_pszIPAddress, 20);

	if (m_pTcpClient->CreateSocket(hWnd, m_pszIPAddress))
	{
		SendMessage(hWnd, WM_CREATE_TCP, NULL, NULL);
	}
}

void CGameFramework::OnDestroy()
{
	ReleaseObjects();

	::CloseHandle(m_hFenceEvent);

	if(m_dxgiSwapChain)
	{
		m_dxgiSwapChain->SetFullscreenState(FALSE, NULL);
	}

#if defined(_DEBUG)
	IDXGIDebug1* pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif

}

void CGameFramework::OnDestroyEntryWindow()
{
	DestroyWindow(m_hConnectButton);
	DestroyWindow(m_hIPAddressEdit);
}

void CGameFramework::CreateEntryWindow(HWND hWnd)
{
	int nConnectButtonWidth = 300;
	int nConnectButtonHeight= 60;

	m_hConnectButton = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"BUTTON", // 클래스 이름
		L"Connect to Server", // 텍스트
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // 스타일
		FRAME_BUFFER_WIDTH / 2 - nConnectButtonWidth / 2, FRAME_BUFFER_HEIGHT / 2 + nConnectButtonHeight, nConnectButtonWidth, nConnectButtonHeight, // 위치와 크기
		hWnd, // 부모 윈도우
		(HMENU)BUTTON_CREATE_TCP_ID, // 버튼 ID
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL
	);

	int nIPEditWidth = 400;
	int nIPEditHeight = 50;
	m_hIPAddressEdit = CreateWindowEx(
		WS_EX_CLIENTEDGE,      
		L"EDIT",                
		L"",      
		WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER,
		FRAME_BUFFER_WIDTH / 2 - nIPEditWidth / 2, FRAME_BUFFER_HEIGHT / 2, nIPEditWidth, nIPEditHeight,
		hWnd,                
		(HMENU)EDIT_INPUT_ADDRESS_ID,       
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),      
		NULL                
	);
	SendMessage(m_hIPAddressEdit, EM_LIMITTEXT, (WPARAM)16, 0);	// 16글자 제한

	
	HFONT hFont = CreateFont(40, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	SendMessage(m_hIPAddressEdit, WM_SETFONT, (WPARAM)hFont, TRUE);


	int nIPCaptionWidth = 400;
	int nIPCaptionHeight = 40;
	HWND hLabel = CreateWindowEx(
		0,
		L"STATIC",
		L"IP ADDRESS",
		WS_CHILD | WS_VISIBLE | SS_CENTER,
		FRAME_BUFFER_WIDTH / 2 - nIPCaptionWidth / 2, FRAME_BUFFER_HEIGHT / 2 - nIPCaptionHeight, nIPCaptionWidth, nIPCaptionHeight,
		hWnd,
		(HMENU)EDIT_INPUT_ADDRESS_ID,
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL
	);
	SendMessage(hLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void CGameFramework::BuildObjects()
{
	m_GameTimer.Reset();

	m_d3dCommandList->Reset(m_d3dCommandAllocator[m_nSwapChainBufferIndex].Get(), NULL);
	if (m_nGameState == GAME_STATE::IN_LOBBY)
	{
		m_pScene = make_shared<CLobbyScene>(m_hWnd, m_pCamera);
		m_pScene->SetNumOfSwapChainBuffers(m_nSwapChainBuffers);
		m_pScene->SetRTVDescriptorHeap(m_d3dRtvDescriptorHeap);

		int nMainClientId = m_pTcpClient->GetMainClientId();
		m_pScene->BuildObjects(m_d3d12Device.Get(), m_d3dCommandList.Get(), nMainClientId);
		m_pCamera.lock()->CreateShaderVariables(m_d3d12Device.Get(), m_d3dCommandList.Get());

		for (int i = 0; i < MAX_CLIENT; ++i)
		{
			m_apPlayer[i] = m_pScene->m_apPlayer[i];
			m_pTcpClient->SetPlayer(m_pScene->m_apPlayer[i], i);
			int nClientId = m_pTcpClient->GetClientID(i);
			m_apPlayer[i]->SetClientId(nClientId);
		}		
		m_nMainClientId = nMainClientId;
		m_pMainPlayer = m_apPlayer[nMainClientId];
		m_pScene->SetMainPlayer(m_pMainPlayer);

		m_d3dCommandList->Close();
		ID3D12CommandList* ppd3dCommandLists[] = { m_d3dCommandList.Get() };
		m_d3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

		WaitForGpuComplete();
	}
	else if (m_nGameState == GAME_STATE::IN_GAME)
	{
		g_collisionManager.CreateCollision(SPACE_FLOOR, SPACE_WIDTH, SPACE_DEPTH);

		m_pScene = make_shared<CMainScene>();
		m_pScene->SetNumOfSwapChainBuffers(m_nSwapChainBuffers);
		m_pScene->SetRTVDescriptorHeap(m_d3dRtvDescriptorHeap);

		shared_ptr<CMainScene> pMainScene = dynamic_pointer_cast<CMainScene>(m_pScene);
		if (m_pScene.get())
		{
			int nMainClientId = m_pTcpClient->GetMainClientId();
			m_pScene->BuildObjects(m_d3d12Device.Get(), m_d3dCommandList.Get(), nMainClientId);

			for (int i = 0; i < MAX_CLIENT; ++i)
			{
				m_apPlayer[i] = m_pScene->m_apPlayer[i];
				m_pTcpClient->SetPlayer(m_pScene->m_apPlayer[i], i);
				int nClientId = m_pTcpClient->GetClientID(i);
				m_apPlayer[i]->SetClientId(nClientId);
			}

			m_nMainClientId = nMainClientId;
			m_pMainPlayer = m_apPlayer[nMainClientId];
			m_pScene->SetMainPlayer(m_pMainPlayer);
		}

		//for (int i = 0; i < MAX_CLIENT; ++i)
		//{
		//	m_apPlayer[i] = m_pScene->m_apPlayer[i];
		//	m_pTcpClient->SetPlayer(m_pScene->m_apPlayer[i], i);
		//}

		////[0626] 포스트 프로세싱 셰이더가 Scene으로 오면서 옮김
		//m_pScene->m_pPostProcessingShader = new CPostProcessingShader();
		//m_pScene->m_pPostProcessingShader->CreateShader(m_d3d12Device.Get(), m_d3dCommandList.Get(), m_pScene->GetGraphicsRootSignature().Get(), 1, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT);
		//
		//D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		//d3dRtvCPUDescriptorHandle.ptr += (::gnRtvDescriptorIncrementSize * m_nSwapChainBuffers);
		//
		//DXGI_FORMAT pdxgiResourceFormats[ADD_RENDERTARGET_COUNT] = { DXGI_FORMAT_R8G8B8A8_UNORM,  DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT ,DXGI_FORMAT_R32G32B32A32_FLOAT };
		//m_pScene->m_pPostProcessingShader->CreateResourcesAndRtvsSrvs(m_d3d12Device.Get(), m_d3dCommandList.Get(), ADD_RENDERTARGET_COUNT, pdxgiResourceFormats, d3dRtvCPUDescriptorHandle); //SRV to (Render Targets) + (Depth Buffer)
		//
		//d3dRtvCPUDescriptorHandle.ptr += (::gnRtvDescriptorIncrementSize * ADD_RENDERTARGET_COUNT);
		//m_pScene->m_pPostProcessingShader->CreateShadowMapResource(m_d3d12Device.Get(), m_d3dCommandList.Get(), m_pScene->m_nLights, d3dRtvCPUDescriptorHandle);
		////D3D12_GPU_DESCRIPTOR_HANDLE d3dDsvGPUDescriptorHandle = CScene::CreateShaderResourceView(m_d3d12Device.Get(), m_d3dDepthStencilBuffer.Get(), DXGI_FORMAT_R32_FLOAT);
		//m_pScene->m_pPostProcessingShader->CreateLightCamera(m_d3d12Device.Get(), m_d3dCommandList.Get(), m_pScene.get());
		//
		////[0523] 이제 좀비 플레이어 외에도 사용, COutLineShader 내부에서 m_pPostProcessingShader->GetDsvCPUDesctriptorHandle(0)을 사용하기위해서 필요
		//dynamic_cast<COutLineShader*>(m_pScene->m_vForwardRenderShader[OUT_LINE_SHADER].get())->SetPostProcessingShader(m_pScene->m_pPostProcessingShader);
		m_d3dCommandList->Close();
		ID3D12CommandList* ppd3dCommandLists[] = { m_d3dCommandList.Get() };
		m_d3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

		WaitForGpuComplete();

		if (m_pScene)
		{
			m_pScene->ReleaseUploadBuffers();
		}

		PreRenderTasks(pMainScene); // 사전 렌더링 작업

		int light_id = 0;
		auto& LightCamera = pMainScene->GetLightCamera();
		for (auto& pPlayer : m_apPlayer)
		{
			pPlayer->ChangeCamera(FIRST_PERSON_CAMERA, 0.0f);
			pPlayer->Update(m_GameTimer.GetTimeElapsed());

			auto survivor = dynamic_pointer_cast<CBlueSuitPlayer>(pPlayer);
			if (survivor) {
				LightCamera[light_id]->SetPlayer(pPlayer);
				light_id++;
			}
		}
		m_pCamera = m_pMainPlayer->GetCamera();
		
		PrepareDrawText();// Scene이 초기화 되고 나서 수행해야함 SRV를 Scene이 가지고 있음.
	}
}

void CGameFramework::ReleaseObjects()
{
	//if (m_pScene) m_pScene->ReleaseObjects();
	//if (m_pScene) delete m_pScene;
}

void CGameFramework::ProcessInput()
{
	bool bProcessedByScene = false;
	GetKeyboardState(m_pKeysBuffer);
	if (!m_pMainPlayer)
	{
		return;
	}

	if (m_nGameState == GAME_STATE::IN_LOBBY)
	{
		m_pScene->ProcessInput(m_pKeysBuffer);
		return;
	}

	//if ( && m_pScene) bProcessedByScene = m_pScene->ProcessInput(m_pKeysBuffer);
	PostMessage(m_hWnd, WM_SOCKET, (WPARAM)m_pTcpClient->m_sock, MAKELPARAM(FD_WRITE, 0));

	if (!bProcessedByScene)
	{
		float cxDelta = 0.0f, cyDelta = 0.0f;
		POINT ptCursorPos;
		if (GetCapture() == m_hWnd)
		{
			GetCursorPos(&ptCursorPos);
			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
			SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}

		if ((cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (cxDelta || cyDelta)
			{

				if (m_pMainPlayer->m_bAlive || !m_pMainPlayer->m_pSkinnedAnimationController->IsAnimation())
				{
					m_pMainPlayer->Rotate(cyDelta, cxDelta, 0.0f);
				}
			}
		}
	}

	for(auto& pPlayer: m_apPlayer)
	{
		if (pPlayer->GetClientId() == -1)
		{
			continue;
		}
		pPlayer->Update(m_GameTimer.GetTimeElapsed());
	}
}

void CGameFramework::AnimateObjects()
{
	float fElapsedTime = m_GameTimer.GetTimeElapsed();

	if (m_pScene) m_pScene->AnimateObjects(fElapsedTime);

	//vector<shared_ptr<CLightCamera>>& lightCamera = m_pScene->GetLightCamera();

	//XMFLOAT3 clientCameraPos = m_pCamera.lock().get()->GetPosition();
	//sort(lightCamera.begin() + 4, lightCamera.end(), [clientCameraPos](const shared_ptr<CLightCamera>& A, const shared_ptr<CLightCamera>& B) {
	//	//const float epsilon = 1e-5f; // 허용 오차
	//	XMFLOAT3 clToA = Vector3::Subtract(clientCameraPos, A->GetPosition());
	//	XMFLOAT3 clToB = Vector3::Subtract(clientCameraPos, B->GetPosition());
	//	return Vector3::Length(clToA) < Vector3::Length(clToB);
	//	});

	//for (auto& cm : lightCamera) {
	//	cm->Update(cm->GetLookAtPosition(), fElapsedTime);
	//	if (auto player = cm->GetPlayer().lock()) {
	//		if (player->GetClientId() == -1) {
	//			cm->m_pLight->m_bEnable = false;
	//		}
	//	}
	//}
}

void CGameFramework::AnimateEnding()
{
	static bool bUpdateElevatorDoor = false;
	shared_ptr<CGameObject> pDoor = g_collisionManager.GetCollisionObjectWithNumber(m_pTcpClient->GetEscapeDoor()).lock();

	if (!bUpdateElevatorDoor)
	{
		g_collisionManager.GetCollisionObjectWithNumber(m_pTcpClient->GetEscapeDoor()).lock()->UpdatePicking();
		bUpdateElevatorDoor = true;
	}
	pDoor->Animate(m_GameTimer.GetTimeElapsed());
	AnimateObjects();

}


void CGameFramework::WaitForGpuComplete()
{
	const UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_d3dCommandQueue->Signal(m_d3dFence.Get(), nFenceValue);

	if (m_d3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_d3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();

	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_d3dCommandQueue->Signal(m_d3dFence.Get(), nFenceValue);

	if (m_d3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_d3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::PreRenderTasks(shared_ptr<CMainScene>& pMainScene)
{
	INT8 nClientId = m_pTcpClient->GetMainClientId();
	if (nClientId != -1)
	{
		SetPlayerObjectOfClient(nClientId);
		//m_bPrevRender = true;
	}
	else	// 에러임
	{
		assert("FAIL CLIENT ID");
	}

	if (pMainScene->m_nLights >= MAX_LIGHTS)
	{
		pMainScene->m_nLights = MAX_LIGHTS;
	}
	m_pMainPlayer->Update(/*m_GameTimer.GetTimeElapsed()*/0.01f);

	AnimateObjects();
	// 이곳에서 렌더링 하기전에 준비작업을 시행하도록한다. ex) 쉐도우맵 베이킹
	// buildobject함수 호출 이후 처리되어야할 작업이다. -> 모든 객체들이 렌더링되어야 그림자맵을 생성함.
	
	//HRESULT hResult = m_d3dCommandAllocator->Reset();
	HRESULT hResult = m_d3dCommandAllocator[m_nSwapChainBufferIndex]->Reset();
	hResult = m_d3dCommandList->Reset(m_d3dCommandAllocator[m_nSwapChainBufferIndex].Get(), NULL);

	SynchronizeResourceTransition(m_d3dCommandList.Get(), m_d3dSwapChainBackBuffers[m_nSwapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * ::gnRtvDescriptorIncrementSize);

	pMainScene->PrevRenderTask(m_d3dCommandList.Get());

	SynchronizeResourceTransition(m_d3dCommandList.Get(), m_d3dSwapChainBackBuffers[m_nSwapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	hResult = m_d3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { m_d3dCommandList.Get() };
	m_d3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	m_dxgiSwapChain->Present(0, 0);

	MoveToNextFrame();
}

//#define _WITH_PLAYER_TOP
void CGameFramework::FrameAdvance()
{
	m_GameTimer.Tick(0.0f);

	if (m_nGameState == GAME_STATE::IN_LOBBY)
	{
		ProcessInput();
		AnimateObjects();
	}
	else if(m_nGameState == GAME_STATE::IN_GAME)
	{
		ProcessInput(); 
		AnimateObjects();
	}
	else
	{
		AnimateEnding();
		m_fEndingElapsedTime += m_GameTimer.GetTimeElapsed();

		m_pMainPlayer->UpdateEnding(m_fEndingElapsedTime, m_nGameState);
	}

	HRESULT hResult = m_d3dCommandAllocator[m_nSwapChainBufferIndex]->Reset();
	hResult = m_d3dCommandList->Reset(m_d3dCommandAllocator[m_nSwapChainBufferIndex].Get(), NULL);

	// 렌더링
	switch (m_nGameState)
	{
	case GAME_STATE::IN_LOBBY:
	{
		SynchronizeResourceTransition(m_d3dCommandList.Get(), m_d3dSwapChainBackBuffers[m_nSwapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_d3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * ::gnRtvDescriptorIncrementSize);

		FLOAT ClearValue[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
		m_d3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
		m_d3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, ClearValue, 0, NULL);
		m_d3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

		m_pScene->Render(m_d3dCommandList.Get(), m_pCamera.lock(), 0);

		SynchronizeResourceTransition(m_d3dCommandList.Get(), m_d3dSwapChainBackBuffers[m_nSwapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}
		break;
	case GAME_STATE::IN_GAME:
	case GAME_STATE::BLUE_SUIT_WIN:
	case GAME_STATE::ZOMBIE_WIN:
	{
		shared_ptr<CMainScene> pMainScene = dynamic_pointer_cast<CMainScene>(m_pScene);

		if (!pMainScene)
		{
			assert("FAIL MAIN SCENE");
		}

		pMainScene->ShadowRender(m_d3dCommandList.Get(), m_pCamera.lock(), 0);

		//그림자맵 생성이 끝났을때의 처리.
		for (auto& pl : m_apPlayer)
		{
			if (pl->GetClientId() == -1)
				continue;
			pl->SetShadowRender(false);
		}

		SynchronizeResourceTransition(m_d3dCommandList.Get(), m_d3dSwapChainBackBuffers[m_nSwapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * ::gnRtvDescriptorIncrementSize);

		D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = pMainScene->m_pPostProcessingShader->GetDsvCPUDesctriptorHandle(0);
		m_d3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
		pMainScene->m_pPostProcessingShader->OnPrepareRenderTarget(m_d3dCommandList.Get(), 0, &m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBufferIndex], &d3dDsvCPUDescriptorHandle);
		pMainScene->FinalRender(m_d3dCommandList.Get(), m_pCamera.lock(), d3dRtvCPUDescriptorHandle, m_nGameState);
	}
		break;
	default:
		break;
	}

	//SynchronizeResourceTransition(m_d3dCommandList.Get(), m_d3dSwapChainBackBuffers[m_nSwapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	//[CJI 0411] -> RenderUI에서 렌더타겟 사용이 끝나면 m_wrappedBackBuffers가 자원을 해제할때 자동적으로 상태를 D3D12_RESOURCE_STATE_PRESENT으로 되돌리기 때문에 불필요

	hResult = m_d3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { m_d3dCommandList.Get()};
	m_d3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	RenderUI();

	WaitForGpuComplete();
	
	m_dxgiSwapChain->Present(0, 0);

	MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 15, 37);
	size_t nLength = _tcslen(m_pszFrameRate);
	XMFLOAT3 xmf3Position = xmf3Position = m_pMainPlayer->GetPosition();
	_stprintf_s(m_pszFrameRate + nLength, 200 - nLength, _T("ID:%d, NumOfClient: %d, (%4f, %4f, %4f), %d"), m_pTcpClient->GetMainClientId(), m_pTcpClient->GetNumOfClient(), xmf3Position.x, xmf3Position.y, xmf3Position.z, g_collisionManager.GetNumOfCollisionObject());
	::SetWindowText(m_hWnd, m_pszFrameRate);
}
