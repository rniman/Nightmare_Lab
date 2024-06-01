#include "stdafx.h"
#include "Shader.h"
#include "Scene.h"
#include "Player.h"

CShader::CShader()
{
}

CShader::~CShader()
{
	ReleaseShaderVariables();

	//if (m_ppd3dPipelineState) 
	//{
	//	for (int i = 0; i < m_nPipelineState;++i) 
	//	{
	//		if (m_ppd3dPipelineState[i]) {
	//			m_ppd3dPipelineState[i]->Release();
	//		}
	//	}
	//}
}

D3D12_SHADER_BYTECODE CShader::CreateVertexShader()
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CreatePixelShader()
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CompileShaderFromFile(const WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob)
{
	UINT nCompileFlags = 0;
#if defined(_DEBUG)
	nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pd3dErrorBlob = NULL;
	HRESULT hResult = ::D3DCompileFromFile(pszFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, pszShaderName, pszShaderProfile, nCompileFlags, 0, ppd3dShaderBlob, &pd3dErrorBlob);
	char* pErrorString = NULL;
	if (pd3dErrorBlob) pErrorString = (char*)pd3dErrorBlob->GetBufferPointer();

	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
	d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();

	return(d3dShaderByteCode);
}

#define _WITH_WFOPEN
//#define _WITH_STD_STREAM

#ifdef _WITH_STD_STREAM
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#endif

D3D12_SHADER_BYTECODE CShader::ReadCompiledShaderFromFile(const WCHAR* pszFileName, ID3DBlob** ppd3dShaderBlob)
{
	std::ifstream ifsFile(pszFileName, std::ios::binary | std::ios::ate);

	if (!ifsFile)
	{
		assert("파일 열기 실패!");
	}

	std::streamoff llFileSize = ifsFile.tellg();
	ifsFile.seekg(0, std::ios::beg);

	std::vector<BYTE> vbyteCode;
	vbyteCode.reserve(llFileSize);
	ifsFile.read((char*)vbyteCode.data(), llFileSize);
	D3D12_SHADER_BYTECODE d3dByteCode;

	if (ppd3dShaderBlob)
	{
		HRESULT hResult = D3DCreateBlob(llFileSize, ppd3dShaderBlob);
		memcpy((*ppd3dShaderBlob)->GetBufferPointer(), vbyteCode.data(), llFileSize);
		d3dByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
		d3dByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();
}
	else
	{
		d3dByteCode.BytecodeLength = llFileSize;
		d3dByteCode.pShaderBytecode = vbyteCode.data();
	}

	return d3dByteCode;
}

D3D12_INPUT_LAYOUT_DESC CShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = NULL;
	d3dInputLayoutDesc.NumElements = 0;

	return(d3dInputLayoutDesc);
}

D3D12_RASTERIZER_DESC CShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	//d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = true;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

D3D12_DEPTH_STENCIL_DESC CShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
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

D3D12_BLEND_DESC CShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

void CShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat)
{
	::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	m_d3dPipelineStateDesc.VS = CreateVertexShader();
	m_d3dPipelineStateDesc.PS = CreatePixelShader();
	m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	m_d3dPipelineStateDesc.BlendState = CreateBlendState();
	m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
	m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	m_d3dPipelineStateDesc.NumRenderTargets = nRenderTargets;
	if (pdxgiRtvFormats) {
		for (int i = 0;i < nRenderTargets;++i) {
			m_d3dPipelineStateDesc.RTVFormats[i] = pdxgiRtvFormats[i];
		}
	}
	else {
		m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	m_d3dPipelineStateDesc.DSVFormat = dxgiDsvFormat;
	m_d3dPipelineStateDesc.SampleDesc.Count = 1;
	m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)m_vpd3dPipelineState[m_PipeLineIndex++].GetAddressOf());

	if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void CShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, const shared_ptr<CPlayer>& pPlayer, int nPipelineState)
{
	UpdatePipeLineState(pd3dCommandList, nPipelineState);

	for (auto& object : m_vGameObjects) 
	{
		if (!object->m_bThisContainTransparent) {
			object->Render(pd3dCommandList);
		}
		else {
			object->RenderOpaque(pd3dCommandList);
		}
	}
}

void CShader::PrevRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, int nPipelineState)
{
	UpdatePipeLineState(pd3dCommandList, nPipelineState);

	for (auto& object : m_vGameObjects)
	{
		if (!object->IsStatic())	// 사전 그림자맵에서 스태틱만 그린다.
		{
			continue;
		}

		if (!object->m_bThisContainTransparent) {
			object->Render(pd3dCommandList);
		}
		else {
			object->RenderOpaque(pd3dCommandList);
		}
	}
}

void CShader::UpdatePipeLineState(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	if (m_vpd3dPipelineState[nPipelineState].Get())
	{
		pd3dCommandList->SetPipelineState(m_vpd3dPipelineState[nPipelineState].Get());
	}
}

void CShader::ReleaseUploadBuffers()
{
	for (auto& object : m_vGameObjects)
	{
		object->ReleaseUploadBuffers();
	}
}

void CShader::AddGameObject(const shared_ptr<CGameObject>& pGameObject)
{
	object_count++;
	m_vGameObjects.push_back(pGameObject);
	//m_vGameObjects.push_back(shared_ptr<CGameObject>(object));
}

void CShader::AnimateObjects(float fElapsedTime)
{
	for (auto& object : m_vGameObjects) 
	{
		object->Animate(fElapsedTime);
		//object->UpdateTransform(NULL);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// StandardShader

StandardShader::StandardShader()
{

}


StandardShader::~StandardShader()
{

}

D3D12_INPUT_LAYOUT_DESC StandardShader::CreateInputLayout()
{
	UINT nInputElementDescs = 5;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_RASTERIZER_DESC StandardShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;

	if (m_PipeLineIndex == 0) {
		d3dRasterizerDesc = CShader::CreateRasterizerState();
	}
	else if (m_PipeLineIndex == 1) {
		::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
		d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;//D3D12_CULL_MODE_FRONT;//;
		d3dRasterizerDesc.FrontCounterClockwise = FALSE;
		d3dRasterizerDesc.DepthBias = 0;
		d3dRasterizerDesc.DepthBiasClamp = 0.0f;
		d3dRasterizerDesc.SlopeScaledDepthBias = 3.f;
		d3dRasterizerDesc.DepthClipEnable = TRUE;
		d3dRasterizerDesc.MultisampleEnable = FALSE;
		d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
		d3dRasterizerDesc.ForcedSampleCount = 0;
		d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	}

	return(d3dRasterizerDesc);
}

D3D12_SHADER_BYTECODE StandardShader::CreateVertexShader()
{
	return CShader::ReadCompiledShaderFromFile(L"cso/VSStandard.cso", m_pd3dVertexShaderBlob.GetAddressOf());
	//return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSStandard", "vs_5_1", m_pd3dVertexShaderBlob.GetAddressOf()));
}

D3D12_SHADER_BYTECODE StandardShader::CreatePixelShader()
{
	if (m_PipeLineIndex == 0) { // 기본 파이프라인
		return CShader::ReadCompiledShaderFromFile(L"cso/PSStandard.cso", m_pd3dPixelShaderBlob.GetAddressOf());
		//return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSStandard", "ps_5_1", m_pd3dPixelShaderBlob.GetAddressOf()));
	}
	else if (m_PipeLineIndex == 1) { // 그림자맵 생성 파이프 라인
		return CShader::ReadCompiledShaderFromFile(L"cso/PSShadow.cso", m_pd3dPixelShaderBlob.GetAddressOf());
		//return(CShader::CompileShaderFromFile(L"Shadow.hlsl", "PS_Shadow", "ps_5_1", m_pd3dPixelShaderBlob.GetAddressOf()));
	}
}


void StandardShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat)
{
	m_nPipelineState = 2;
	m_vpd3dPipelineState.reserve(m_nPipelineState);
	for (int i = 0; i < m_nPipelineState; ++i)
		m_vpd3dPipelineState.emplace_back();
	//m_ppd3dPipelineState = new ID3D12PipelineState * [m_nPipelineState];

	CShader::CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nRenderTargets, pdxgiRtvFormats, dxgiDsvFormat); //m_ppd3dPipelineStates[0] 생성

	DXGI_FORMAT shadowFormat = DXGI_FORMAT_R32_FLOAT;
	CShader::CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, 1, &shadowFormat, dxgiDsvFormat);

}

