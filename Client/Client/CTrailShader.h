#pragma once
#include "Shader.h"
#include "Trail.h"

class CTrailShader : public CShader {
public:
	CTrailShader();
	~CTrailShader() {};
	void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat) override;

	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;

	D3D12_SHADER_BYTECODE CreateVertexShader() override;
	D3D12_SHADER_BYTECODE CreatePixelShader() override;
	D3D12_RASTERIZER_DESC CreateRasterizerState() override;
	D3D12_BLEND_DESC CreateBlendState() override;
	D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) override;
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, const shared_ptr<CPlayer>& pPlayer, int nPipelineState = 0) override;

	shared_ptr<Trail> GetZombieSwordTrail1() {
		return m_vpTrail[0];
	}
	shared_ptr<Trail> GetZombieSwordTrail2() {
		return m_vpTrail[1];
	}
private:
	vector<shared_ptr<Trail>> m_vpTrail;
};
