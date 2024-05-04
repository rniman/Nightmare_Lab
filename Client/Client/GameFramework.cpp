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

	CoInitialize(NULL);

	//m_pTcpClient = make_shared<CTcpClient>(hMainWnd);

	g_collisionManager.CreateCollision(4, 10, 10);

	BuildObjects();

	PrepareDrawText();// Scene이 초기화 되고 나서 수행해야함 SRV를 Scene이 가지고 있음.
	
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
	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_dxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)m_d3dSwapChainBackBuffers[i].GetAddressOf());
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

	BOOL bFullScreenState = FALSE;
	m_dxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	m_dxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);

	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_dxgiSwapChain->ResizeTarget(&dxgiTargetParameters);

	for (int i = 0; i < m_nSwapChainBuffers; i++)
	{
		if (m_d3dSwapChainBackBuffers[i])
		{
			m_d3dSwapChainBackBuffers[i].Reset();
		}
	}

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_dxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_dxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
	
	m_nSwapChainBufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
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

	//D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =m_d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT n = 0; n < m_nSwapChainBuffers; n++)
	{
		//ThrowIfFailed(m_dxgiSwapChain->GetBuffer(n, IID_PPV_ARGS(&pRenderTargetResource)));
		//m_d3d12Device->CreateRenderTargetView(pRenderTargetResource, &d3dRenderTargetViewDesc, rtvHandle);

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

		//rtvHandle.ptr = rtvHandle.ptr + ::gnRtvDescriptorIncrementSize;

		//ThrowIfFailed(m_d3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_d3dCommandAllocator[n])));
	}

	// Create D2D/DWrite objects for rendering text.
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
}

//float uiX{}, uiY{};

void CGameFramework::RenderUI()
{
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
	if (m_pScene) m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);
		if (!m_pMainPlayer)
		{
			break;
		}
		break;
	case WM_RBUTTONDOWN:
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		::ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScene) m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
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
		case VK_PRIOR:
		{
			if (!m_pMainPlayer)
			{
				break;
			}
			XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
			XMFLOAT3 xmf3Up = m_pMainPlayer->GetUpVector();
			if (m_pMainPlayer->GetPosition().y < 13.5f + FLT_EPSILON) xmf3Shift = Vector3::Add(xmf3Shift, xmf3Up, 4.5f);

			m_pMainPlayer->SetPosition(Vector3::Add(m_pMainPlayer->GetPosition(), xmf3Shift));
		}
			break;
		case VK_NEXT:
		{
			if (!m_pMainPlayer)
			{
				break;
			}
			XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
			XMFLOAT3 xmf3Up = m_pMainPlayer->GetUpVector();
			if (m_pMainPlayer->GetPosition().y > 0.0f + FLT_EPSILON) xmf3Shift = Vector3::Add(xmf3Shift, xmf3Up, -4.5f);

			m_pMainPlayer->SetPosition(Vector3::Add(m_pMainPlayer->GetPosition(), xmf3Shift));
		}
			break;
		//case 'E': //상호작용
		//	if (!m_pMainPlayer)
		//	{
		//		break;
		//	}
		//	if (shared_ptr<CGameObject> pPickedObject = m_pMainPlayer->GetPickedObject().lock())
		//	{
		//		m_pMainPlayer->UpdatePicking();
		//	}
		//	break;
		/*case '1': {
			auto player = dynamic_pointer_cast<CBlueSuitPlayer>(m_pMainPlayer);
			if (player)
			{
				player->UseMine(0);
				player->SetHitEvent();
			}
			auto Zplayer = dynamic_pointer_cast<CZombiePlayer>(m_pMainPlayer);
			if (Zplayer) {
				Zplayer->SetEectricShock();
			}
		}
			break;*/
		//case '2':
		//	//uiY += 10.f;
		//	break;
		//case '3':
		//case '4':
		//	if (!m_pMainPlayer)
		//	{
		//		break;
		//	}
		//	m_pMainPlayer->UseItem(wParam - '1');
		//	break;
		default:
			break;
		}
		break;
	default:
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
		break;
	default:
		break;
	}
}

void CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
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

void CGameFramework::OnDestroy()
{
	ReleaseObjects();

	::CloseHandle(m_hFenceEvent);

	m_dxgiSwapChain->SetFullscreenState(FALSE, NULL);

#if defined(_DEBUG)
	IDXGIDebug1* pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif
}

void CGameFramework::CreateTcpClient(HWND hWnd)
{
	m_pTcpClient = make_unique<CTcpClient>(hWnd);
}