InstanceStandardShader::InstanceStandardShader()
{
}

InstanceStandardShader::~InstanceStandardShader()
{
}

D3D12_INPUT_LAYOUT_DESC InstanceStandardShader::CreateInputLayout()
{
	UINT nInputElementDescs = 9;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	pd3dInputElementDescs[5] = { "INSMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[6] = { "INSMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[7] = { "INSMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[8] = { "INSMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE InstanceStandardShader::CreateVertexShader()
{
	return CShader::ReadCompiledShaderFromFile(L"cso/VSInstanceStandard.cso", m_pd3dVertexShaderBlob.GetAddressOf());
	//return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSInstanceStandard", "vs_5_1", m_pd3dVertexShaderBlob.GetAddressOf()));
}

void InstanceStandardShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, const shared_ptr<CPlayer>& pPlayer, int nPipelineState)
{
	UpdatePipeLineState(pd3dCommandList, nPipelineState);
	auto pos = pPlayer->GetPosition();
	int curFloor = static_cast<int>(std::floor(pos.y / 4.5))-1;
		
	for (int i = curFloor; i < curFloor + 3;++i) {
		if (i > 3) break;
		if (i < 0) continue;
		for (const auto& object : m_vFloorObjects[i]) {
			if (!object->m_bThisContainTransparent) {
				object->Render(pd3dCommandList);
			}
			else {
				object->RenderOpaque(pd3dCommandList);
			}
		}
	}
}

//void InstanceStandardShader::FloorRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, const shared_ptr<CPlayer>& pPlayer, int nPipelineState)
//{
//	UpdatePipeLineState(pd3dCommandList, nPipelineState);
//	auto& pos = pPlayer->GetPosition();
//	int curFloor = static_cast<int>(std::floor(pos.y / 4.5));
//	
//	for (int i = curFloor; i < curFloor + 2;++i) {
//		if (i > 3) break;
//		for (const auto& object : m_vFloorObjects[i]) {
//			object->Render(pd3dCommandList);
//		}
//	}
//}

void InstanceStandardShader::AnimateObjects(float fElapsedTime)
{
	for (auto& object : m_vGameObjects) 
	{
		object->Animate(fElapsedTime);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TransparentShader::TransparentShader()
{
}

TransparentShader::~TransparentShader()
{
}

D3D12_RASTERIZER_DESC TransparentShader::CreateRasterizerState()
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

	return(d3dRasterizerDesc);
}

D3D12_BLEND_DESC TransparentShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = true;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = false;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	return d3dBlendDesc;
}

D3D12_DEPTH_STENCIL_DESC TransparentShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = false;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;//D3D12_DEPTH_WRITE_MASK_ALL
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS; // D3D12_COMPARISON_FUNC_LESS_EQUAL
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
	return d3dDepthStencilDesc;
}

D3D12_SHADER_BYTECODE TransparentShader::CreatePixelShader()
{
	return CShader::ReadCompiledShaderFromFile(L"cso/PSTransparent.cso", m_pd3dPixelShaderBlob.GetAddressOf());
	//return(CShader::CompileShaderFromFile(L"Shaders.hlsl","PSTransparent", "ps_5_1", m_pd3dPixelShaderBlob.GetAddressOf()));
}

void TransparentShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, const shared_ptr<CPlayer>& pPlayer, int nPipelineState)
{
	UpdatePipeLineState(pd3dCommandList, nPipelineState);

	for (auto& object : m_vGameObjects)
	{
		object->RenderTransparent(pd3dCommandList);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CSkinnedAnimationStandardShader::CSkinnedAnimationStandardShader()
{
}

CSkinnedAnimationStandardShader::~CSkinnedAnimationStandardShader()
{
}

D3D12_INPUT_LAYOUT_DESC CSkinnedAnimationStandardShader::CreateInputLayout()
{
	UINT nInputElementDescs = 7;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CSkinnedAnimationStandardShader::CreateVertexShader()
{
	return CShader::ReadCompiledShaderFromFile(L"cso/VSSkinnedAnimationStandard.cso", m_pd3dVertexShaderBlob.GetAddressOf());
	//return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSkinnedAnimationStandard", "vs_5_1", m_pd3dVertexShaderBlob.GetAddressOf()));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CPostProcessingShader::CPostProcessingShader()
{
}

CPostProcessingShader::~CPostProcessingShader()
{

}

D3D12_INPUT_LAYOUT_DESC CPostProcessingShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = NULL;
	d3dInputLayoutDesc.NumElements = 0;

	return(d3dInputLayoutDesc);
}

D3D12_DEPTH_STENCIL_DESC CPostProcessingShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;//D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
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

D3D12_SHADER_BYTECODE CPostProcessingShader::CreateVertexShader()
{
	return CShader::ReadCompiledShaderFromFile(L"cso/VSPostProcessing.cso", m_pd3dVertexShaderBlob.GetAddressOf());
	//return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSPostProcessing", "vs_5_1", m_pd3dVertexShaderBlob.GetAddressOf()));
}

D3D12_SHADER_BYTECODE CPostProcessingShader::CreatePixelShader()
{
	if (m_PipeLineIndex == 0) {
		return CShader::ReadCompiledShaderFromFile(L"cso/PSPostProcessing.cso", m_pd3dPixelShaderBlob.GetAddressOf());
		//return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSPostProcessing", "ps_5_1", m_pd3dPixelShaderBlob.GetAddressOf()));
	}
	else if (m_PipeLineIndex == 1) {
		return CShader::ReadCompiledShaderFromFile(L"cso/PSShadow.cso", m_pd3dPixelShaderBlob.GetAddressOf());
		//return(CShader::CompileShaderFromFile(L"Shadow.hlsl", "PS_Shadow", "ps_5_1", m_pd3dPixelShaderBlob.GetAddressOf()));
	}
}

D3D12_RASTERIZER_DESC CPostProcessingShader::CreateRasterizerState()
{
	if (m_PipeLineIndex == 0) {
		return(CShader::CreateRasterizerState());
	}
	else if (m_PipeLineIndex == 1) {
		D3D12_RASTERIZER_DESC resState = CShader::CreateRasterizerState();
		resState.CullMode = D3D12_CULL_MODE_FRONT;
		return resState;
	}
}

void CPostProcessingShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat)
{
	/*m_pd3dGraphicsRootSignature = pd3dGraphicsRootSignature;
	m_pd3dGraphicsRootSignature->AddRef();*/
	m_nPipelineState = 1;
	m_vpd3dPipelineState.reserve(m_nPipelineState);
	for (int i = 0; i < m_nPipelineState; ++i)
	{
		m_vpd3dPipelineState.emplace_back();
	}

	//기본 파이프라인
	CShader::CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nRenderTargets, pdxgiRtvFormats, dxgiDsvFormat);
	////그림자 파이프라인
	//DXGI_FORMAT shadowformat = DXGI_FORMAT_R32_FLOAT;
	//CShader::CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nRenderTargets, &shadowformat, dxgiDsvFormat);
}

void CPostProcessingShader::CreateResourcesAndRtvsSrvs(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nRenderTargets, DXGI_FORMAT* pdxgiFormats, D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle)
{
	m_pTexture = make_shared<CTexture>(nRenderTargets, RESOURCE_TEXTURE2D, 0, 1);

	D3D12_CLEAR_VALUE d3dClearValue;

	for (UINT i = 0; i < nRenderTargets; i++)
	{
		if (pdxgiFormats[i] == DXGI_FORMAT_R32_FLOAT) {
			d3dClearValue = { DXGI_FORMAT_R32_FLOAT, { 1.0f, 1.0f, 1.0f, 1.0f } };
		}
		else {
			d3dClearValue = { pdxgiFormats[i], {m_fClearValue[0],m_fClearValue[1],m_fClearValue[2],m_fClearValue[3]} };
		}
		
		d3dClearValue.Format = pdxgiFormats[i];
		m_pTexture->CreateTexture(pd3dDevice, i, RESOURCE_TEXTURE2D, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 1, 0, pdxgiFormats[i], D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &d3dClearValue);
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CScene::CreateShaderResourceViews(pd3dDevice, m_pTexture, 0, 10);

	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;

	for (UINT i = 0; i < nRenderTargets; i++)
	{
		d3dRenderTargetViewDesc.Format = pdxgiFormats[i];
		ID3D12Resource* pd3dTextureResource = m_pTexture->GetResource(i);
		pd3dDevice->CreateRenderTargetView(pd3dTextureResource, &d3dRenderTargetViewDesc, d3dRtvCPUDescriptorHandle);
		m_vpRtvCPUDescriptorHandles.push_back(make_unique<D3D12_CPU_DESCRIPTOR_HANDLE>(d3dRtvCPUDescriptorHandle));
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}

	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	ThrowIfFailed(pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap));

	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = FRAME_BUFFER_WIDTH;
	d3dResourceDesc.Height = FRAME_BUFFER_HEIGHT;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = 1;
	d3dResourceDesc.SampleDesc.Quality = 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void**)m_pd3dDepthBuffer.GetAddressOf());

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	//m_vpDsvDescriptorCPUHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[ADD_DEPTH_MAP_COUNT];
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pd3dDsvDescriptorHeap.Get()->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < ADD_DEPTH_MAP_COUNT;++i) {
		m_vpDsvDescriptorCPUHandles.push_back(make_unique<D3D12_CPU_DESCRIPTOR_HANDLE>(handle));
		pd3dDevice->CreateDepthStencilView(m_pd3dDepthBuffer.Get(), &d3dDepthStencilViewDesc, *m_vpDsvDescriptorCPUHandles[i]);
		handle.ptr = ::gnDsvDescriptorIncrementSize;
	}

}

