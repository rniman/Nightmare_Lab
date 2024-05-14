#pragma once

#include "Object.h"
#include "Camera.h"

class CShader
{
public:
	CShader();
	~CShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	D3D12_SHADER_BYTECODE CompileShaderFromFile(const WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob);
	D3D12_SHADER_BYTECODE ReadCompiledShaderFromFile(const WCHAR* pszFileName, ID3DBlob** ppd3dShaderBlob = NULL);

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets = 1,
		DXGI_FORMAT* pdxgiRtvFormats = nullptr, DXGI_FORMAT dxgiDsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) { }
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) { }
	virtual void ReleaseShaderVariables() { }

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World) { }

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, int nPipelineState = 0);
	virtual void PrevRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, int nPipelineState = 0);

	virtual void UpdatePipeLineState(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState);

	virtual void ReleaseUploadBuffers();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) { }
	virtual void AnimateObjects(float fElapsedTime);
	virtual void ReleaseObjects() { }

	virtual void AddGameObject(const shared_ptr<CGameObject>& pGameObject);

	// Interface
	vector<shared_ptr<CGameObject>> GetGameObjects() const { return m_vGameObjects; };

protected:
	// 게임내 오브젝트는 쉐이더가 관리한다.
	vector<shared_ptr<CGameObject>> m_vGameObjects;
	int object_count{};

	// m_ppd3dPipelineState 를 만들때 Blob을 사용하므로 ComPtr 사용x (오류발생가능)
	ComPtr<ID3DBlob> m_pd3dVertexShaderBlob;
	ComPtr<ID3DBlob> m_pd3dPixelShaderBlob;

	UINT					m_nPipelineState = 1;
	vector<ComPtr<ID3D12PipelineState>> m_vpd3dPipelineState;
	UINT					m_PipeLineIndex = 0;
	//ID3D12PipelineState**	m_ppd3dPipelineState = NULL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC	m_d3dPipelineStateDesc;

	float								m_fElapsedTime = 0.0f;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

class StandardShader : public CShader {
public:
	StandardShader();
	virtual ~StandardShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,  UINT nRenderTargets = 1,
		DXGI_FORMAT* pdxgiRtvFormats = nullptr, DXGI_FORMAT dxgiDsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT);
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) override { }

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

class InstanceStandardShader : public StandardShader {
public:
	InstanceStandardShader();
	virtual ~InstanceStandardShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, int nPipelineState = 0);
	virtual void AnimateObjects(float fElapsedTime);
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TransparentShader : public InstanceStandardShader {
public:
	TransparentShader();
	virtual ~TransparentShader();

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, int nPipelineState = 0);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSkinnedAnimationStandardShader : public StandardShader
{
public:
	CSkinnedAnimationStandardShader();
	virtual ~CSkinnedAnimationStandardShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CScene;

class CPostProcessingShader : public CShader
{
public:
	CPostProcessingShader();
	virtual ~CPostProcessingShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets = 1,
		DXGI_FORMAT* pdxgiRtvFormats = nullptr, DXGI_FORMAT dxgiDsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT);
	virtual void CreateResourcesAndRtvsSrvs(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nRenderTargets, DXGI_FORMAT* pdxgiFormats, D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle);

	virtual void OnPrepareRenderTarget(ID3D12GraphicsCommandList* pd3dCommandList, int nRenderTargets, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle);
	virtual void OnPrepareRenderTarget2(ID3D12GraphicsCommandList* pd3dCommandList, int nRenderTargets, 
		D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dshadowRTVDescriptorHandle);
	virtual void TransitionRenderTargetToCommon(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void TransitionCommonToRenderTarget(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera);

protected:
	shared_ptr<CTexture> m_pTexture;

	vector<unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE>> m_vpRtvCPUDescriptorHandles;

	FLOAT m_fClearValue[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	ComPtr<ID3D12DescriptorHeap> m_pd3dDsvDescriptorHeap;
	ComPtr<ID3D12Resource> m_pd3dDepthBuffer;
	vector<unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE>> m_vpDsvDescriptorCPUHandles;

public:
	shared_ptr<CTexture> GetTexture() { return(m_pTexture); }
	ID3D12Resource* GetTextureResource(UINT nIndex) { return(m_pTexture->GetResource(nIndex)); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetRtvCPUDescriptorHandle(UINT nIndex) { return(*m_vpRtvCPUDescriptorHandles[nIndex]); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetDsvCPUDesctriptorHandle(UINT nIndex) { return (*m_vpDsvDescriptorCPUHandles[nIndex]); }
	
//ShadowMap Processing
protected:
	shared_ptr<CTexture> m_pShadowTextures;
	vector<shared_ptr<CCamera>> m_pLightCamera;

	vector<unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE>> m_vpShadowRtvCPUDescriptorHandles;
	vector<unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE>> m_vpShadowDsvDescriptorCPUHandles;

	ComPtr<ID3D12DescriptorHeap> m_pd3dShadowRtvDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> m_pd3dShadowDsvDescriptorHeap;
	ComPtr<ID3D12Resource> m_pd3dShadowDepthBuffer;

	const UINT m_iShadowPipeLineIndex = 1;
public:
	shared_ptr<CTexture> GetShadowTexture() { return(m_pShadowTextures); }
	ID3D12Resource* GetShadowTextureResource(UINT nIndex) { return(m_pShadowTextures->GetResource(nIndex)); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetShadowRtvCPUDescriptorHandle(UINT nIndex) { return(*m_vpShadowRtvCPUDescriptorHandles[nIndex]); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetShadowDsvCPUDesctriptorHandle(UINT nIndex) { return (*m_vpShadowDsvDescriptorCPUHandles[nIndex]); }

	void CreateShadowMapResource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nlight, D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle);
	void OnShadowPrepareRenderTarget(ID3D12GraphicsCommandList* pd3dCommandList, int nclearcount = 0);
	void ShadowTextureWriteRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera);

	void TransitionShadowMapRenderTargetToCommon(ID3D12GraphicsCommandList* pd3dCommandList, int nTransition=0);

	void CreateLightCamera(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,CScene* scene);
	vector<shared_ptr<CCamera>>& GetLightCamera() { return  m_pLightCamera; }
};

/// <CShader - CPostProcessingShader>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CShader - CUserInterfaceShader>

// [0504] UI SHADER
class CUserInterfaceShader : public CShader
{
public:

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	
	void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat);

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) override;

private:
};

/// <CShader - UserInterfaceShader>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CShader - StandardShader - OutLineShader>

class CBlueSuitPlayer;
class CZombiePlayer;

//[0505] OutLine
class COutLineShader : public CShader
{
public:
	COutLineShader() {};
	virtual ~COutLineShader() {};

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat);

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, int nPipelineState = 0);

	virtual void AddGameObject(const shared_ptr<CGameObject>& pGameObject);

	void SetPostProcessingShader(CPostProcessingShader* pPostProcessingShader) { m_pPostProcessingShader = pPostProcessingShader; }
private:
	shared_ptr<CZombiePlayer> m_pZombiePlayer;

	bool m_bOutLine = false;
	CPostProcessingShader* m_pPostProcessingShader = nullptr;
};