void CGameFramework::BuildObjects()
{
	m_d3dCommandList->Reset(m_d3dCommandAllocator[m_nSwapChainBufferIndex].Get(), NULL);

	m_pScene = make_shared<CScene>();
	if (m_pScene.get())
	{
		int nClientId = m_pTcpClient->GetClientId();
		m_pScene->BuildObjects(m_d3d12Device.Get(), m_d3dCommandList.Get(), nClientId);
	}

	INT ncbElementBytes = ((sizeof(FrameTimeInfo) + 255) & ~255); //256의 배수

	m_pd3dcbTime = ::CreateBufferResource(m_d3d12Device.Get(), m_d3dCommandList.Get(), NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbTime->Map(0, NULL, (void**)&m_pcbMappedTime);
	m_d3dTimeCbvGPUDescriptorHandle = CScene::CreateConstantBufferViews(m_d3d12Device.Get(), 1, m_pd3dcbTime.Get(), ncbElementBytes);

	//m_pPlayer = make_shared<CZombiePlayer>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), nullptr);
	//shared_ptr<CLoadedModelInfo> pZombiePlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), "Asset/Model/Zom_1.bin");
	//m_pPlayer->LoadModelAndAnimation(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), pZombiePlayerModel);
	for(int i = 0; i< MAX_CLIENT;++i)
	{
		m_apPlayer[i] = m_pScene->m_apPlayer[i];
		m_pTcpClient->SetPlayer(m_pScene->m_apPlayer[i], i);
	}

	m_pPostProcessingShader = new CPostProcessingShader();
	m_pPostProcessingShader->CreateShader(m_d3d12Device.Get(), m_d3dCommandList.Get(), m_pScene->GetGraphicsRootSignature().Get(), 1, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT);
	
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (::gnRtvDescriptorIncrementSize * m_nSwapChainBuffers);

	DXGI_FORMAT pdxgiResourceFormats[ADD_RENDERTARGET_COUNT] = { DXGI_FORMAT_R8G8B8A8_UNORM,  DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT ,DXGI_FORMAT_R32G32B32A32_FLOAT };
	m_pPostProcessingShader->CreateResourcesAndRtvsSrvs(m_d3d12Device.Get(), m_d3dCommandList.Get(), ADD_RENDERTARGET_COUNT, pdxgiResourceFormats, d3dRtvCPUDescriptorHandle); //SRV to (Render Targets) + (Depth Buffer)

	d3dRtvCPUDescriptorHandle.ptr += (::gnRtvDescriptorIncrementSize * ADD_RENDERTARGET_COUNT);
	m_pPostProcessingShader->CreateShadowMapResource(m_d3d12Device.Get(), m_d3dCommandList.Get(),m_pScene->m_nLights, d3dRtvCPUDescriptorHandle);
	//D3D12_GPU_DESCRIPTOR_HANDLE d3dDsvGPUDescriptorHandle = CScene::CreateShaderResourceView(m_d3d12Device.Get(), m_d3dDepthStencilBuffer.Get(), DXGI_FORMAT_R32_FLOAT);
	m_pPostProcessingShader->CreateLightCamera(m_d3d12Device.Get(), m_d3dCommandList.Get(), m_pScene.get());

	m_d3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_d3dCommandList.Get()};
	m_d3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();


	if (m_pScene)
	{
		m_pScene->ReleaseUploadBuffers();
	}

	m_GameTimer.Reset();
	PreRenderTasks(); // 사전 렌더링 작업
}

void CGameFramework::ReleaseObjects()
{
	if (m_pScene) m_pScene->ReleaseObjects();
	//if (m_pScene) delete m_pScene;
}