void CPostProcessingShader::OnPrepareRenderTarget(ID3D12GraphicsCommandList* pd3dCommandList, int nRenderTargets, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle)
{
	int nResources = m_pTexture->GetTextures();
	D3D12_CPU_DESCRIPTOR_HANDLE* pd3dAllRtvCPUHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[nRenderTargets + nResources];

	for (int i = 0; i < nRenderTargets; i++)
	{
		pd3dAllRtvCPUHandles[i] = pd3dRtvCPUHandles[i];
		pd3dCommandList->ClearRenderTargetView(pd3dRtvCPUHandles[i], m_fClearValue, 0, NULL);
	}

	for (int i = 0; i < nResources; i++)
	{
		::SynchronizeResourceTransition(pd3dCommandList, GetTextureResource(i), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

		D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = GetRtvCPUDescriptorHandle(i);
		if (i == 2) { // 깊이값을 저장하는 렌더타겟
			FLOAT value[4] = { 1.0f,1.0f,1.0f,1.0f };
			pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, value, 0, NULL);
		}
		else {
			pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, m_fClearValue, 0, NULL);
		}
		pd3dAllRtvCPUHandles[nRenderTargets + i] = d3dRtvCPUDescriptorHandle;
	}
	pd3dCommandList->OMSetRenderTargets(nRenderTargets + nResources, pd3dAllRtvCPUHandles, FALSE, pd3dDsvCPUHandle);

	if (pd3dAllRtvCPUHandles) delete[] pd3dAllRtvCPUHandles;
}

void CPostProcessingShader::OnPrepareRenderTarget2(ID3D12GraphicsCommandList* pd3dCommandList, int nRenderTargets, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles,
	D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dshadowRTVDescriptorHandle)
{
	int nResources = m_pTexture->GetTextures();
	D3D12_CPU_DESCRIPTOR_HANDLE* pd3dAllRtvCPUHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[nRenderTargets + nResources];

	for (int i = 0; i < nRenderTargets; i++)
	{
		pd3dAllRtvCPUHandles[i] = pd3dRtvCPUHandles[i];
		pd3dCommandList->ClearRenderTargetView(pd3dRtvCPUHandles[i], m_fClearValue, 0, NULL);
	}

	for (int i = 0; i < nResources; i++)
	{
		::SynchronizeResourceTransition(pd3dCommandList, GetTextureResource(i), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

		D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = GetRtvCPUDescriptorHandle(i);
		if (i == 2) { // 깊이값을 저장하는 렌더타겟
			FLOAT value[4] = { 1.0f,1.0f,1.0f,1.0f };
			pd3dCommandList->ClearRenderTargetView(*pd3dshadowRTVDescriptorHandle, value, 0, NULL);
			d3dRtvCPUDescriptorHandle = *pd3dshadowRTVDescriptorHandle;
		}
		else {
			pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, m_fClearValue, 0, NULL);
		}
		pd3dAllRtvCPUHandles[nRenderTargets + i] = d3dRtvCPUDescriptorHandle;
	}
	pd3dCommandList->OMSetRenderTargets(nRenderTargets + nResources, pd3dAllRtvCPUHandles, FALSE, pd3dDsvCPUHandle);

	if (pd3dAllRtvCPUHandles) delete[] pd3dAllRtvCPUHandles;
}


