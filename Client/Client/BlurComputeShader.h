#pragma once
#include "Shader.h"

class CTextureToScreenShader : public CShader
{
public:
	CTextureToScreenShader() {};
	CTextureToScreenShader(shared_ptr<CTexture>& pTexture);
	virtual ~CTextureToScreenShader() {};

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat);

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, const shared_ptr<CPlayer>& pPlayer);
private:
	shared_ptr<CTexture> m_pTexture;
};

class CComputeShader : public CShader
{
public:
	CComputeShader() {};
	virtual ~CComputeShader() {};

	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob);

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups);

	virtual void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups);

protected:
	ComPtr<ID3DBlob> m_pd3dComputeShaderBlob;

	UINT m_cxThreadGroups = 0;
	UINT m_cyThreadGroups = 0;
	UINT m_czThreadGroups = 0;
};


class CBlurComputeShader : public CComputeShader
{
public:
	CBlurComputeShader() {};
	~CBlurComputeShader() {};

	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob) override;

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature, UINT cxThreadGroups = 1, UINT cyThreadGroups = 1, UINT czThreadGroups = 1);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState);

	void SetTextureRtv(shared_ptr<CTexture>& pTexture) { m_pTextureRtv = pTexture; }
	shared_ptr<CTexture>& GetTextureSrv() { return m_pTextureSrv; }

	void SetPipeLineIndex(int nIndex) { m_nCurPipeLineIndex = nIndex; }
	int GetPipeLineIndex() const { return m_nCurPipeLineIndex; }
private:
	shared_ptr<CTexture> m_pTextureUav;	// 컴퓨트 셰이더 Uav 텍스쳐
	shared_ptr<CTexture> m_pTextureSrv;	// 컴퓨트 셰이더 Srv 텍스쳐(ping pong을 위한)
	shared_ptr<CTexture> m_pTextureRtv; // G-Buffer로 썼던것

	int m_nCurPipeLineIndex = 0;
};

