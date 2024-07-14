#pragma once
#include "Shader.h"
#include "ParticleObject.h"

class ParticleShader : public CShader {
public:
	ParticleShader();
	~ParticleShader() = default;

	D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	D3D12_RASTERIZER_DESC CreateRasterizerState();
	D3D12_BLEND_DESC CreateBlendState();
	D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	D3D12_SHADER_BYTECODE CreateVertexShader();
	D3D12_SHADER_BYTECODE CreatePixelShader();
	D3D12_SHADER_BYTECODE CreateGeometryShader();
	D3D12_STREAM_OUTPUT_DESC CreateStreamOutput();

	void CreateParticleShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat);
	void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat);

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, const shared_ptr<CPlayer>& pPlayer, int nPipelineState = 0);
	void ParticleUpdate(float fCurTime);

	void SetParticleTest(float fCurTime);
	vector<shared_ptr<CParticleObject>>& GetParticleObjects() { return m_vParticleObjects; }
public:
	vector<shared_ptr<CParticleObject>> m_vParticleObjects;
private:
	shared_ptr<CTexture> m_pRandowmValueTexture;
};