void CPostProcessingShader::TransitionRenderTargetToCommon(ID3D12GraphicsCommandList* pd3dCommandList)
{
	int nResources = m_pTexture->GetTextures();
	for (int i = 0; i < nResources; i++)
	{
		::SynchronizeResourceTransition(pd3dCommandList, GetTextureResource(i), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
	}
}

void CPostProcessingShader::TransitionShadowMapRenderTargetToCommon(ID3D12GraphicsCommandList* pd3dCommandList,int nTransition)
{
	int nResources = m_pShadowTextures->GetTextures();
	if (nTransition != 0) {
		nResources = nTransition;
	}

	for (int i = 0; i < nResources; i++)
	{
		::SynchronizeResourceTransition(pd3dCommandList, GetShadowTextureResource(i), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
	}
}

void CPostProcessingShader::TransitionCommonToRenderTarget(ID3D12GraphicsCommandList* pd3dCommandList)
{
	int nResources = m_pTexture->GetTextures();
	for (int i = 0; i < nResources; i++)
	{
		::SynchronizeResourceTransition(pd3dCommandList, GetTextureResource(i), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
}

void CPostProcessingShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera,const shared_ptr<CPlayer>& pPlayer )
{
	CShader::Render(pd3dCommandList, pCamera, pPlayer);

	if (m_pTexture) m_pTexture->UpdateShaderVariables(pd3dCommandList);
	if (m_pShadowTextures) m_pShadowTextures->UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

void CPostProcessingShader::ShadowTextureWriteRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, const shared_ptr<CPlayer>& pPlayer)
{
	CShader::Render(pd3dCommandList, pCamera, pPlayer, m_iShadowPipeLineIndex);

	if (m_pTexture) m_pTexture->UpdateShaderVariables(pd3dCommandList);
	if (m_pShadowTextures) m_pShadowTextures->UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

void CPostProcessingShader::CreateShadowMapResource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,UINT nlight, D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = nlight; // 빛의 개수만큼 렌더타겟을 생성
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)m_pd3dShadowRtvDescriptorHeap.GetAddressOf());

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pd3dShadowRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	m_pShadowTextures = make_shared<CTexture>(nlight, RESOURCE_TEXTURE2D, 0, 1);

	DXGI_FORMAT pdxgiFormat = DXGI_FORMAT_R32_FLOAT;
	D3D12_CLEAR_VALUE d3dClearValue(pdxgiFormat, { 1.0f, 1.0f, 1.0f, 1.0f });

	for (UINT i = 0; i < nlight; i++)
	{
		m_pShadowTextures->CreateTexture(pd3dDevice, i, RESOURCE_TEXTURE2D, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 1, 0, pdxgiFormat, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &d3dClearValue);

	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CScene::CreateShaderResourceViews(pd3dDevice, m_pShadowTextures, 0, 11);

	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;
	d3dRenderTargetViewDesc.Format = pdxgiFormat;

	for (UINT i = 0; i < nlight; i++)
	{
		ID3D12Resource* pd3dTextureResource = m_pShadowTextures->GetResource(i);
		pd3dDevice->CreateRenderTargetView(pd3dTextureResource, &d3dRenderTargetViewDesc, rtvHandle);
		m_vpShadowRtvCPUDescriptorHandles.push_back(make_unique<D3D12_CPU_DESCRIPTOR_HANDLE>(rtvHandle));
		rtvHandle.ptr += ::gnRtvDescriptorIncrementSize;
		/*pd3dDevice->CreateRenderTargetView(pd3dTextureResource, &d3dRenderTargetViewDesc, d3dRtvCPUDescriptorHandle);
		m_vpShadowRtvCPUDescriptorHandles.push_back(make_unique<D3D12_CPU_DESCRIPTOR_HANDLE>(d3dRtvCPUDescriptorHandle));
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;*/
	}
}

void CPostProcessingShader::CreateLightCamera(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,CScene* scene)
{
	vector<XMFLOAT3> positions = scene->GetLightPositions();
	vector<XMFLOAT3> looks = scene->GetLightLooks();

	XMFLOAT3 xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);

	XMFLOAT4X4 xmf4x4ToTexture = {
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f };

	XMMATRIX xmProjectionToTexture = XMLoadFloat4x4(&xmf4x4ToTexture);
	XMMATRIX xmmtxViewProjection;

	for (int i = 0;i < scene->m_nLights;++i) {
		m_pLightCamera.push_back(make_shared<CCamera>());

		XMFLOAT3 xmf3Up = Vector3::CrossProduct(looks[i], xmf3Right);
		XMFLOAT3 lookAtPosition = Vector3::Add(positions[i], looks[i]);
		m_pLightCamera[i]->GenerateViewMatrix(positions[i], lookAtPosition, xmf3Up);
		if(i >= MAX_SURVIVOR)
		{
			m_pLightCamera[i]->GenerateProjectionMatrix(1.01f, 5.0f, ASPECT_RATIO, 90.0f);	//[0513] 근평면이 있어야  그림자를 그림
		}
		m_pLightCamera[i]->GenerateFrustum();
		m_pLightCamera[i]->MultiplyViewProjection();

		XMFLOAT4X4 viewProjection = m_pLightCamera[i]->GetViewProjection();
		xmmtxViewProjection = XMLoadFloat4x4(&viewProjection);
		XMStoreFloat4x4(&scene->m_pLights[i].m_xmf4x4ViewProjection, XMMatrixTranspose(xmmtxViewProjection * xmProjectionToTexture));

		m_pLightCamera[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	}

	// 빛의 카메라 파티션 설정
	unique_ptr<PartitionInsStandardShader> PtShader(static_cast<PartitionInsStandardShader*>(scene->m_vPreRenderShader[PARTITION_SHADER].release()));
	auto vBB = PtShader->GetPartitionBB();

	for (int i = 0; i < scene->m_nLights;++i) {
		BoundingBox camerabb;
		camerabb.Center = m_pLightCamera[i]->GetPosition();
		camerabb.Extents = XMFLOAT3(0.1f, 0.1f, 0.1f);

		for (int bbIdx = 0; bbIdx < vBB.size();++bbIdx) {
			if (vBB[bbIdx]->Intersects(camerabb)) {
				m_pLightCamera[i]->SetPartition(bbIdx);
				break;
			}
		}
	}

	scene->m_vPreRenderShader[PARTITION_SHADER].reset(PtShader.release());
}


void CPostProcessingShader::OnShadowPrepareRenderTarget(ID3D12GraphicsCommandList* pd3dCommandList,int nclearcount)
{
	int nResources = m_pShadowTextures->GetTextures();
	if (nclearcount != 0) {
		nResources = nclearcount;
	}
	FLOAT clearValue[4] = { 1.0f,1.0f,1.0f,1.0f };
	for (int i = 0; i < nResources; i++)
	{
		::SynchronizeResourceTransition(pd3dCommandList, GetShadowTextureResource(i), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

		D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = GetShadowRtvCPUDescriptorHandle(i);
		pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, clearValue, 0, NULL);
	}
}

/// <CShader - CPostProcessingShader>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CShader - CBlueSuitUserInterfaceShader>

D3D12_INPUT_LAYOUT_DESC CBlueSuitUserInterfaceShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CBlueSuitUserInterfaceShader::CreateVertexShader()
{
	return CShader::ReadCompiledShaderFromFile(L"cso/VSUserInterface.cso", m_pd3dVertexShaderBlob.GetAddressOf());
	//return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSUserInterface", "vs_5_1", m_pd3dVertexShaderBlob.GetAddressOf()));
}

D3D12_SHADER_BYTECODE CBlueSuitUserInterfaceShader::CreatePixelShader()
{
	return CShader::ReadCompiledShaderFromFile(L"cso/PSUserInterface.cso", m_pd3dPixelShaderBlob.GetAddressOf());
	//return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSUserInterface", "ps_5_1", m_pd3dPixelShaderBlob.GetAddressOf()));
}

D3D12_RASTERIZER_DESC CBlueSuitUserInterfaceShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

D3D12_BLEND_DESC CBlueSuitUserInterfaceShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

void CBlueSuitUserInterfaceShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat)
{
	m_nPipelineState = 1;
	m_vpd3dPipelineState.reserve(m_nPipelineState);
	for (int i = 0; i < m_nPipelineState; ++i)
	{
		m_vpd3dPipelineState.emplace_back();
	}

	CShader::CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nRenderTargets, pdxgiRtvFormats, dxgiDsvFormat); //m_ppd3dPipelineStates[0] 생성
}

void CBlueSuitUserInterfaceShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	shared_ptr<CMesh> pmeshRect = make_shared<CUserInterfaceRectMesh>(pd3dDevice, pd3dCommandList, 1.f, 1.f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f);

	//CROSSHAIR
	shared_ptr<CTexture> ptexCrosshair = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexCrosshair->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/crosshair.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexCrosshair, 0, 3);

	shared_ptr<CMaterial> pmatCrosshair = make_shared<CMaterial>(1);
	pmatCrosshair->SetMaterialType(MATERIAL_ALBEDO_MAP);
	pmatCrosshair->SetTexture(ptexCrosshair);

	float fxScale = float(FRAME_BUFFER_HEIGHT) / FRAME_BUFFER_WIDTH;
	shared_ptr<CGameObject> pCrosshair = make_shared<CGameObject>(pd3dDevice, pd3dCommandList, 1);
	pCrosshair->SetMaterial(0, pmatCrosshair);
	pCrosshair->SetMesh(pmeshRect);
	pCrosshair->SetScale(0.02f * fxScale, 0.02f, 1.0f);

	//TeleportUI
	shared_ptr<CTexture> ptexMaskTeleport = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexMaskTeleport->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/teleportMaskUI.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexMaskTeleport, 0, 3);
	m_vpmatTeleport[0] = make_shared<CMaterial>(1);
	m_vpmatTeleport[0]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatTeleport[0]->SetTexture(ptexMaskTeleport);

	shared_ptr<CTexture> ptexTeleport = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexTeleport->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/teleportUI.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexTeleport, 0, 3);
	m_vpmatTeleport[1] = make_shared<CMaterial>(1);
	m_vpmatTeleport[1]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatTeleport[1]->SetTexture(ptexTeleport);

	m_pTeleport = make_shared<CGameObject>(pd3dDevice, pd3dCommandList, 1);
	m_pTeleport->SetMaterial(0, m_vpmatTeleport[0]);
	m_pTeleport->SetMesh(pmeshRect);
	m_pTeleport->SetScale(0.2f * fxScale, 0.2f, 1.0f);
	m_pTeleport->SetPosition(-0.9f, 0.85f, 0.1f);

	//RadarUI
	shared_ptr<CTexture> ptexMaskRadar = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexMaskRadar->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/radarMaskUI.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexMaskRadar, 0, 3);
	m_vpmatRadar[0] = make_shared<CMaterial>(1);
	m_vpmatRadar[0]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatRadar[0]->SetTexture(ptexMaskRadar);

	shared_ptr<CTexture> ptexRadar = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexRadar->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/radarUI.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexRadar, 0, 3);
	m_vpmatRadar[1] = make_shared<CMaterial>(1);
	m_vpmatRadar[1]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatRadar[1]->SetTexture(ptexRadar);

	m_pRadar = make_shared<CGameObject>(pd3dDevice, pd3dCommandList, 1);
	m_pRadar->SetMaterial(0, m_vpmatRadar[0]);
	m_pRadar->SetMesh(pmeshRect);
	m_pRadar->SetScale(0.2f * fxScale, 0.2f, 1.0f);
	m_pRadar->SetPosition(-0.9f + 0.21f * fxScale * 1, 0.85f, 0.1f);

	//MineUI
	shared_ptr<CTexture> ptexMaskMine = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexMaskMine->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/mineMaskUI.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexMaskMine, 0, 3);
	m_vpmatMine[0] = make_shared<CMaterial>(1);
	m_vpmatMine[0]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatMine[0]->SetTexture(ptexMaskMine);

	shared_ptr<CTexture> ptexMine = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexMine->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/mineUI.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexMine, 0, 3);
	m_vpmatMine[1] = make_shared<CMaterial>(1);
	m_vpmatMine[1]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatMine[1]->SetTexture(ptexMine);

	m_pMine = make_shared<CGameObject>(pd3dDevice, pd3dCommandList, 1);
	m_pMine->SetMaterial(0, m_vpmatMine[0]);
	m_pMine->SetMesh(pmeshRect);
	m_pMine->SetScale(0.2f * fxScale, 0.2f, 1.0f);
	m_pMine->SetPosition(-0.9f + 0.21f * fxScale * 2, 0.85f, 0.1f);

	//FuseUI
	shared_ptr<CTexture> ptexMaskFuseOne = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexMaskFuseOne->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/fuseOneMaskUI.dds", RESOURCE_TEXTURE2D, 0);
	m_vpmatFuse[0] = make_shared<CMaterial>(1);
	m_vpmatFuse[0]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatFuse[0]->SetTexture(ptexMaskFuseOne, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexMaskFuseOne, 0, 3);

	shared_ptr<CTexture> ptexFuseOne = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexFuseOne->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/fuseOneUI.dds", RESOURCE_TEXTURE2D, 0);
	m_vpmatFuse[1] = make_shared<CMaterial>(1);
	m_vpmatFuse[1]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatFuse[1]->SetTexture(ptexFuseOne, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexFuseOne, 0, 3);
	shared_ptr<CTexture> ptexFuseTwo = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);

	ptexFuseTwo->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/fuseTwoUI.dds", RESOURCE_TEXTURE2D, 0);
	m_vpmatFuse[2] = make_shared<CMaterial>(1);
	m_vpmatFuse[2]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatFuse[2]->SetTexture(ptexFuseTwo, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexFuseTwo, 0, 3);

	shared_ptr<CTexture> ptexFuseThree = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexFuseThree->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/fuseThreeUI.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexFuseThree, 0, 3);
	m_vpmatFuse[3] = make_shared<CMaterial>(1);
	m_vpmatFuse[3]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatFuse[3]->SetTexture(ptexFuseThree, 0);

	m_pFuse = make_shared<CGameObject>(pd3dDevice, pd3dCommandList, 1);
	m_pFuse->SetMaterial(0, m_vpmatFuse[0]);
	m_pFuse->SetMesh(pmeshRect);
	m_pFuse->SetScale(0.2f * fxScale, 0.2f, 1.0f);
	m_pFuse->SetPosition(-0.9f + 0.21f * fxScale * 3, 0.85f, 0.1f);

	//selectRoundedRectangle.dds
	shared_ptr<CTexture> ptexSelect = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexSelect->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/selectRoundedRectangle.dds", RESOURCE_TEXTURE2D, 0);
	m_pmatSelect = make_shared<CMaterial>(1);
	m_pmatSelect->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_pmatSelect->SetTexture(ptexSelect, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexSelect, 0, 3);

	m_pSelectRect = make_shared<CGameObject>(pd3dDevice, pd3dCommandList, 1);
	m_pSelectRect->SetMaterial(0, m_pmatSelect);
	m_pSelectRect->SetMesh(pmeshRect);
	m_pSelectRect->SetScale(0.2f * fxScale, 0.2f, 1.0f);
	m_pSelectRect->SetAlive(false);

	//STAMINA BAR
	shared_ptr<CTexture> ptexStaminaBG = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexStaminaBG->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/staminaBarBG.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexStaminaBG, 0, 3);
	m_vpmatStamina[0] = make_shared<CMaterial>(1);
	m_vpmatStamina[0]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatStamina[0]->SetTexture(ptexStaminaBG);

	shared_ptr<CTexture> ptexStamina = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexStamina->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/staminaBar.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexStamina, 0, 3);
	m_vpmatStamina[1] = make_shared<CMaterial>(1);
	m_vpmatStamina[1]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatStamina[1]->SetTexture(ptexStamina);

	m_vpStamina[0] = make_shared<CGameObject>(pd3dDevice, pd3dCommandList, 1);
	m_vpStamina[0]->SetMaterial(0, m_vpmatStamina[0]);
	m_vpStamina[0]->SetMesh(pmeshRect);
	m_vpStamina[0]->SetScale(fxScale, 0.1f, 1.0f);
	m_vpStamina[0]->SetPosition(0.0f, -0.75f, 0.01f);
	m_vpStamina[0]->SetAlive(false);

	m_pmeshStaminaRect = make_shared<CUserInterfaceRectMesh>(pd3dDevice, pd3dCommandList, 1.f, 1.f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f);
	m_pmeshStaminaRect->UpdateShaderVariables(pd3dCommandList);
	m_vpStamina[1] = make_shared<CGameObject>(pd3dDevice, pd3dCommandList, 1);
	m_vpStamina[1]->SetMaterial(0, m_vpmatStamina[1]);
	m_vpStamina[1]->SetMesh(m_pmeshStaminaRect);
	m_vpStamina[1]->SetScale(fxScale* (844.0f / 860.0f), 0.1f, 1.0f);	//  (844.0f / 860.0f) -> 이미지 크기
	m_vpStamina[1]->SetPosition(0.0f, -0.75f, 0.0f);
	m_vpStamina[1]->SetAlive(false);

	shared_ptr<CTexture> ptexGameWin = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexGameWin->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/GameWin.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexGameWin, 0, 3);
	m_vpmatGameEnding[PLAYER_RESULT::WIN] = make_shared<CMaterial>(1);
	m_vpmatGameEnding[PLAYER_RESULT::WIN]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatGameEnding[PLAYER_RESULT::WIN]->SetTexture(ptexGameWin);

	shared_ptr<CTexture> ptexGameOver = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexGameOver->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/GameOver.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexGameOver, 0, 3);
	m_vpmatGameEnding[PLAYER_RESULT::OVER] = make_shared<CMaterial>(1);
	m_vpmatGameEnding[PLAYER_RESULT::OVER]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatGameEnding[PLAYER_RESULT::OVER]->SetTexture(ptexGameOver);

	m_pGameEnding = make_shared<CGameObject>(pd3dDevice, pd3dCommandList, 1);
	m_pGameEnding->SetMaterial(0, nullptr);
	m_pGameEnding->SetMesh(pmeshRect);
	m_pGameEnding->SetScale(1.0f, 1.0f, 1.0f);
	m_pGameEnding->SetPosition(0.0f, 0.0f, 0.01f);
	m_pGameEnding->SetAlive(false);

	AddGameObject(pCrosshair);
	AddGameObject(m_pTeleport);
	AddGameObject(m_pRadar);
	AddGameObject(m_pMine);
	AddGameObject(m_pFuse);
	AddGameObject(m_pSelectRect);
	AddGameObject(m_vpStamina[0]);
	AddGameObject(m_vpStamina[1]);
	AddGameObject(m_pGameEnding);
}

void CBlueSuitUserInterfaceShader::AnimateObjects(float fElapsedTime)
{
	if (m_pBlueSuitPlayer)
	{
		AnimateObjectBlueSuit(fElapsedTime);
	}

	CShader::AnimateObjects(fElapsedTime);
}

void CBlueSuitUserInterfaceShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, const shared_ptr<CPlayer>& pPlayer, int nPipelineState)
{
	UpdatePipeLineState(pd3dCommandList, nPipelineState);

	for (auto& object : m_vGameObjects)
	{
		if (!object->IsAlive())
		{
			continue;
		}
		object->Render(pd3dCommandList);
	}
}

void CBlueSuitUserInterfaceShader::AnimateObjectBlueSuit(float fElapsedTime)
{
	switch (m_nGameState)
	{
	case GAME_STATE::IN_GAME:
		AnimateInGame();
		break;
	case GAME_STATE::ZOMBIE_WIN:
		m_fEndingElapsedTime += fElapsedTime;
		if (m_fEndingElapsedTime > 3.0) m_fEndingElapsedTime = 3.0f;
		m_pGameEnding->SetAlive(true);
		m_pGameEnding->SetMaterial(0, m_vpmatGameEnding[PLAYER_RESULT::OVER]);
		m_pGameEnding->m_vpMaterials[0]->m_xmf4AlbedoColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 2.0f - m_fEndingElapsedTime / 3.0f);
		break;
	case GAME_STATE::BLUE_SUIT_WIN:
		m_pGameEnding->SetAlive(true);
		if (m_pBlueSuitPlayer->IsAlive())
		{
			m_pGameEnding->SetMaterial(0, m_vpmatGameEnding[PLAYER_RESULT::WIN]);
		}
		else
		{
			m_fEndingElapsedTime += fElapsedTime;
			if (m_fEndingElapsedTime > 3.0) m_fEndingElapsedTime = 3.0f;
			m_pGameEnding->SetMaterial(0, m_vpmatGameEnding[PLAYER_RESULT::OVER]);
			m_pGameEnding->m_vpMaterials[0]->m_xmf4AlbedoColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 2.0f - m_fEndingElapsedTime / 3.0f);
		}
		break;
	default:
		break;
	}
}

