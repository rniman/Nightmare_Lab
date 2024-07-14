#pragma once
#include "stdafx.h"
#include "Scene.h"
#include "ParticleShader.h"
#include "ParticleMesh.h"
#include "SharedObject.h"


ParticleShader::ParticleShader()
{
}

D3D12_INPUT_LAYOUT_DESC ParticleShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	UINT nInputElementDescs = 0;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = nullptr;

	nInputElementDescs = 8;
	pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "LIFETIME", 0, DXGI_FORMAT_R32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "PARTICLETYPE", 0, DXGI_FORMAT_R32_UINT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "STARTTIME", 0, DXGI_FORMAT_R32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	pd3dInputElementDescs[5] = { "IPARTICLEID", 0, DXGI_FORMAT_R32_UINT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[6] = { "ISPOSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 4, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[7] = { "ISTARTTIME", 0, DXGI_FORMAT_R32_FLOAT, 1, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_STREAM_OUTPUT_DESC ParticleShader::CreateStreamOutput()
{
	D3D12_STREAM_OUTPUT_DESC d3dStreamOutputDesc;
	::ZeroMemory(&d3dStreamOutputDesc, sizeof(D3D12_STREAM_OUTPUT_DESC));

	if (m_PipeLineIndex == 0)
	{
		UINT nStreamOutputDecls = 5;
		D3D12_SO_DECLARATION_ENTRY* pd3dStreamOutputDecls = new D3D12_SO_DECLARATION_ENTRY[nStreamOutputDecls];
		pd3dStreamOutputDecls[0] = { 0, "POSITION", 0, 0, 3, 0 };
		pd3dStreamOutputDecls[1] = { 0, "VELOCITY", 0, 0, 3, 0 };
		pd3dStreamOutputDecls[2] = { 0, "LIFETIME", 0, 0, 1, 0 };
		pd3dStreamOutputDecls[3] = { 0, "PARTICLETYPE", 0, 0, 1, 0 };
		pd3dStreamOutputDecls[4] = { 0, "STARTTIME", 0, 0, 1, 0 };
		/*pd3dStreamOutputDecls[4] = { 0, "PARTICLEID", 0, 0, 1, 0 };
		pd3dStreamOutputDecls[5] = { 0, "SPOSITION", 0, 0, 3, 0 };*/

		UINT* pBufferStrides = new UINT[1];
		pBufferStrides[0] = sizeof(CSOParticleMesh::ParticleVertex);

		d3dStreamOutputDesc.NumEntries = nStreamOutputDecls;
		d3dStreamOutputDesc.pSODeclaration = pd3dStreamOutputDecls;
		d3dStreamOutputDesc.NumStrides = 1;
		d3dStreamOutputDesc.pBufferStrides = pBufferStrides;
		d3dStreamOutputDesc.RasterizedStream = D3D12_SO_NO_RASTERIZED_STREAM;
	}

	return(d3dStreamOutputDesc);
}

D3D12_RASTERIZER_DESC ParticleShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return d3dRasterizerDesc;
}

D3D12_BLEND_DESC ParticleShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

D3D12_DEPTH_STENCIL_DESC ParticleShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = false;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

D3D12_SHADER_BYTECODE ParticleShader::CreateVertexShader()
{
	if (m_PipeLineIndex == 0) {
		return(CShader::CompileShaderFromFile(L"ParticleShader.hlsl", "VSParticleStreamOutput", "vs_5_1", m_pd3dVertexShaderBlob.GetAddressOf()));
	}
	else {
		return(CShader::CompileShaderFromFile(L"ParticleShader.hlsl", "VSParticleDraw", "vs_5_1", m_pd3dVertexShaderBlob.GetAddressOf()));
	}
}

D3D12_SHADER_BYTECODE ParticleShader::CreatePixelShader()
{
	if (m_PipeLineIndex == 0) {
		return(CShader::CreatePixelShader());
	}
	else {
		return(CShader::CompileShaderFromFile(L"ParticleShader.hlsl", "PSParticleDraw", "ps_5_1", m_pd3dPixelShaderBlob.GetAddressOf()));
	}
}

D3D12_SHADER_BYTECODE ParticleShader::CreateGeometryShader()
{
	if (m_PipeLineIndex == 0) {
		return(CShader::CompileShaderFromFile(L"ParticleShader.hlsl", "GSParticleStreamOutput", "gs_5_1", m_pd3dGeometryShaderBlob.GetAddressOf()));
	}
	else {
		return(CShader::CompileShaderFromFile(L"ParticleShader.hlsl", "GSParticleDraw", "gs_5_1", m_pd3dGeometryShaderBlob.GetAddressOf()));
	}
}

void ParticleShader::CreateParticleShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat)
{
	::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	m_d3dPipelineStateDesc.VS = CreateVertexShader();
	m_d3dPipelineStateDesc.PS = CreatePixelShader();
	m_d3dPipelineStateDesc.GS = CreateGeometryShader();
	m_d3dPipelineStateDesc.StreamOutput = CreateStreamOutput();
	m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	m_d3dPipelineStateDesc.BlendState = CreateBlendState();
	m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
	m_d3dPipelineStateDesc.PrimitiveTopologyType = m_topology_type;
	m_d3dPipelineStateDesc.NumRenderTargets = nRenderTargets;
	if (pdxgiRtvFormats) {
		for (int i = 0;i < nRenderTargets;++i) {
			m_d3dPipelineStateDesc.RTVFormats[i] = pdxgiRtvFormats[i];
		}
	}
	else {
		if (nRenderTargets == 0) {
			m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
		}
		else {
			m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		}
	}
	m_d3dPipelineStateDesc.DSVFormat = dxgiDsvFormat;
	m_d3dPipelineStateDesc.SampleDesc.Count = 1;
	m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)m_vpd3dPipelineState[m_PipeLineIndex++].GetAddressOf());

	if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void ParticleShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat)
{
	m_nPipelineState = 2;
	m_vpd3dPipelineState.reserve(m_nPipelineState);
	for (int i = 0; i < m_nPipelineState; ++i)
	{
		m_vpd3dPipelineState.emplace_back();
	}
	m_topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

	CreateParticleShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, 0,nullptr, DXGI_FORMAT_D24_UNORM_S8_UINT);
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	CreateParticleShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, 1, &format, DXGI_FORMAT_D24_UNORM_S8_UINT);

}

void ParticleShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	srand((unsigned)time(NULL));
	XMFLOAT4* pxmf4RandomValues = new XMFLOAT4[1024];
	for (int i = 0; i < 1024; i++) { pxmf4RandomValues[i].x = float((rand() % 10000) - 5000) / 5000.0f; pxmf4RandomValues[i].y = float((rand() % 10000) - 5000) / 5000.0f; pxmf4RandomValues[i].z = float((rand() % 10000) - 5000) / 5000.0f; pxmf4RandomValues[i].w = float((rand() % 10000) - 5000) / 5000.0f; }

	m_pRandowmValueTexture = make_shared<CTexture>(1, RESOURCE_BUFFER, 0, 1);
	m_pRandowmValueTexture->LoadBuffer(pd3dDevice, pd3dCommandList, pxmf4RandomValues, 1024, sizeof(XMFLOAT4), DXGI_FORMAT_R32G32B32A32_FLOAT, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_pRandowmValueTexture, 0, 15);

	// 파티클 마티리얼
	shared_ptr<CTexture> pRoundSoftParticleTexture = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	pRoundSoftParticleTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/Textures/trailTest.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pRoundSoftParticleTexture, 0, 3);
	shared_ptr<CMaterial> pParticleMaterial = make_shared<CMaterial>(1);
	pParticleMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	pParticleMaterial->SetTexture(pRoundSoftParticleTexture);

	//텔레포트 효과 파티클
	shared_ptr<CTPParticleMesh> pGSParticleMesh = make_shared<CTPParticleMesh>(pd3dDevice, pd3dCommandList);
	pGSParticleMesh->CreateShaderVariable(pd3dDevice, pd3dCommandList);
	shared_ptr<CGSParticleObject> pGSParticleObject = make_shared<CGSParticleObject>(pd3dDevice, pd3dCommandList);
	pGSParticleObject->SetMesh(pGSParticleMesh);
	pGSParticleObject->SetMaterial(0, pParticleMaterial);
	m_vParticleObjects.push_back(pGSParticleObject);

	//아이템 획득 효과 파티클
	shared_ptr<CDefaultParticleMesh> pDefaultParticleMesh = make_shared<CDefaultParticleMesh>(pd3dDevice, pd3dCommandList);
	pDefaultParticleMesh->CreateShaderVariable(pd3dDevice, pd3dCommandList, CParticleMesh::SPARK, 64);
	shared_ptr<CGSParticleObject> pItemGetParticleObject = make_shared<CGSParticleObject>(pd3dDevice, pd3dCommandList);
	pItemGetParticleObject->SetMesh(pDefaultParticleMesh);
	pItemGetParticleObject->SetMaterial(0, pParticleMaterial);
	m_vParticleObjects.push_back(pItemGetParticleObject);

	//배양기 기포 효과 파티클
	shared_ptr<CSOParticleMesh> pSOParticleMesh = make_shared<CSOParticleMesh>(pd3dDevice, pd3dCommandList);
	pSOParticleMesh->CreateShaderVariable(pd3dDevice, pd3dCommandList,MAX_PARTICLE, XMFLOAT3(0.f, 0.f, 0.f));
	shared_ptr<CSOParticleObject> pSOParticleObject = make_shared<CSOParticleObject>(pd3dDevice, pd3dCommandList);
	pSOParticleObject->SetMesh(pSOParticleMesh);
	pSOParticleObject->SetMaterial(0, pParticleMaterial);
	m_vParticleObjects.push_back(pSOParticleObject);

	//공격 효과 파티클
	shared_ptr<CAttackParticleMesh> pAttackParticleMesh = make_shared<CAttackParticleMesh>(pd3dDevice, pd3dCommandList);
	pAttackParticleMesh->CreateShaderVariable(pd3dDevice, pd3dCommandList);
	shared_ptr<CGSParticleObject> pAttackParticleObject = make_shared<CGSParticleObject>(pd3dDevice, pd3dCommandList);
	pAttackParticleObject->SetMesh(pAttackParticleMesh);
	pAttackParticleObject->SetMaterial(0, pParticleMaterial);
	m_vParticleObjects.push_back(pAttackParticleObject);

	// 모든 파티클 객체가 PSO 객체를 참조 할 수 있도록함.
	for (auto& ob : m_vParticleObjects) {
		ob->SetRefPSO(m_vpd3dPipelineState);
	}

	sharedobject.m_vParticleObjects = m_vParticleObjects;
}

void ParticleShader::SetParticleTest(float fCurTime)
{
	XMFLOAT3 pos = XMFLOAT3(rand() % 10, 2.0f, 0.0f);
	m_vParticleObjects[0]->SetParticleInsEnable(-1, true, fCurTime, pos);
}

void ParticleShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, const shared_ptr<CPlayer>& pPlayer, int nPipelineState)
{
	UpdatePipeLineState(pd3dCommandList, nPipelineState);
	m_pRandowmValueTexture->UpdateShaderVariables(pd3dCommandList);

	for (auto& object : m_vParticleObjects)
	{
		object->Render(pd3dCommandList);
	}
}

void ParticleShader::ParticleUpdate(float fCurTime)
{
	for (auto& object : m_vParticleObjects)
	{
		object->Update(fCurTime);
	}
}