void CGameFramework::ProcessInput()
{
	bool bProcessedByScene = false;

	if (GetKeyboardState(m_pKeysBuffer) && m_pScene) bProcessedByScene = m_pScene->ProcessInput(m_pKeysBuffer);
	PostMessage(m_hWnd, WM_SOCKET, (WPARAM)m_pTcpClient->m_sock, MAKELPARAM(FD_WRITE, 0));

	if (!m_pMainPlayer)
	{
		return;
	}

	if (!bProcessedByScene)
	{
		float cxDelta = 0.0f, cyDelta = 0.0f;
		POINT ptCursorPos;
		if (GetCapture() == m_hWnd)
		{
			SetCursor(NULL);
			GetCursorPos(&ptCursorPos);
			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
			SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}

		if ((cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (cxDelta || cyDelta)
			{
				if (m_pKeysBuffer[VK_LBUTTON] & 0xF0)
				{
					if(m_pMainPlayer->m_bAlive || !m_pMainPlayer->m_pSkinnedAnimationController->IsAnimation())
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

	m_pcbMappedTime->time += fElapsedTime;

	if (m_pScene) m_pScene->AnimateObjects(fElapsedTime);
	
	//if (m_pMainPlayer)
	//{
	//	shared_ptr<CGameObject> pPickedObject = m_pMainPlayer->GetPickedObject().lock();
	//}
	////{
	////	//pPickedObject->UpdatePicking();
	////}
	//////m_pPlayer->Animate(fElapsedTime);
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

void CGameFramework::PreRenderTasks()
{
	//ProcessInput();

	int nClientId = m_pTcpClient->GetClientId();
	if (nClientId != -1)
	{
		SetPlayerObjectOfClient(nClientId);
		//m_bPrevRender = true;
	}
	else	// 에러임
	{
		exit(0);
	}

	m_pMainPlayer->Update(m_GameTimer.GetTimeElapsed());

	AnimateObjects();
	// 이곳에서 렌더링 하기전에 준비작업을 시행하도록한다. ex) 쉐도우맵 베이킹
	// buildobject함수 호출 이후 처리되어야할 작업이다. -> 모든 객체들이 렌더링되어야 그림자맵을 생성함.
	
	//HRESULT hResult = m_d3dCommandAllocator->Reset();
	HRESULT hResult = m_d3dCommandAllocator[m_nSwapChainBufferIndex]->Reset();
	hResult = m_d3dCommandList->Reset(m_d3dCommandAllocator[m_nSwapChainBufferIndex].Get(), NULL);

	SynchronizeResourceTransition(m_d3dCommandList.Get(), m_d3dSwapChainBackBuffers[m_nSwapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	
	// 그림자맵에 해당하는 텍스처를 렌더타겟으로 변환
	m_pPostProcessingShader->OnShadowPrepareRenderTarget(m_d3dCommandList.Get());

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * ::gnRtvDescriptorIncrementSize);
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pPostProcessingShader->GetDsvCPUDesctriptorHandle(0);

	auto& vlightCamera = m_pPostProcessingShader->GetLightCamera();

	for (int i = 0; i < m_pPostProcessingShader->GetShadowTexture()->GetTextures();++i) {
		D3D12_CPU_DESCRIPTOR_HANDLE shadowRTVDescriptorHandle = m_pPostProcessingShader->GetShadowRtvCPUDescriptorHandle(i);
		{
			m_d3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

			m_d3dCommandList->OMSetRenderTargets(1, &shadowRTVDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

			//1차 렌더링
			if (m_pScene)
			{
				m_pScene->Render(m_d3dCommandList.Get(), vlightCamera[i], 1); // 카메라만 빛의 위치대로 설정해서 렌더링함.
			}
		}
	}
	// 그림자맵에 해당하는 렌더타겟을 텍스처로 변환
	m_pPostProcessingShader->TransitionShadowMapRenderTargetToCommon(m_d3dCommandList.Get());
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

	ProcessInput();

#ifndef SINGLE_PLAY
	if (!m_pClientNetwork->Logic()) {
		::PostQuitMessage(0);
	}
#endif // SINGLE_PLAY

	AnimateObjects();

	//ProcessCollide();

	HRESULT hResult = m_d3dCommandAllocator[m_nSwapChainBufferIndex]->Reset();
	hResult = m_d3dCommandList->Reset(m_d3dCommandAllocator[m_nSwapChainBufferIndex].Get(), NULL);

	{
		int ndynamicShadowMap = 1;
		// 그림자맵에 해당하는 텍스처를 렌더타겟으로 변환
		m_pPostProcessingShader->OnShadowPrepareRenderTarget(m_d3dCommandList.Get(), ndynamicShadowMap); //플레이어의 손전등 1개 -> [0] 번째 요소에 들어있음.

		D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pPostProcessingShader->GetDsvCPUDesctriptorHandle(0);

		auto& vlightCamera = m_pPostProcessingShader->GetLightCamera();

		auto client_player = dynamic_pointer_cast<CBlueSuitPlayer>(m_pScene->m_pMainPlayer);
		if (client_player) {
			XMFLOAT4X4* xmf4x4playerLight = client_player->GetFlashLigthWorldTransform();
			vlightCamera[0]->SetPosition(XMFLOAT3(xmf4x4playerLight->_41, xmf4x4playerLight->_42, xmf4x4playerLight->_43));
			vlightCamera[0]->SetLookVector(XMFLOAT3(xmf4x4playerLight->_21, xmf4x4playerLight->_22, xmf4x4playerLight->_23));
			vlightCamera[0]->RegenerateViewMatrix();
			vlightCamera[0]->MultiplyViewProjection();
		}

		static XMFLOAT4X4 xmf4x4ToTexture = {
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f };
		static XMMATRIX xmProjectionToTexture = XMLoadFloat4x4(&xmf4x4ToTexture);

		XMFLOAT4X4 viewProjection = vlightCamera[0]->GetViewProjection();
		XMMATRIX xmmtxViewProjection = XMLoadFloat4x4(&viewProjection);
		XMStoreFloat4x4(&m_pScene->m_pLights[0].m_xmf4x4ViewProjection, XMMatrixTranspose(xmmtxViewProjection * xmProjectionToTexture));

		for (int i = 0; i < ndynamicShadowMap/*m_pPostProcessingShader->GetShadowTexture()->GetTextures()*/;++i) {
			D3D12_CPU_DESCRIPTOR_HANDLE shadowRTVDescriptorHandle = m_pPostProcessingShader->GetShadowRtvCPUDescriptorHandle(i);
			{
				m_d3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

				m_d3dCommandList->OMSetRenderTargets(1, &shadowRTVDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

				//1차 렌더링
				if (m_pScene)
				{
					m_pScene->Render(m_d3dCommandList.Get(), vlightCamera[i], 1/*nPipelinestate*/); // 카메라만 빛의 위치대로 설정해서 렌더링함.
				}
			}
		}
		// 그림자맵에 해당하는 렌더타겟을 텍스처로 변환
		m_pPostProcessingShader->TransitionShadowMapRenderTargetToCommon(m_d3dCommandList.Get(), ndynamicShadowMap); //플레이어의 손전등 1개 -> [0] 번째 요소에 들어있음.
	}
	
	SynchronizeResourceTransition(m_d3dCommandList.Get(), m_d3dSwapChainBackBuffers[m_nSwapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * ::gnRtvDescriptorIncrementSize);

	//D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_d3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pPostProcessingShader->GetDsvCPUDesctriptorHandle(0);
	{
		m_d3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

		m_pPostProcessingShader->OnPrepareRenderTarget(m_d3dCommandList.Get(), 0, &m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBufferIndex], &d3dDsvCPUDescriptorHandle);

		//1차 렌더링
		if (m_pScene)
		{
			m_d3dCommandList->SetGraphicsRootDescriptorTable(12, m_d3dTimeCbvGPUDescriptorHandle);
			m_pScene->Render(m_d3dCommandList.Get(), m_pCamera.lock(), 0);
		}

#ifdef _WITH_PLAYER_TOP
		m_d3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif
		//m_pPlayer->Render(m_d3dCommandList.Get()); //[CJI 0405] 씬에 플레이어가 존재하므로 렌더링 x

		
		m_pPostProcessingShader->TransitionRenderTargetToCommon(m_d3dCommandList.Get());

		FLOAT ClearValue[4] = { 1.0f,1.0f,1.0f,1.0f };
		
		ClearValue[3] = 1.0f;
		m_d3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, ClearValue, 0, NULL);

		//OM 최종타겟으로 재설정
		m_d3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);
		//불투명 객체 최종 렌더링
		m_pPostProcessingShader->Render(m_d3dCommandList.Get(), m_pCamera.lock());

		// 투명 객체 렌더링
		if (m_pScene)
		{
			for (auto& s : m_pScene->m_vForwardRenderShader) {
				s->Render(m_d3dCommandList.Get(), m_pCamera.lock());
			}
		}
	}
	//SynchronizeResourceTransition(m_d3dCommandList.Get(), m_d3dSwapChainBackBuffers[m_nSwapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	//[CJI 0411] -> RenderUI에서 렌더타겟 사용이 끝나면 m_wrappedBackBuffers가 자원을 해제할때 자동적으로 상태를 D3D12_RESOURCE_STATE_PRESENT으로 되돌리기 때문에 불필요

	hResult = m_d3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { m_d3dCommandList.Get()};
	m_d3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	RenderUI();

	WaitForGpuComplete();
	
#ifdef _WITH_PRESENT_PARAMETERS
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	m_pdxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);
#else
#ifdef _WITH_SYNCH_SWAPCHAIN
	m_pdxgiSwapChain->Present(1, 0);
#else
	m_dxgiSwapChain->Present(0, 0);
#endif
#endif

	MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 15, 37);
	size_t nLength = _tcslen(m_pszFrameRate);
	XMFLOAT3 xmf3Position = xmf3Position = m_pMainPlayer->GetPosition();
	_stprintf_s(m_pszFrameRate + nLength, 200 - nLength, _T("ID:%d, NumOfClinet: %d, (%4f, %4f, %4f)"), m_pTcpClient->GetClientId(), m_pTcpClient->GetNumOfClient(), xmf3Position.x, xmf3Position.y, xmf3Position.z);
	::SetWindowText(m_hWnd, m_pszFrameRate);

	//char buf[256];
	//sprintf_s(buf, sizeof(buf), "Debug: %f %f %f\n", xmf3Position.x, xmf3Position.y, xmf3Position.z);
	//OutputDebugStringA(buf);
}