void CBlueSuitUserInterfaceShader::AnimateInGame()
{
	if (m_pBlueSuitPlayer->GetReferenceSlotItemNum(0) >= 0) m_pTeleport->SetMaterial(0, m_vpmatTeleport[1]);
	else m_pTeleport->SetMaterial(0, m_vpmatTeleport[0]);

	if (m_pBlueSuitPlayer->GetReferenceSlotItemNum(1) >= 0) m_pRadar->SetMaterial(0, m_vpmatRadar[1]);
	else  m_pRadar->SetMaterial(0, m_vpmatRadar[0]);

	if (m_pBlueSuitPlayer->GetReferenceSlotItemNum(2) >= 0) m_pMine->SetMaterial(0, m_vpmatMine[1]);
	else  m_pMine->SetMaterial(0, m_vpmatMine[0]);

	int nFuseNum = 0;
	for (int i = 0; i < 3; ++i)
	{
		if (m_pBlueSuitPlayer->GetReferenceFuseItemNum(i) >= 0)
		{
			++nFuseNum;
		}
	}
	if (nFuseNum < 4) m_pFuse->SetMaterial(0, m_vpmatFuse[nFuseNum]);

	int nSelectItem = m_pBlueSuitPlayer->GetSelectItem();
	if (nSelectItem != 0)
	{
		m_pSelectRect->SetAlive(true);
		float fxScale = float(FRAME_BUFFER_HEIGHT) / FRAME_BUFFER_WIDTH;
		m_pSelectRect->SetPosition(-0.9f + 0.21f * fxScale * (nSelectItem - 1), 0.85f, 0.01f);
	}
	else
	{
		m_pSelectRect->SetAlive(false);
	}

	if (m_pBlueSuitPlayer->GetFullStaminaTime() > 2.0f)
	{
		m_vpStamina[0]->SetAlive(false);
		m_vpStamina[1]->SetAlive(false);
	}
	else
	{
		m_vpStamina[0]->SetAlive(true);
		m_vpStamina[1]->SetAlive(true);

		float fxScale = float(FRAME_BUFFER_HEIGHT) / FRAME_BUFFER_WIDTH;
		float fStamina = m_pBlueSuitPlayer->GetStamina();
		fStamina = fStamina / BLUESUIT_STAMINA_MAX;
		m_pmeshStaminaRect->SetVertexData(1.0f * fStamina, 1.0f, 0.5f + fStamina * 0.5f, 1.0f, 0.5f - fStamina * 0.5f, 0.0f);
	}
}

void CBlueSuitUserInterfaceShader::AddGameObject(const shared_ptr<CGameObject>& pGameObject)
{
	if (dynamic_pointer_cast<CBlueSuitPlayer>(pGameObject))
	{
		m_pBlueSuitPlayer = dynamic_pointer_cast<CBlueSuitPlayer>(pGameObject);
	}
	else
	{
		m_vGameObjects.push_back(pGameObject);
	}
}

/// <CShader - CBlueSuitUserInterfaceShader>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CShader - CZombieUserInterfaceShader>


D3D12_INPUT_LAYOUT_DESC CZombieUserInterfaceShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CZombieUserInterfaceShader::CreateVertexShader()
{
	return CShader::ReadCompiledShaderFromFile(L"cso/VSUserInterface.cso", m_pd3dVertexShaderBlob.GetAddressOf());
}

D3D12_SHADER_BYTECODE CZombieUserInterfaceShader::CreatePixelShader()
{
	return CShader::ReadCompiledShaderFromFile(L"cso/PSUserInterface.cso", m_pd3dPixelShaderBlob.GetAddressOf());
}

D3D12_RASTERIZER_DESC CZombieUserInterfaceShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

D3D12_BLEND_DESC CZombieUserInterfaceShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

void CZombieUserInterfaceShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat)
{
	m_nPipelineState = 1;
	m_vpd3dPipelineState.reserve(m_nPipelineState);
	for (int i = 0; i < m_nPipelineState; ++i)
	{
		m_vpd3dPipelineState.emplace_back();
	}

	CShader::CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nRenderTargets, pdxgiRtvFormats, dxgiDsvFormat); //m_ppd3dPipelineStates[0] 생성
}

void CZombieUserInterfaceShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	shared_ptr<CMesh> pmeshRect = make_shared<CUserInterfaceRectMesh>(pd3dDevice, pd3dCommandList, 1.f, 1.f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f);

	//CROSSHAIR
	shared_ptr<CTexture> ptexCrosshair = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexCrosshair->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/crosshair.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexCrosshair, 0, 3);

	shared_ptr<CMaterial> pmatCrosshair = make_shared<CMaterial>(1);
	pmatCrosshair->SetMaterialType(MATERIAL_ALBEDO_MAP);
	pmatCrosshair->SetTexture(ptexCrosshair);

	float fxScale = float(FRAME_BUFFER_HEIGHT) / FRAME_BUFFER_WIDTH;
	shared_ptr<CGameObject> pCrosshair = make_shared<CGameObject>(pd3dDevice, pd3dCommandList, 1);
	pCrosshair->SetMaterial(0, pmatCrosshair);
	pCrosshair->SetMesh(pmeshRect);
	pCrosshair->SetScale(0.02f * fxScale, 0.02f, 1.0f);

	//TrackingUI
	shared_ptr<CTexture> ptexTrackingOn = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexTrackingOn->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/zomTrackingUI0.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexTrackingOn, 0, 3);
	m_vpmatTracking[0] = make_shared<CMaterial>(1);
	m_vpmatTracking[0]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatTracking[0]->SetTexture(ptexTrackingOn);

	shared_ptr<CTexture> ptexTrackingUsing = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexTrackingUsing->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/zomTrackingUI1.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexTrackingUsing, 0, 3);
	m_vpmatTracking[1] = make_shared<CMaterial>(1);
	m_vpmatTracking[1]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatTracking[1]->SetTexture(ptexTrackingUsing);

	shared_ptr<CTexture> ptexTrackingCoolTime = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexTrackingCoolTime->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/zomTrackingUI2.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexTrackingCoolTime, 0, 3);
	m_vpmatTracking[2] = make_shared<CMaterial>(1);
	m_vpmatTracking[2]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatTracking[2]->SetTexture(ptexTrackingCoolTime);

	m_pTracking = make_shared<CGameObject>(pd3dDevice, pd3dCommandList, 1);
	m_pTracking->SetMaterial(0, m_vpmatTracking[0]);
	m_pTracking->SetMesh(pmeshRect);
	m_pTracking->SetScale(0.2f * fxScale, 0.2f, 1.0f);
	m_pTracking->SetPosition(-0.9f, 0.85f, 0.0f);

	//Interruption
	shared_ptr<CTexture> ptexInterruptionOn = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexInterruptionOn->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/zomInterruptionUI0.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexInterruptionOn, 0, 3);
	m_vpmatInterruption[0] = make_shared<CMaterial>(1);
	m_vpmatInterruption[0]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatInterruption[0]->SetTexture(ptexInterruptionOn);

	shared_ptr<CTexture> ptexInterruptionUsing = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexInterruptionUsing->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/zomInterruptionUI1.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexInterruptionUsing, 0, 3);
	m_vpmatInterruption[1] = make_shared<CMaterial>(1);
	m_vpmatInterruption[1]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatInterruption[1]->SetTexture(ptexInterruptionUsing);

	shared_ptr<CTexture> ptexInterruptionCool = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexInterruptionCool->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/zomInterruptionUI2.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexInterruptionCool, 0, 3);
	m_vpmatInterruption[2] = make_shared<CMaterial>(1);
	m_vpmatInterruption[2]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatInterruption[2]->SetTexture(ptexInterruptionCool);

	m_pInterruption = make_shared<CGameObject>(pd3dDevice, pd3dCommandList, 1);
	m_pInterruption->SetMaterial(0, m_vpmatInterruption[0]);
	m_pInterruption->SetMesh(pmeshRect);
	m_pInterruption->SetScale(0.2f * fxScale, 0.2f, 1.0f);
	m_pInterruption->SetPosition(-0.9f + 0.21f * fxScale * 1, 0.85f, 0.0f);

	//Running
	shared_ptr<CTexture> ptexRunningOn = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexRunningOn->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/zomRunningUI0.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexRunningOn, 0, 3);
	m_vpmatRunning[0] = make_shared<CMaterial>(1);
	m_vpmatRunning[0]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatRunning[0]->SetTexture(ptexRunningOn);

	shared_ptr<CTexture> ptexRunningUsing = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexRunningUsing->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/zomRunningUI1.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexRunningUsing, 0, 3);
	m_vpmatRunning[1] = make_shared<CMaterial>(1);
	m_vpmatRunning[1]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatRunning[1]->SetTexture(ptexRunningUsing);

	shared_ptr<CTexture> ptexRunningCool = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexRunningCool->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/zomRunningUI2.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexRunningCool, 0, 3);
	m_vpmatRunning[2] = make_shared<CMaterial>(1);
	m_vpmatRunning[2]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatRunning[2]->SetTexture(ptexRunningCool);

	m_pRunning = make_shared<CGameObject>(pd3dDevice, pd3dCommandList, 1);
	m_pRunning->SetMaterial(0, m_vpmatRunning[0]);
	m_pRunning->SetMesh(pmeshRect);
	m_pRunning->SetScale(0.2f * fxScale, 0.2f, 1.0f);
	m_pRunning->SetPosition(-0.9f + 0.21f * fxScale * 2, 0.85f, 0.0f);

	shared_ptr<CTexture> ptexGameWin = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexGameWin->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/GameWin.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexGameWin, 0, 3);
	m_vpmatGameEnding[PLAYER_RESULT::WIN] = make_shared<CMaterial>(1);
	m_vpmatGameEnding[PLAYER_RESULT::WIN]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatGameEnding[PLAYER_RESULT::WIN]->SetTexture(ptexGameWin);

	shared_ptr<CTexture> ptexGameOver = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	ptexGameOver->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/UI/GameOver.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, ptexGameOver, 0, 3);
	m_vpmatGameEnding[PLAYER_RESULT::OVER] = make_shared<CMaterial>(1);
	m_vpmatGameEnding[PLAYER_RESULT::OVER]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpmatGameEnding[PLAYER_RESULT::OVER]->SetTexture(ptexGameOver);

	m_pGameEnding = make_shared<CGameObject>(pd3dDevice, pd3dCommandList, 1);
	m_pGameEnding->SetMaterial(0, nullptr);
	m_pGameEnding->SetMesh(pmeshRect);
	m_pGameEnding->SetScale(1.0f, 1.0f, 1.0f);
	m_pGameEnding->SetPosition(0.0f, 0.0f, 0.01f);
	m_pGameEnding->SetAlive(false);


	AddGameObject(pCrosshair);
	AddGameObject(m_pTracking);
	AddGameObject(m_pInterruption);
	AddGameObject(m_pRunning);
	AddGameObject(m_pGameEnding);
}

void CZombieUserInterfaceShader::AnimateObjects(float fElapsedTime)
{
	AnimateObjectZombie(fElapsedTime);

	CShader::AnimateObjects(fElapsedTime);
}

void CZombieUserInterfaceShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, const shared_ptr<CPlayer>& pPlayer, int nPipelineState)
{
	UpdatePipeLineState(pd3dCommandList, nPipelineState);

	for (auto& object : m_vGameObjects)
	{
		if (!object->IsAlive())
		{
			continue;
		}
		object->Render(pd3dCommandList);
	}
}

void CZombieUserInterfaceShader::AnimateObjectZombie(float fElapsedTime)
{
	switch (m_nGameState)
	{
	case GAME_STATE::IN_GAME:
		if (m_pZombiePlayer->IsAbleTracking()) m_pTracking->SetMaterial(0, m_vpmatTracking[0]);
		else if (m_pZombiePlayer->IsTracking()) m_pTracking->SetMaterial(0, m_vpmatTracking[1]);
		else if (!m_pZombiePlayer->IsAbleTracking()) m_pTracking->SetMaterial(0, m_vpmatTracking[2]);

		if (m_pZombiePlayer->IsAbleInterruption()) m_pInterruption->SetMaterial(0, m_vpmatInterruption[0]);
		else if (m_pZombiePlayer->IsInterruption()) m_pInterruption->SetMaterial(0, m_vpmatInterruption[1]);
		else if (!m_pZombiePlayer->IsAbleInterruption()) m_pInterruption->SetMaterial(0, m_vpmatInterruption[2]);

		if (m_pZombiePlayer->IsAbleRunning()) m_pRunning->SetMaterial(0, m_vpmatRunning[0]);
		else if (m_pZombiePlayer->IsRunning()) m_pRunning->SetMaterial(0, m_vpmatRunning[1]);
		else if (!m_pZombiePlayer->IsAbleRunning()) m_pRunning->SetMaterial(0, m_vpmatRunning[2]);
		break;
	case GAME_STATE::BLUE_SUIT_WIN:
		m_pGameEnding->SetAlive(true);
		m_pGameEnding->SetMaterial(0, m_vpmatGameEnding[PLAYER_RESULT::OVER]);
		break;
	case GAME_STATE::ZOMBIE_WIN:
		m_fEndingElapsedTime += fElapsedTime;
		if (m_fEndingElapsedTime > 3.0) m_fEndingElapsedTime = 3.0f;
		m_pGameEnding->SetAlive(true);
		m_pGameEnding->SetMaterial(0, m_vpmatGameEnding[PLAYER_RESULT::WIN]);
		m_pGameEnding->m_vpMaterials[0]->m_xmf4AlbedoColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 2.0f - m_fEndingElapsedTime / 3.0f);
		break;
	default:
		break;
	}
}

void CZombieUserInterfaceShader::AddGameObject(const shared_ptr<CGameObject>& pGameObject)
{
	if (dynamic_pointer_cast<CZombiePlayer>(pGameObject))
	{
		m_pZombiePlayer = dynamic_pointer_cast<CZombiePlayer>(pGameObject);
	}
	else
	{
		m_vGameObjects.push_back(pGameObject);
	}
}

/// <CShader - CZombieUserInterfaceShader>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CShader - StandardShader - OutLineShader>

COutLineShader::COutLineShader(int nMainPlayer) 
{
	m_nMainPlayer = nMainPlayer;
}

D3D12_INPUT_LAYOUT_DESC COutLineShader::CreateInputLayout()
{
	if (m_PipeLineIndex == STANDARD_OUT_LINE || m_PipeLineIndex == STANDARD_OUT_LINE_MASK)
	{
		UINT nInputElementDescs = 5;
		D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

		pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

		D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
		d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
		d3dInputLayoutDesc.NumElements = nInputElementDescs;

		return(d3dInputLayoutDesc);
	}
	else if(m_PipeLineIndex == INSTANCE_OUT_LINE || m_PipeLineIndex == INSTANCE_OUT_LINE_MASK)
	{
		UINT nInputElementDescs = 9;
		D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

		pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

		pd3dInputElementDescs[5] = { "INSMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
		pd3dInputElementDescs[6] = { "INSMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
		pd3dInputElementDescs[7] = { "INSMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
		pd3dInputElementDescs[8] = { "INSMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

		D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
		d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
		d3dInputLayoutDesc.NumElements = nInputElementDescs;

		return(d3dInputLayoutDesc);
	}
	else if(m_PipeLineIndex == SKINNING_OUT_LINE || m_PipeLineIndex == SKINNING_OUT_LINE_MASK)
	{
		UINT nInputElementDescs = 7;
		D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

		pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

		D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
		d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
		d3dInputLayoutDesc.NumElements = nInputElementDescs;

		return(d3dInputLayoutDesc);
	}
}

D3D12_SHADER_BYTECODE COutLineShader::CreateVertexShader()
{
	if (m_PipeLineIndex == STANDARD_OUT_LINE_MASK)
	{
		return CShader::ReadCompiledShaderFromFile(L"cso/VSStandardOutLineMask.cso", m_pd3dVertexShaderBlob.GetAddressOf());
	}
	else if (m_PipeLineIndex == INSTANCE_OUT_LINE_MASK)
	{
		return CShader::ReadCompiledShaderFromFile(L"cso/VSInstanceOutLineMask.cso", m_pd3dVertexShaderBlob.GetAddressOf());
	}
	else if(m_PipeLineIndex == SKINNING_OUT_LINE_MASK)
	{
		return CShader::ReadCompiledShaderFromFile(L"cso/VSOutLineMask.cso", m_pd3dVertexShaderBlob.GetAddressOf());
	}
	else if (m_PipeLineIndex == STANDARD_OUT_LINE)
	{
		return CShader::ReadCompiledShaderFromFile(L"cso/VSStandardOutLine.cso", m_pd3dVertexShaderBlob.GetAddressOf());
	}
	else if (m_PipeLineIndex == INSTANCE_OUT_LINE)
	{
		return CShader::ReadCompiledShaderFromFile(L"cso/VSInstanceOutLine.cso", m_pd3dVertexShaderBlob.GetAddressOf());
	}
	else if(m_PipeLineIndex == SKINNING_OUT_LINE)
	{
		return CShader::ReadCompiledShaderFromFile(L"cso/VSOutLine.cso", m_pd3dVertexShaderBlob.GetAddressOf());
	}
}

D3D12_SHADER_BYTECODE COutLineShader::CreatePixelShader()
{
	return CShader::ReadCompiledShaderFromFile(L"cso/PSOutLine.cso", m_pd3dPixelShaderBlob.GetAddressOf());
}

D3D12_DEPTH_STENCIL_DESC COutLineShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	if(m_PipeLineIndex == STANDARD_OUT_LINE_MASK || m_PipeLineIndex == SKINNING_OUT_LINE_MASK || m_PipeLineIndex == INSTANCE_OUT_LINE_MASK)
	{
		d3dDepthStencilDesc.DepthEnable = TRUE;
		d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;//D3D12_DEPTH_WRITE_MASK_ALL
		d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // D3D12_COMPARISON_FUNC_LESS_EQUAL
		d3dDepthStencilDesc.StencilEnable = TRUE;
		d3dDepthStencilDesc.StencilReadMask = 0xff;
		d3dDepthStencilDesc.StencilWriteMask = 0xff;
		d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_REPLACE;
		d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_REPLACE;
		d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
		d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
		d3dDepthStencilDesc.BackFace = d3dDepthStencilDesc.FrontFace;
	}
	else
	{
		d3dDepthStencilDesc.DepthEnable = TRUE;
		d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;//D3D12_DEPTH_WRITE_MASK_ALL
		d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS; // D3D12_COMPARISON_FUNC_LESS_EQUAL
		d3dDepthStencilDesc.StencilEnable = TRUE;
		d3dDepthStencilDesc.StencilReadMask = 0xff;
		d3dDepthStencilDesc.StencilWriteMask = 0xff;
		d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
		d3dDepthStencilDesc.BackFace = d3dDepthStencilDesc.FrontFace;
	}

	return d3dDepthStencilDesc;
}

D3D12_RASTERIZER_DESC COutLineShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

void COutLineShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat)
{
	m_nPipelineState = 6;
	m_vpd3dPipelineState.reserve(m_nPipelineState);
	for (int i = 0; i < m_nPipelineState; ++i)
	{
		m_vpd3dPipelineState.emplace_back();
		CShader::CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nRenderTargets, pdxgiRtvFormats, dxgiDsvFormat);
	}
}

void COutLineShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, const shared_ptr<CPlayer>& pPlayer, int nPipelineState)
{
	pd3dCommandList->ClearDepthStencilView(m_pPostProcessingShader->GetDsvCPUDesctriptorHandle(0), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	pd3dCommandList->OMSetStencilRef(1);

	for (int i = 0; i < m_nPipelineState; ++i)
	{
		UpdatePipeLineState(pd3dCommandList, i);

		if (i == INSTANCE_OUT_LINE || i == INSTANCE_OUT_LINE_MASK)
		{	
			// 픽킹 오브젝트 렌더링
			shared_ptr<CGameObject> pPickedObject = m_pMainPlayer->GetPickedObject();
			if (pPickedObject && pPickedObject->GetCollisionType() == 2 && pPickedObject->IsInstance())
			{
				pPickedObject->Render(pd3dCommandList);
			}
		}

		if (i == STANDARD_OUT_LINE || i == STANDARD_OUT_LINE_MASK)
		{
			shared_ptr<CGameObject> pPickedObject = m_pMainPlayer->GetPickedObject();
			if (pPickedObject && pPickedObject->GetCollisionType() == 2 && !pPickedObject->IsInstance())
			{
				pPickedObject->Render(pd3dCommandList);
			}
		}

		if (i == SKINNING_OUT_LINE || i == SKINNING_OUT_LINE_MASK)
		{
			if (!m_pZombiePlayer)
			{
				continue;
			}
			// 좀비 외곽선 스킬
			if (!m_pZombiePlayer->IsTracking())
			{
				continue;
			}

			m_pZombiePlayer->Render(pd3dCommandList);
			for (const auto& pBlueSuitPlayer : m_vpBlueSuitPlayer)
			{
				pBlueSuitPlayer->Render(pd3dCommandList);
			}
		}
	}
}

void COutLineShader::AddGameObject(const shared_ptr<CGameObject>& pGameObject)
{
	if (dynamic_pointer_cast<CZombiePlayer>(pGameObject))
	{
		m_pZombiePlayer = dynamic_pointer_cast<CZombiePlayer>(pGameObject);
		if (m_nMainPlayer == ZOMBIEPLAYER)
			m_pMainPlayer = m_pZombiePlayer;
	}
	else if (dynamic_pointer_cast<CBlueSuitPlayer>(pGameObject))
	{
		m_vpBlueSuitPlayer.push_back(dynamic_pointer_cast<CBlueSuitPlayer>(pGameObject));
		if(m_nMainPlayer != ZOMBIEPLAYER)	// 셰이더의 메인 플레이어 설정필요
		{
			m_pMainPlayer = m_vpBlueSuitPlayer[0]; // 0~3
		}
	}
	else
	{
		m_vGameObjects.push_back(pGameObject);
	}
}

//D3D12_BLEND_DESC COutLineShader::CreateBlendState()
//{
//	D3D12_BLEND_DESC d3dBlendDesc;
//	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
//	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
//	d3dBlendDesc.IndependentBlendEnable = FALSE;
//	d3dBlendDesc.RenderTarget[0].BlendEnable = true;
//	d3dBlendDesc.RenderTarget[0].LogicOpEnable = false;
//	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
//	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
//	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
//	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
//	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
//	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
//	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
//	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
//	return d3dBlendDesc;
//}

PartitionInsStandardShader::PartitionInsStandardShader()
{

}

PartitionInsStandardShader::~PartitionInsStandardShader()
{

}

void PartitionInsStandardShader::AddPartitionGameObject(const shared_ptr<CGameObject>& pGameObject, int nPartition)
{
	m_vPartitionObject[nPartition].push_back(pGameObject);
}

void PartitionInsStandardShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, const shared_ptr<CPlayer>& pPlayer, int nPipelineState)
{
	if (pCamera->GetPartitionPos() == -1) {
		//assert(0); // 파티션이 없으면 종료. 테스트 후 잘 되면 주석 처리 해도 ok
		return;
	}
	UpdatePipeLineState(pd3dCommandList, nPipelineState);

	auto& ptObjects = m_vPartitionObject[pCamera->GetPartitionPos()];

	for (auto& object : ptObjects)
	{
		if (!object->m_bThisContainTransparent) {
			object->Render(pd3dCommandList);
		}
		else {
			object->RenderOpaque(pd3dCommandList);
		}
	}
}

void PartitionInsStandardShader::PartitionRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, int nPipelineState)
{
	if (pCamera->GetPartitionPos() == -1) {
		//assert(0); // 파티션이 없으면 종료. 테스트 후 잘 되면 주석 처리 해도 ok
		return;
	}
	UpdatePipeLineState(pd3dCommandList, nPipelineState);

	auto& ptObjects = m_vPartitionObject[pCamera->GetPartitionPos()];

	for (auto& object : ptObjects)
	{
		if (!object->m_bThisContainTransparent) {
			object->Render(pd3dCommandList);
		}
		else {
			object->RenderOpaque(pd3dCommandList);
		}
	}
}


void PartitionInsStandardShader::AddPartition()
{
	m_vPartitionObject.emplace_back(vector<shared_ptr<CGameObject>>());
}

void PartitionInsStandardShader::AddPartitionBB(shared_ptr<BoundingBox>& bb)
{
	m_vPartitionBB.push_back(bb);
}
