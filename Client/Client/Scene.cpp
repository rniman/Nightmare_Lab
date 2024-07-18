#include "stdafx.h"
#include "Scene.h"
#include "Shader.h"
#include "TextureBlendAnimationShader.h"
#include "Player.h"
#include "PlayerController.h"
#include "EnvironmentObject.h"
#include "Collision.h"
#include "TextureBlendObject.h"

ComPtr<ID3D12DescriptorHeap> CScene::m_pd3dCbvSrvDescriptorHeap;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorNextHandle;

vector<unique_ptr<CShader>> CMainScene::m_vShader;
//CShader* CScene::m_pRefShader;

extern bool g_InstanceMeshNotAddCollision;

int ReadLightObjectInfo(vector<XMFLOAT3>& positions, vector<XMFLOAT3>& looks);
void PartisionShaderCollision(unique_ptr<PartitionInsStandardShader>& PtShader, shared_ptr<CGameObject>& pObject);

void CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[14];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; //b0 Camera: 
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 1; //b1 GameObject: 
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 2; //b2 Light?: 
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 0; //t0: AlbedoTexture
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 1; //t1: SpecularTexture
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 2; //t2: NormalTexture
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 3; //t3: MetallicTexture
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[7].NumDescriptors = 1;
	pd3dDescriptorRanges[7].BaseShaderRegister = 4; //t4: EmissionTexture
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[8].NumDescriptors = 1;
	pd3dDescriptorRanges[8].BaseShaderRegister = 3; //b3: gpmtxBoneOffsets
	pd3dDescriptorRanges[8].RegisterSpace = 0;
	pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[9].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[9].NumDescriptors = 1;
	pd3dDescriptorRanges[9].BaseShaderRegister = 4; //b4: gpmtxBoneTransforms
	pd3dDescriptorRanges[9].RegisterSpace = 0;
	pd3dDescriptorRanges[9].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[10].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[10].NumDescriptors = 4;
	pd3dDescriptorRanges[10].BaseShaderRegister = 5; //t5,t6,t7,t8: Deferred Render Texture
	pd3dDescriptorRanges[10].RegisterSpace = 0;
	pd3dDescriptorRanges[10].OffsetInDescriptorsFromTableStart = 0;

	pd3dDescriptorRanges[11].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[11].NumDescriptors = 28;
	pd3dDescriptorRanges[11].BaseShaderRegister = 20; //t20~ Shadow Map
	pd3dDescriptorRanges[11].RegisterSpace = 0;
	pd3dDescriptorRanges[11].OffsetInDescriptorsFromTableStart = 0;

	pd3dDescriptorRanges[12].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[12].NumDescriptors = 1;
	pd3dDescriptorRanges[12].BaseShaderRegister = 5; //b5
	pd3dDescriptorRanges[12].RegisterSpace = 0;
	pd3dDescriptorRanges[12].OffsetInDescriptorsFromTableStart = 0;

	pd3dDescriptorRanges[13].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[13].NumDescriptors = 1;
	pd3dDescriptorRanges[13].BaseShaderRegister = 9; //t9 patterntexture
	pd3dDescriptorRanges[13].RegisterSpace = 0;
	pd3dDescriptorRanges[13].OffsetInDescriptorsFromTableStart = 0;

	D3D12_ROOT_PARAMETER pd3dRootParameters[14];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[0].DescriptorTable.NumDescriptorRanges = 1; //Camera
	pd3dRootParameters[0].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[1].DescriptorTable.NumDescriptorRanges = 1; //GameObject
	pd3dRootParameters[1].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1; //Lights
	pd3dRootParameters[2].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1; //AlbedoTexture
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1; //SpecularTexture
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]);
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1; //NormalTexture
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]);
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1; //MetallicTexture
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[6]);
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1; //EmissionTexture
	pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[7]);
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[8].DescriptorTable.NumDescriptorRanges = 1; //gpmtxBoneOffsets
	pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[8]);
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[9].DescriptorTable.NumDescriptorRanges = 1; //gpmtxBoneTransforms
	pd3dRootParameters[9].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[9]);
	pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[10].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[10].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[10]; //Deferred Render Texture
	pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[11].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[11].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[11]; //Shadow Map
	pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[12].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[12].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[12]; //cbFrameInfo
	pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	// -> 일단 임시로 ALL(TrackingTime)

	pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[13].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[13].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[13]; //pattern Texture
	pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDescs[3];

	d3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDescs[0].MipLODBias = 0;
	d3dSamplerDescs[0].MaxAnisotropy = 1;
	d3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDescs[0].MinLOD = 0;
	d3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDescs[0].ShaderRegister = 0;
	d3dSamplerDescs[0].RegisterSpace = 0;
	d3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


	d3dSamplerDescs[1].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	d3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	d3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	d3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	d3dSamplerDescs[1].MipLODBias = 0.0f;
	d3dSamplerDescs[1].MaxAnisotropy = 1;
	d3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; //D3D12_COMPARISON_FUNC_LESS
	d3dSamplerDescs[1].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE; // D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	d3dSamplerDescs[1].MinLOD = 0;
	d3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDescs[1].ShaderRegister = 2;
	d3dSamplerDescs[1].RegisterSpace = 0;
	d3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	d3dSamplerDescs[2].Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	d3dSamplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDescs[2].MipLODBias = 0;
	d3dSamplerDescs[2].MaxAnisotropy = 1;
	d3dSamplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDescs[2].MinLOD = 0;
	d3dSamplerDescs[2].MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDescs[2].ShaderRegister = 3;
	d3dSamplerDescs[2].RegisterSpace = 0;
	d3dSamplerDescs[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(d3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = d3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(),
		pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)
		&m_pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) {
		pd3dSignatureBlob->Release();
	}
	if (pd3dErrorBlob) {
		pd3dErrorBlob->Release();
	}
}

int CScene::m_nCntCbv = 0;
int CScene::m_nCntSrv = 0;

void CScene::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	if (m_pd3dCbvSrvDescriptorHeap.Get())
	{
		m_pd3dCbvSrvDescriptorHeap.Reset();
	}

	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT h= pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorNextHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorNextHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
{
	m_nCntCbv++;
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorNextHandle;
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_d3dCbvCPUDescriptorNextHandle);
		m_d3dCbvCPUDescriptorNextHandle.ptr = m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
		m_d3dCbvGPUDescriptorNextHandle.ptr = m_d3dCbvGPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
	}
	return(d3dCbvGPUDescriptorHandle);
}

void CScene::CreateShaderResourceViews(ID3D12Device* pd3dDevice, const shared_ptr<CTexture>& pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_nCntSrv++;
	m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
	m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource* pShaderResource = pTexture->GetResource(i);
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
			pTexture->SetGpuDescriptorHandle(i, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int j = 0; j < nRootParameters; j++) pTexture->SetRootParameterIndex(j, nRootParameterStartIndex + j);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateShaderResourceView(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dResource, DXGI_FORMAT dxgiSrvFormat)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	d3dShaderResourceViewDesc.Format = dxgiSrvFormat;
	d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	d3dShaderResourceViewDesc.Texture2D.MipLevels = 1;
	d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
	d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorNextHandle;
	pd3dDevice->CreateShaderResourceView(pd3dResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
	m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

	return(d3dSrvGPUDescriptorHandle);
}

/// <CScene>
/////////////////////////////////////////////////////////////////////
/// <CScene - CLobbyScene>

CLobbyScene::CLobbyScene(HWND hWnd, weak_ptr<CCamera>& pCamera)
{
	m_hWnd = hWnd;

	m_pCamera = make_shared<CCamera>();
	m_pCamera->GenerateProjectionMatrix(0.01f, 50.0f, ASPECT_RATIO, 45);
	m_pCamera->RegenerateViewMatrix();
	pCamera = m_pCamera;
}

bool CLobbyScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) 
{ 
	shared_ptr<CLobbyUserInterfaceShader> pLobbyUIShader = dynamic_pointer_cast<CLobbyUserInterfaceShader>(m_vpShader[LOBBY_UI_SHADER]);
	POINT ptCursorPos = m_ptCursor;
	ptCursorPos.y = FRAME_BUFFER_HEIGHT - ptCursorPos.y;
	switch (nMessageID)
	{
	case WM_MOUSEMOVE:
		m_ptCursor.x = ((int)(short)LOWORD(lParam));
		m_ptCursor.y = ((int)(short)HIWORD(lParam));

		break;
	case WM_LBUTTONDOWN:
	{
		ProcessButtonDown(ptCursorPos, pLobbyUIShader);
		ProcessClickBorder(ptCursorPos, pLobbyUIShader);
		// 선택된 슬롯 번호
		m_nSelectedSlot = pLobbyUIShader->GetSelectedBorder();
	}
		break;
	case WM_LBUTTONUP:
	{
		int nRetVal = pLobbyUIShader->ProcessInput(LOBBY_PROCESS_INPUT::START_BUTTON_UP);
		if (nRetVal == 1)	// GAME START
		{
			PostMessage(m_hWnd, WM_SOCKET, NULL, MAKELPARAM(FD_WRITE, 0));
			break;
		}
		nRetVal = pLobbyUIShader->ProcessInput(LOBBY_PROCESS_INPUT::CHANGE_BUTTON_UP);
		if (nRetVal == 2)
		{
			// CHANGE 
			PostMessage(hWnd, WM_CHANGE_SLOT, 0, 0);
			break;
		}
	}
	break;
	}
	return false;
}

void CLobbyScene::ProcessButtonDown(const POINT& ptCursorPos, std::shared_ptr<CLobbyUserInterfaceShader>& pLobbyUIShader)
{
	float fWidth, fHeight;
	float fCenterX, fCenterY;
	fCenterX = FRAME_BUFFER_WIDTH * 0.75f;
	fCenterY = FRAME_BUFFER_HEIGHT * 0.2f;
	fWidth = FRAME_BUFFER_WIDTH / 2.0f * 0.5f;

	float fButtonScale = 160.0f / 680.0f;
	fHeight = FRAME_BUFFER_HEIGHT / 2.0f * fButtonScale;


	if (CheckCursor(ptCursorPos, fCenterX, fCenterY, fWidth, fHeight))
	{
		pLobbyUIShader->ProcessInput(LOBBY_PROCESS_INPUT::START_BUTTON_DOWN);
	}

	fCenterX = FRAME_BUFFER_WIDTH * 0.25f;
	if (CheckCursor(ptCursorPos, fCenterX, fCenterY, fWidth, fHeight))
	{
		pLobbyUIShader->ProcessInput(LOBBY_PROCESS_INPUT::CHANGE_BUTTON_DOWN);
	}
}

void CLobbyScene::ProcessClickBorder(const POINT& ptCursorPos, std::shared_ptr<CLobbyUserInterfaceShader>& pLobbyUIShader)
{
	// BORDER
	float fWidth, fHeight;
	float fCenterX, fCenterY;
	float fxScale = float(FRAME_BUFFER_HEIGHT) / FRAME_BUFFER_WIDTH;
	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		fCenterX = FRAME_BUFFER_WIDTH * 0.116f + FRAME_BUFFER_WIDTH * 0.192f * i;
		fCenterX = FRAME_BUFFER_WIDTH * 0.116f + FRAME_BUFFER_WIDTH / 2.0f * fxScale * 0.6f * i;
		fCenterY = FRAME_BUFFER_HEIGHT * 0.65f;
		fWidth = FRAME_BUFFER_WIDTH / 2.0f * fxScale * 0.6f;
		fHeight = FRAME_BUFFER_HEIGHT / 2.0f;
		if (CheckCursor(ptCursorPos, fCenterX, fCenterY, fWidth, fHeight))
		{
			pLobbyUIShader->ProcessInput(LOBBY_PROCESS_INPUT::BORDER_SEL + i);
		}
	}
}

bool CLobbyScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	//switch (nMessageID)
	//{
	//case WM_KEYDOWN:
	//	switch (wParam)
	//	{
	//	case VK_PRIOR:
	//	{
	//		XMFLOAT3 xmf3Up = XMFLOAT3(0.0f, 0.1f, 0.0f);
	//		m_apPlayer[0]->Move(xmf3Up, false);
	//	}
	//		break;
	//	case VK_NEXT:
	//	{
	//		XMFLOAT3 xmf3Down = XMFLOAT3(0.0f, -0.1f, 0.0f);
	//		m_apPlayer[0]->Move(xmf3Down, false);
	//	}
	//		break;
	//	case VK_UP:
	//	{
	//		XMFLOAT3 xmf3Back = XMFLOAT3(0.0f, 0.0f, 0.1f);
	//		m_apPlayer[0]->Move(xmf3Back, false);
	//	}
	//		break;
	//	case VK_DOWN:
	//	{
	//		XMFLOAT3 xmf3Forward = XMFLOAT3(0.0f, 0.0f, -0.1f);
	//		m_apPlayer[0]->Move(xmf3Forward, false);
	//	}
	//		break;
	//	case VK_RIGHT:
	//	{	
	//		XMFLOAT3 xmf3Right = XMFLOAT3(0.1f, 0.0f, 0.0f);
	//		m_apPlayer[0]->Move(xmf3Right, false); 
	//	}
	//		break;
	//	case VK_LEFT:
	//	{
	//		XMFLOAT3 xmf3Left = XMFLOAT3(-0.1f, 0.0f, 0.0f);
	//		m_apPlayer[0]->Move(xmf3Left, false); 
	//	}
	//		break;
	//	}
	//	break;
	//}

	return true;
}

void CLobbyScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int mainPlayerId)
{
	CreateGraphicsRootSignature(pd3dDevice);

	int nCntCbv = 3000;
	int nCntSrv = 1000;

	CreateCbvSrvDescriptorHeaps(pd3dDevice, nCntCbv, nCntSrv);

	// 쉐이더 vector에 삽입한 순서대로 인덱스 define한 값으로 접근
	m_vpShader.push_back(make_shared<CLobbyStandardShader>());
	m_vpShader[LOBBY_SATANDARD_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
	
	m_vpShader.push_back(make_shared<CLobbyUserInterfaceShader>(mainPlayerId));
	m_vpShader[LOBBY_UI_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());

	//Player 생성 + 아이템
	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		if (i == ZOMBIEPLAYER)
		{
			m_apPlayer[i] = std::make_shared<CZombiePlayer>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), nullptr);
			shared_ptr<CLoadedModelInfo> pZombiePlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), "Asset/Model/Zom_1.bin", MeshType::Standard);
			m_apPlayer[i]->LoadModelAndAnimation(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), pZombiePlayerModel);
			m_vpShader[LOBBY_SATANDARD_SHADER]->AddGameObject(m_apPlayer[i]);

			float fxScale = float(FRAME_BUFFER_HEIGHT) / FRAME_BUFFER_WIDTH;
			XMFLOAT3 xmf3Position = XMFLOAT3(-fxScale * 1.2f + fxScale * 0.6f * i, -0.1f, 0.5f);
			XMFLOAT3 xmf3Scale = XMFLOAT3(0.3f, 0.5f, 0.3f);
			m_apPlayer[i]->SetWorldPostion(xmf3Position);
			m_apPlayer[i]->SetScale(xmf3Scale);
			m_apPlayer[i]->Rotate(0.0f, 135.0f, .0f);
			m_apPlayer[i]->OnUpdateToParent();
		}
		else
		{
			m_apPlayer[i] = std::make_shared<CBlueSuitPlayer>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), nullptr);
			shared_ptr<CLoadedModelInfo> pBlueSuitPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), "Asset/Model/BlueSuitFree01.bin", MeshType::Standard);
			m_apPlayer[i]->LoadModelAndAnimation(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), pBlueSuitPlayerModel);
			m_vpShader[LOBBY_SATANDARD_SHADER]->AddGameObject(m_apPlayer[i]);

			float fxScale = float(FRAME_BUFFER_HEIGHT) / FRAME_BUFFER_WIDTH;
			XMFLOAT3 xmf3Position = XMFLOAT3(-fxScale * 1.2f + fxScale * 0.6f * i, -0.1f, 0.5f);
			XMFLOAT3 xmf3Scale = XMFLOAT3(0.35f, 0.35f, 0.35f);
			m_apPlayer[i]->SetWorldPostion(xmf3Position);
			m_apPlayer[i]->SetScale(xmf3Scale);
			m_apPlayer[i]->Rotate(0.0f, -165.0f, .0f);
			m_apPlayer[i]->OnUpdateToParent();
		}
	}

	m_vpShader[LOBBY_UI_SHADER]->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
}

void CLobbyScene::AnimateObjects(float fElapsedTime)
{
	for (auto& pShader : m_vpShader)
	{
		pShader->AnimateObjects(fElapsedTime);
	}
}

bool CLobbyScene::ProcessInput(UCHAR* pKeysBuffer) 
{
	shared_ptr<CLobbyUserInterfaceShader> pLobbyUIShader = dynamic_pointer_cast<CLobbyUserInterfaceShader>(m_vpShader[LOBBY_UI_SHADER]);

	POINT ptCursorPos = m_ptCursor;
	ptCursorPos.y = FRAME_BUFFER_HEIGHT - ptCursorPos.y;

	// START BUTTON
	float fWidth, fHeight;
	float fCenterX, fCenterY;
	fCenterX = FRAME_BUFFER_WIDTH * 0.75f;
	fCenterY = FRAME_BUFFER_HEIGHT * 0.2f;
	fWidth = FRAME_BUFFER_WIDTH / 2.0f * 0.5f;

	float fButtonScale = 160.0f / 680.0f;
	fHeight = FRAME_BUFFER_HEIGHT / 2.0f * fButtonScale;

	
	if (CheckCursor(ptCursorPos, fCenterX, fCenterY, fWidth, fHeight))
	{
		pLobbyUIShader->ProcessInput(LOBBY_PROCESS_INPUT::START_BUTTON_SEL);
	}
	else
	{
		pLobbyUIShader->ProcessInput(LOBBY_PROCESS_INPUT::START_BUTTON_NON);
	}

	fCenterX = FRAME_BUFFER_WIDTH * 0.25f;
	if (CheckCursor(ptCursorPos, fCenterX, fCenterY, fWidth, fHeight))
	{
		pLobbyUIShader->ProcessInput(LOBBY_PROCESS_INPUT::CHANGE_BUTTON_SEL);
	}
	else
	{
		pLobbyUIShader->ProcessInput(LOBBY_PROCESS_INPUT::CHANGE_BUTTON_NON);
	}

	return false; 
}

void CLobbyScene::PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera)
{
	if (m_pd3dGraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature.Get());
	if (m_pd3dCbvSrvDescriptorHeap)
		pd3dCommandList->SetDescriptorHeaps(1, m_pd3dCbvSrvDescriptorHeap.GetAddressOf());

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	//UpdateShaderVariables(pd3dCommandList);
}

void CLobbyScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, int nPipelineState)
{
	PrepareRender(pd3dCommandList, pCamera);

	//m_vpShader[LOBBY_UI_SHADER]->Render(pd3dCommandList, pCamera, m_pMainPlayer, 0);
	for (auto& pShader : m_vpShader)
	{
		pShader->Render(pd3dCommandList, pCamera, m_pMainPlayer, 0);
	}
}

bool CLobbyScene::CheckCursor(POINT ptCursor, float fCenterX, float fCenterY, float fWidth, float fHeight)
{
	if (ptCursor.x > fCenterX - fWidth / 2 && ptCursor.x < fCenterX + fWidth / 2 &&
		ptCursor.y > fCenterY - fHeight / 2 && ptCursor.y < fCenterY + fHeight / 2)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CLobbyScene::UpdateShaderMainPlayer(int nMainClientId) 
{
	shared_ptr<CLobbyUserInterfaceShader> pLobbyUIShader = dynamic_pointer_cast<CLobbyUserInterfaceShader>(m_vpShader[LOBBY_UI_SHADER]);
	pLobbyUIShader->UpdateShaderMainPlayer(nMainClientId);

}

/// <CScene - CLobbyScene>
/////////////////////////////////////////////////////////////////////
/// <CScene - CMainScene>

CMainScene::CMainScene()
{

}
CMainScene::~CMainScene()
{

}

void CMainScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int mainPlayerId)
{
	testAngle = 0.0f;
	CreateGraphicsRootSignature(pd3dDevice);

	// CBV(Default) :  카메라(1), 플레이어(1) , 라이트카메라(2(임시))
	// CBC(Scene Load): 66
	// CBV(RootObject) : //육면체(1), 오브젝트(1), DeskObject(1), DoorObject(1), flashLight(1), 서버인원예상(20), fuse(3)
	// CBV(Model) : Zom(72),  Zom_Controller(2 * N),// BlueSuit(85), BlueSuit_Controller(2 * N), Desk(3), Door(5), flashLight(1), Fuse(6), 레이더(5),텔레포트아이템(1),지뢰(1)
	//int nCntCbv = 1 + 1 + 2 + 66 +
	//	(72 + 2) + (85 + 2) * MAX_CLIENT + 2 + 7 + 10 + 5 * MAX_CLIENT + 1 * MAX_CLIENT + 120 + 1 + 1+2000;
	//// SRV(Default) : 디퍼드렌더링텍스처(ADD_RENDERTARGET_COUNT로 정의된 개수임)
	//// SRV(Scene Load) : 79
	//// SRV: Zombie(3), // BlueSuit(6), 육면체(1), 엘런(8(오클루젼맵제거), Desk(3), Door(9), flashLight(3) , m_nLights,지뢰(4),Electiric
	//int nCntSrv = ADD_RENDERTARGET_COUNT + 6 * MAX_CLIENT + 79 + 3 + 3 + 3 * MAX_CLIENT + m_nLights + 4 * MAX_CLIENT + 1 + 10*4+10+250;
	//CreateCbvSrvDescriptorHeaps(pd3dDevice, nCntCbv, nCntSrv);

	// 쉐이더 vector에 삽입한 순서대로 인덱스 define한 값으로 접근
	m_vShader.push_back(make_unique<StandardShader>());
	DXGI_FORMAT pdxgiRtvFormats[4] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT,DXGI_FORMAT_R32G32B32A32_FLOAT };
	DXGI_FORMAT pdxgiRtvShadowFormat = DXGI_FORMAT_R32_FLOAT;
	m_vShader[STANDARD_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), 4, pdxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);
	
	m_vShader.push_back(make_unique<InstanceStandardShader>());
	m_vShader[INSTANCE_STANDARD_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), ADD_RENDERTARGET_COUNT, pdxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);
	
	m_vShader.push_back(make_unique< CSkinnedAnimationStandardShader>());
	m_vShader[SKINNEDANIMATION_STANDARD_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), ADD_RENDERTARGET_COUNT, pdxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);
	
	m_vForwardRenderShader.push_back(make_unique<TransparentShader>());
	m_vForwardRenderShader[TRANSPARENT_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), 1, nullptr, DXGI_FORMAT_D24_UNORM_S8_UINT);
	
	m_vForwardRenderShader.push_back(make_unique<TextureBlendAnimationShader>());
	m_vForwardRenderShader[TEXTUREBLEND_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), 1, nullptr, DXGI_FORMAT_D24_UNORM_S8_UINT);

	//[0505] UI
	if (mainPlayerId == ZOMBIEPLAYER)
	{
		m_vForwardRenderShader.push_back(make_unique<CZombieUserInterfaceShader>());
		m_vForwardRenderShader[USER_INTERFACE_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), 1, nullptr, DXGI_FORMAT_D24_UNORM_S8_UINT);
	}
	else
	{
		m_vForwardRenderShader.push_back(make_unique<CBlueSuitUserInterfaceShader>());
		m_vForwardRenderShader[USER_INTERFACE_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), 1, nullptr, DXGI_FORMAT_D24_UNORM_S8_UINT);
	}

	m_vPreRenderShader.push_back(make_unique<PartitionInsStandardShader>());
	m_vPreRenderShader[PARTITION_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), 4, pdxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);

	LoadScene(pd3dDevice, pd3dCommandList);

	// [0523] 좀비 플레이어가 아니어도 OutLineShader를 가지게 수정
	m_vForwardRenderShader.push_back(make_unique<COutLineShader>(mainPlayerId));
	m_vForwardRenderShader[OUT_LINE_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), 1, nullptr, DXGI_FORMAT_D24_UNORM_S8_UINT);

	//Player 생성 + 아이템
	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		if (i == ZOMBIEPLAYER)
		{
			m_apPlayer[i] = std::make_shared<CZombiePlayer>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), nullptr);
			shared_ptr<CLoadedModelInfo> pZombiePlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), "Asset/Model/Zom_1.bin", MeshType::Standard);
			m_apPlayer[i]->LoadModelAndAnimation(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), pZombiePlayerModel);
			m_vShader[SKINNEDANIMATION_STANDARD_SHADER]->AddGameObject(m_apPlayer[i]);
			
			// [0506] OutLine Shader
			if (mainPlayerId == ZOMBIEPLAYER)
			{
				// ZOMBIE PLAYER일 경우 ZOMBIE 추가
				m_vForwardRenderShader[OUT_LINE_SHADER]->AddGameObject(m_apPlayer[i]);
			}

			continue;
		}
		else
		{
			m_apPlayer[i] = std::make_shared<CBlueSuitPlayer>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), nullptr);
			shared_ptr<CLoadedModelInfo> pBlueSuitPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), "Asset/Model/BlueSuitFree01.bin",MeshType::Standard);
			m_apPlayer[i]->LoadModelAndAnimation(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), pBlueSuitPlayerModel);
			m_vShader[SKINNEDANIMATION_STANDARD_SHADER]->AddGameObject(m_apPlayer[i]);

			//[0505] BLUE SUIT 플레이어의 외곽선을 그리기 위해
			// Zombie 플레이어여야만 필요하다
			if(mainPlayerId == ZOMBIEPLAYER)
			{
				// ZOMBIE PLAYER일 경우 나머지 플레이어 추가
				m_vForwardRenderShader[OUT_LINE_SHADER]->AddGameObject(m_apPlayer[i]);
			}
			else if (mainPlayerId == i)
			{
				// ZOMBIE PLAYER가 아닐 경우 자신을 MainPlayer 설정
				m_vForwardRenderShader[OUT_LINE_SHADER]->AddGameObject(m_apPlayer[i]);
			}
		}

		//플래시라이트모델 로드
		shared_ptr<CTeleportObject> flashLight = make_shared<CTeleportObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pflashLightModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/Flashlight.bin", MeshType::Standard);
		flashLight->ObjectCopy(pd3dDevice, pd3dCommandList, pflashLightModel->m_pModelRootObject);
		//flashLight->LoadModelAndAnimation(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), pflashLightModel);
		m_vShader[STANDARD_SHADER]->AddGameObject(flashLight);

		//레이더모델 로드
		shared_ptr<CRadarObject> pRaderObject = make_shared<CRadarObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pRaderModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/Radar.bin", MeshType::Standard);
		pRaderObject->ObjectCopy(pd3dDevice, pd3dCommandList, pRaderModel->m_pModelRootObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pRaderObject);

		shared_ptr<CTeleportObject> pTeleportObject = make_shared<CTeleportObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pTeleportItemModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/TeleportItem.bin", MeshType::Standard);
		pTeleportObject->ObjectCopy(pd3dDevice, pd3dCommandList, pTeleportItemModel->m_pModelRootObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pTeleportObject);

		shared_ptr<CMineObject> pMineObject = make_shared<CMineObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pMineItemModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/Item_Mine.bin", MeshType::Standard);
		pMineObject->ObjectCopy(pd3dDevice, pd3dCommandList, pMineItemModel->m_pModelRootObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pMineObject);

		shared_ptr<CFuseObject> pFuseObject = make_shared<CFuseObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pFuseModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/fuse_hi-obj.bin", MeshType::Standard);
		pFuseObject->ObjectCopy(pd3dDevice, pd3dCommandList, pFuseModel->m_pModelRootObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pFuseObject);

		auto player = dynamic_pointer_cast<CBlueSuitPlayer>(m_apPlayer[i]);
		if (player) {
			player->SetFlashLight(flashLight);
			player->SetRader(pRaderObject);
			player->SetTeleportItem(pTeleportObject);
			player->SetMineItem(pMineObject);
			player->SetFuseItem(pFuseObject);
		}
	}
	
	/*auto surviveMainPlayer = dynamic_pointer_cast<CBlueSuitPlayer>(m_apPlayer[mainPlayerId]);
	auto zombieMainPlayer = dynamic_pointer_cast<CZombiePlayer>(m_apPlayer[mainPlayerId]);*/
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// 아이템
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	// 아이템 개수를 고정할지는 상의해봐야할듯? 일단 고정으로 간다치고 만듬
	for (int i = 0; i < 10; ++i) // 아이템도 인스턴스 처리를 해야함.또한 공간분할
	{
		shared_ptr<CFuseObject> pFuseObject = make_shared<CFuseObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pFuseModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/fuse_hi-obj.bin", MeshType::Standard);
		pFuseObject->ObjectCopy(pd3dDevice, pd3dCommandList, pFuseModel->m_pModelRootObject);
		
		g_collisionManager.AddCollisionObject(pFuseObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pFuseObject);
	}
	for (int i = 0; i < 10; ++i)
	{
		shared_ptr<CTeleportObject> pTeleportObject = make_shared<CTeleportObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pTeleportModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/TeleportItem.bin", MeshType::Standard);
		pTeleportObject->ObjectCopy(pd3dDevice, pd3dCommandList, pTeleportModel->m_pModelRootObject);
		
		g_collisionManager.AddCollisionObject(pTeleportObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pTeleportObject);
	}

	for (int i = 0; i < 10; ++i)
	{
		//레이더모델 로드
		shared_ptr<CRadarObject> pRaderObject = make_shared<CRadarObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pRaderModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/Radar.bin", MeshType::Standard);
		pRaderObject->ObjectCopy(pd3dDevice, pd3dCommandList, pRaderModel->m_pModelRootObject);

		g_collisionManager.AddCollisionObject(pRaderObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pRaderObject);
	}

	shared_ptr<CLoadedModelInfo> pElectricBlendModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), "Asset/Model/electricBlend.bin", MeshType::Blend);
	shared_ptr<CLoadedModelInfo> pMineModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/Item_Mine.bin", MeshType::Standard);
	for (int i = 0; i < 50; ++i)
	{	//CJI [0422] : 지뢰아이템 이펙트는 동적할당을 줄이기위해서 미리 만들어둔 블렌드 객체를 이용해 렌더링한다.
		shared_ptr<TextureBlendObject> mineExplosionObject = make_shared<TextureBlendObject>(pd3dDevice, pd3dCommandList, pElectricBlendModel->m_pModelRootObject, m_apPlayer[mainPlayerId]);
		m_vTextureBlendObjects.push_back(mineExplosionObject);
		m_vForwardRenderShader[TEXTUREBLEND_SHADER]->AddGameObject(m_vTextureBlendObjects[i]);

		shared_ptr<CMineObject> pMineObject = make_shared<CMineObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		pMineObject->SetExplosionObject(mineExplosionObject);
		pMineObject->ObjectCopy(pd3dDevice, pd3dCommandList, pMineModel->m_pModelRootObject);

		g_collisionManager.AddCollisionObject(pMineObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pMineObject);
		for (int j = 0; j < MAX_CLIENT; ++j) {
			auto player = dynamic_pointer_cast<CBlueSuitPlayer>(m_apPlayer[j]);
			if (player) {
				player->AddEnvironmentMineItems(pMineObject);
			}
		}
	}
	
	// [0504] UserInterface
	m_vForwardRenderShader[USER_INTERFACE_SHADER]->AddGameObject(m_apPlayer[mainPlayerId]);
	m_vForwardRenderShader[USER_INTERFACE_SHADER]->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());

	BuildLights(pd3dDevice, pd3dCommandList); // 쉐이더 생성 이후 수행하도록 한다.
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//[0626] gameFramework에서 이동
	INT ncbElementBytes = ((sizeof(FrameTimeInfo) + 255) & ~255); //256의 배수

	m_pd3dcbTime = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbTime->Map(0, NULL, (void**)&m_pcbMappedTime);
	m_pcbMappedTime->gfScale = 1.0f;
	m_pcbMappedTime->gfBias = 0.05f;
	m_pcbMappedTime->gfIntesity = 3.0f;
	m_d3dTimeCbvGPUDescriptorHandle = CScene::CreateConstantBufferViews(pd3dDevice, 1, m_pd3dcbTime.Get(), ncbElementBytes);

	//[0626] 포스트 프로세싱 셰이더가 Scene으로 오면서 옮김
	m_pPostProcessingShader = new CPostProcessingShader();
	m_pPostProcessingShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), 1, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (::gnRtvDescriptorIncrementSize * m_nSwapChainBuffers);

	DXGI_FORMAT pdxgiResourceFormats[ADD_RENDERTARGET_COUNT] = { DXGI_FORMAT_R8G8B8A8_UNORM,  DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT ,DXGI_FORMAT_R32G32B32A32_FLOAT };
	m_pPostProcessingShader->CreateResourcesAndRtvsSrvs(pd3dDevice, pd3dCommandList, ADD_RENDERTARGET_COUNT, pdxgiResourceFormats, d3dRtvCPUDescriptorHandle); //SRV to (Render Targets) + (Depth Buffer)

	d3dRtvCPUDescriptorHandle.ptr += (::gnRtvDescriptorIncrementSize * ADD_RENDERTARGET_COUNT);
	m_pPostProcessingShader->CreateShadowMapResource(pd3dDevice, pd3dCommandList, m_nLights, d3dRtvCPUDescriptorHandle);

	//[0523] 이제 좀비 플레이어 외에도 사용, COutLineShader 내부에서 m_pPostProcessingShader->GetDsvCPUDesctriptorHandle(0)을 사용하기위해서 필요
	dynamic_cast<COutLineShader*>(m_vForwardRenderShader[OUT_LINE_SHADER].get())->SetPostProcessingShader(m_pPostProcessingShader);


	//[0626] 
}


void CMainScene::AddDefaultObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ObjectType type, XMFLOAT3 position, int shader, int mesh)
{
	shared_ptr<CGameObject> pObject;
	switch (type)
	{
	case ObjectType::DEFAULT:
		pObject = make_shared<CGameObject>(pd3dDevice, pd3dCommandList, 1);
		break;
	case ObjectType::HEXAHERON:
		pObject = make_shared<CHexahedronObject>(pd3dDevice, pd3dCommandList, 1);
		break;
	default:
		break;
	}
	pObject->SetMesh(m_vMesh[mesh]);
	pObject->SetPosition(position);

	m_vShader[shader]->AddGameObject(pObject);
}

void CMainScene::BuildLights(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_nLights = ReadLightObjectInfo(m_xmf3lightPositions, m_xmf3lightLooks) + MAX_SURVIVOR/*플레이어 조명*/;
	if (m_nLights > MAX_LIGHTS) {
		//m_nLights = MAX_LIGHTS;
	}
	m_pLights = new LIGHT[MAX_LIGHTS];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * MAX_LIGHTS);

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	for (int i = 0; i < MAX_SURVIVOR;++i) {
		m_pLightCamera.push_back(make_shared<CLightCamera>());
		m_pLightCamera[i]->m_pLight = make_shared<LIGHT>();

		m_xmf3lightPositions.insert(m_xmf3lightPositions.begin(), XMFLOAT3(0.0f, -100.0f, 0.0f)); // m_xmf3lightPositions을 가지고 카메라를 만들것임
		m_xmf3lightLooks.insert(m_xmf3lightLooks.begin(), XMFLOAT3(0.0f, -1.0f, 0.0f));

		m_pLightCamera[i]->m_pLight->m_bEnable = true;
		m_pLightCamera[i]->m_pLight->m_nType = SPOT_LIGHT;
		m_pLightCamera[i]->m_pLight->m_fRange = 30.0f;
		m_pLightCamera[i]->m_pLight->m_xmf4Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_pLightCamera[i]->m_pLight->m_xmf4Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_pLightCamera[i]->m_pLight->m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_pLightCamera[i]->m_pLight->m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_pLightCamera[i]->m_pLight->m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
		m_pLightCamera[i]->m_pLight->m_xmf3Attenuation = XMFLOAT3(1.0f, -0.1f, 0.01f);
		m_pLightCamera[i]->m_pLight->m_fFalloff = 1.0f;
		m_pLightCamera[i]->m_pLight->m_fPhi = (float)cos(XMConvertToRadians(35.0f));
		m_pLightCamera[i]->m_pLight->m_fTheta = (float)cos(XMConvertToRadians(25.0f));
	}
	m_pLights[0].m_bEnable = true;

	for (int i = MAX_SURVIVOR; i < m_nLights;++i) {
		m_pLightCamera.push_back(make_shared<CLightCamera>());
		m_pLightCamera[i]->m_pLight = make_shared<LIGHT>();

		m_pLightCamera[i]->m_pLight->m_bEnable = true;
		m_pLightCamera[i]->m_pLight->m_nType = SPOT_LIGHT;
		m_pLightCamera[i]->m_pLight->m_fRange = 30.0f;
		m_pLightCamera[i]->m_pLight->m_xmf4Ambient = XMFLOAT4(0.6f, 0.0f, 0.0f, 0.0f);
		m_pLightCamera[i]->m_pLight->m_xmf4Diffuse = XMFLOAT4(0.6f, 0.0f, 0.0f, 0.0f);
		m_pLightCamera[i]->m_pLight->m_xmf4Specular = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
		m_pLightCamera[i]->m_pLight->m_xmf3Position = m_xmf3lightPositions[i];
		m_pLightCamera[i]->m_pLight->m_xmf3Direction = m_xmf3lightLooks[i];
		m_pLightCamera[i]->m_pLight->m_xmf3Attenuation = XMFLOAT3(1.0f, -0.1f, 0.01f);

		m_pLightCamera[i]->m_pLight->m_fFalloff = 1.0f;
		m_pLightCamera[i]->m_pLight->m_fPhi = (float)cos(XMConvertToRadians(45.0f));
		m_pLightCamera[i]->m_pLight->m_fTheta = (float)cos(XMConvertToRadians(35.0f));
	}

	

	vector<XMFLOAT3> positions = GetLightPositions();
	vector<XMFLOAT3> looks = GetLightLooks();

	XMFLOAT3 xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);

	XMFLOAT4X4 xmf4x4ToTexture = {
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f };

	XMMATRIX xmProjectionToTexture = XMLoadFloat4x4(&xmf4x4ToTexture);
	XMMATRIX xmmtxViewProjection;

	for (int i = 0;i < m_nLights;++i) {

		XMFLOAT3 xmf3Up = Vector3::CrossProduct(looks[i], xmf3Right);
		XMFLOAT3 lookAtPosition = Vector3::Add(positions[i], looks[i]);
		m_pLightCamera[i]->GenerateViewMatrix(positions[i], lookAtPosition, xmf3Up);
		if (i >= MAX_SURVIVOR)
		{
			m_pLightCamera[i]->GenerateProjectionMatrix(1.01f, 5.0f, ASPECT_RATIO, 90.0f);	//[0513] 근평면이 있어야  그림자를 그림
		}
		m_pLightCamera[i]->GenerateFrustum();
		m_pLightCamera[i]->MultiplyViewProjection();

		XMFLOAT4X4 viewProjection = m_pLightCamera[i]->GetViewProjection();
		xmmtxViewProjection = XMLoadFloat4x4(&viewProjection);
		XMStoreFloat4x4(&m_pLightCamera[i]->m_pLight->m_xmf4x4ViewProjection, XMMatrixTranspose(xmmtxViewProjection * xmProjectionToTexture));
		m_pLightCamera[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	}

	// 빛의 카메라 파티션 설정
	unique_ptr<PartitionInsStandardShader> PtShader(static_cast<PartitionInsStandardShader*>(m_vPreRenderShader[PARTITION_SHADER].release()));
	auto vBB = PtShader->GetPartitionBB();

	for (int i = 0; i < m_nLights;++i) {
		BoundingBox camerabb;
		camerabb.Center = m_pLightCamera[i]->GetPosition();
		camerabb.Extents = XMFLOAT3(0.1f, 0.1f, 0.1f);
		int curFloor = static_cast<int>(std::floor(camerabb.Center.y / 4.5f));

		m_pLightCamera[i]->SetFloor(curFloor);
		for (int bbIdx = 0; bbIdx < vBB.size();++bbIdx) {
			if (vBB[bbIdx]->Intersects(camerabb)) {
				m_pLightCamera[i]->SetPartition(bbIdx);
				break;
			}
		}
	}

	m_vPreRenderShader[PARTITION_SHADER].reset(PtShader.release());
}

void CMainScene::LoadScene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	ifstream tpobFile("Asset/Data/투명객체.txt");
	// 이름 , 재질개수 , 재질인덱스
	if (!tpobFile) {
		assert(0);
	}
	unordered_map<string, vector<int>> transparentObjects;

	string name;
	while (tpobFile >> name)
	{
		int count{};
		tpobFile >> count;
		for (int i = 0; i < count; ++i) {
			int mtNum;
			tpobFile >> mtNum;
			transparentObjects[name].push_back(mtNum);
		}
	}

	FILE* pSceneFile = NULL;
	::fopen_s(&pSceneFile, (char*)"Asset/Model/Scene.bin", "rb");
	::rewind(pSceneFile);
	int fileEnd{};
	
	unique_ptr<InstanceStandardShader> InsStShader(static_cast<InstanceStandardShader*>(m_vShader[INSTANCE_STANDARD_SHADER].release()));
	int n_curfloor = -1;
	static int count{};
	while (true)
	{
		shared_ptr<CLoadedModelInfo> pLoadedModel = make_shared<CLoadedModelInfo>();

		char pstrToken[128] = { '\0' };
		for (; ; )
		{
			if (::ReadStringFromFile(pSceneFile, pstrToken))
			{
				if (!strcmp(pstrToken, "<Floor>:")) {
					InsStShader->m_vFloorObjects.push_back(vector<shared_ptr<CGameObject>>());
					n_curfloor += 1;
				}
				else if (!strcmp(pstrToken, "<Hierarchy>:"))
				{
					count++;
					pLoadedModel->m_pModelRootObject = CGameObject::LoadInstanceFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), NULL, pSceneFile, &pLoadedModel->m_nSkinnedMeshes);
					::ReadStringFromFile(pSceneFile, pstrToken); //"</Hierarchy>"
					pLoadedModel->m_pModelRootObject->Rotate(0.0f, 0.0f, 0.0f);

					//if (!pLoadedModel->m_pModelRootObject->m_pChild->m_pMesh) continue;

					if (!transparentObjects[pLoadedModel->m_pModelRootObject->m_pstrFrameName].empty()) {
						pLoadedModel->m_pModelRootObject->SetTransparentObjectInfo(transparentObjects[pLoadedModel->m_pModelRootObject->m_pstrFrameName]);
						InsStShader->m_vFloorObjects[n_curfloor].push_back(pLoadedModel->m_pModelRootObject);
						m_vForwardRenderShader[TRANSPARENT_SHADER]->AddGameObject((pLoadedModel->m_pModelRootObject));
						// 첫번째 쉐이더는 불투명한 재질들만 렌더링, 두번째 쉐이더는 투명한 재질들만 렌더링 분류를 위함이고 마지막에 렌더링해야하기 떄문에 두 쉐이더에 모두 포함한다. 
					}
					else
					{
						InsStShader->m_vFloorObjects[n_curfloor].push_back(pLoadedModel->m_pModelRootObject);
					}
				}
				else if (!strcmp(pstrToken, "<Animation>:"))
				{
					CGameObject::LoadAnimationFromFile(pSceneFile, pLoadedModel);
					pLoadedModel->PrepareSkinning();
				}
				else if (!strcmp(pstrToken, "</Animation>:"))
				{
					break;
				}
				else if (!strcmp(pstrToken, "</Scene>:"))
				{
					fileEnd = 1;
					break;
				}
			}
			else
			{
				break;
			}
		}
		if (fileEnd) {
			break;
		}
	}

	for (int i = 0; i < g_collisionManager.GetNumOfCollisionObject();++i) { // 월드변환 행렬을 가지고 있는 객체들.
		auto pObject = g_collisionManager.GetCollisionObjectWithNumber(i).lock();
		if (!transparentObjects[pObject->m_pstrFrameName].empty()) {
			pObject->SetTransparentObjectInfo(transparentObjects[pObject->m_pstrFrameName]);
		}
	}

	


	// 파티션 분할한 씬 로드
	FILE* pPartitionFile = NULL;
	::fopen_s(&pPartitionFile, (char*)"Asset/Model/PartisionScene.bin", "rb");
	::rewind(pPartitionFile);
	fileEnd = 0;

	unique_ptr<PartitionInsStandardShader> PtShader(static_cast<PartitionInsStandardShader*>(m_vPreRenderShader[PARTITION_SHADER].release()));
	int nPartition = -1;
	g_InstanceMeshNotAddCollision = true; // 이 오브젝트들은 Collision 체크를 할 필요 없는 객체들이다.(단순히 쉐도우맵을 만드는곳에만 쓰는 객체들)
	while (true)
	{
		shared_ptr<CLoadedModelInfo> pLoadedModel = make_shared<CLoadedModelInfo>();

		char pstrToken[128] = { '\0' };

		for (; ; )
		{
			if (::ReadStringFromFile(pPartitionFile, pstrToken))
			{
				if (!strcmp(pstrToken, "<Partition>:")) {
					PtShader->AddPartition(); // 파티션 추가
					nPartition++;
				}
				else if (!strcmp(pstrToken, "<Bound>:")) {
					shared_ptr<BoundingBox> bb = make_shared<BoundingBox>();
					int nIndex = 0;
					XMFLOAT3 xmf3bbCenter, xmf3bbExtents;
					fread(&nIndex, sizeof(int), 1, pPartitionFile);
					(UINT)::fread(&xmf3bbCenter, sizeof(XMFLOAT3), 1, pPartitionFile);
					(UINT)::fread(&xmf3bbExtents, sizeof(XMFLOAT3), 1, pPartitionFile);

					bb->Center = xmf3bbCenter;
					bb->Extents = xmf3bbExtents;

					PtShader->AddPartitionBB(bb);
				} // 바운딩 박스만 읽고 객체는 콜리전컨테이너에서 꺼내서 사용
				else if (!strcmp(pstrToken, "<Hierarchy>:"))
				{
					pLoadedModel->m_pModelRootObject = CGameObject::LoadInstanceFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), NULL, pPartitionFile, &pLoadedModel->m_nSkinnedMeshes);
					::ReadStringFromFile(pPartitionFile, pstrToken); //"</Hierarchy>"
					pLoadedModel->m_pModelRootObject->Rotate(0.0f, 0.0f, 0.0f);
					if (!strcmp(pLoadedModel->m_pModelRootObject->m_pstrFrameName, "Zom_1"))
					{
					}
					else if (!transparentObjects[pLoadedModel->m_pModelRootObject->m_pstrFrameName].empty())
					{
						//pLoadedModel->m_pModelRootObject->SetTransparentObjectInfo(transparentObjects[pLoadedModel->m_pModelRootObject->m_pstrFrameName]);
						//PtShader->AddPartitionGameObject((pLoadedModel->m_pModelRootObject), nPartition);
						// 첫번째 쉐이더는 불투명한 재질들만 렌더링, 두번째 쉐이더는 투명한 재질들만 렌더링 분류를 위함이고 마지막에 렌더링해야하기 떄문에 두 쉐이더에 모두 포함한다. 

					}
					else
					{
						//PtShader->AddPartitionGameObject((pLoadedModel->m_pModelRootObject), nPartition);

					}
				}
				else if (!strcmp(pstrToken, "<Animation>:"))
				{
					CGameObject::LoadAnimationFromFile(pPartitionFile, pLoadedModel);
					pLoadedModel->PrepareSkinning();
				}
				else if (!strcmp(pstrToken, "</Animation>:"))
				{
					break;
				}
				else if (!strcmp(pstrToken, "</Scene>:"))
				{
					fileEnd = 1;
					break;
				}
			}
			else
			{
				break;
			}
		}
		if (fileEnd) {
			break;
		}
	}

	//0528 CJI - 객체를 다시 만들지 말고 원래 있던 객체를 사용하자.
	//1. 파티션에 해당하는 오브젝트를 파티션별로 넣는다.
	//2. 똑같은 메쉬를 가지는 오브젝트들은 인스턴스화 시킨다.
	for (int i = 0; i < g_collisionManager.GetNumOfCollisionObject();++i) { // 월드변환 행렬을 가지고 있는 객체들.
		auto pObject = g_collisionManager.GetCollisionObjectWithNumber(i).lock();
		PartisionShaderCollision(PtShader, pObject);
	}
	auto nonCollisionObjects = g_collisionManager.GetNonCollisionObjects();
	for (int i = 0; i < nonCollisionObjects.size();++i) { // 월드변환 행렬을 가지고 있는 객체들.
		auto pObject = nonCollisionObjects[i].lock();	
		PartisionShaderCollision(PtShader, pObject);
	}

	map<string, shared_ptr<CInstanceObject>> mStr_GameObejcts;
	auto& partition = PtShader->GetPartitionObjects();
	for (auto& objects : partition) { // 한 파티션씩 오브젝트를 인스턴싱화.
		mStr_GameObejcts.clear();
		for (auto& ob : objects) {
			if (!mStr_GameObejcts[ob->m_pstrFrameName]) {
				mStr_GameObejcts[ob->m_pstrFrameName] = make_shared<CInstanceObject>(pd3dDevice, pd3dCommandList);
			}
			mStr_GameObejcts[ob->m_pstrFrameName]->m_vInstanceObjectInfo.push_back(ob); 
			//인스턴싱된 오브젝트의 메쉬의 이름을 복사했기때문에 str 정보는 mesh의 이름을 의미함. 
		}
		if (mStr_GameObejcts.size() == 0) continue;

		auto strVector = mStr_GameObejcts.begin();// 메쉬의 이름을 이용해 오브젝트를 찾는다.
		while (strVector != mStr_GameObejcts.end())
		{
			shared_ptr<CInstanceStandardMesh> pMesh = make_shared<CInstanceStandardMesh>(pd3dDevice, pd3dCommandList);
			strncpy(pMesh->m_pstrMeshName, strVector->first.c_str(), strVector->first.size());
			CStandardMesh::SaveStandardMesh(dynamic_pointer_cast<CStandardMesh>(pMesh));

			pMesh->m_nCntInstance = strVector->second->m_vInstanceObjectInfo.size();
			XMFLOAT4X4* InsTrans = new XMFLOAT4X4[pMesh->m_nCntInstance];
			for (int i = 0; i < strVector->second->m_vInstanceObjectInfo.size();++i) {
				InsTrans[i] = Matrix4x4::Transpose(strVector->second->m_vInstanceObjectInfo[i]->m_xmf4x4World); // 버퍼로 복사할 행렬은 전치행렬로 보내야함.
			}
			pMesh->SetInstanceTransformMatrix(InsTrans);
			pMesh->GetInstanceTransformMatrixBuffer() = ::CreateBufferResource(pd3dDevice, pd3dCommandList, InsTrans,
				sizeof(XMFLOAT4X4) * pMesh->m_nCntInstance, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
			D3D12_VERTEX_BUFFER_VIEW bufferView;
			bufferView.BufferLocation = pMesh->GetInstanceTransformMatrixBuffer()->GetGPUVirtualAddress();
			bufferView.StrideInBytes = sizeof(XMFLOAT4X4);
			bufferView.SizeInBytes = sizeof(XMFLOAT4X4) * pMesh->m_nCntInstance;
			pMesh->SetInstanceMatrixBufferView(bufferView);

			for (auto& floor : InsStShader->m_vFloorObjects) {
				bool bFind = false;
				for (auto& f_ob : floor) { // f_ob는 원본 오브젝트이므로 Ins -> child.mesh메쉬에 이름으로 비교해야함
					if (!f_ob->m_pChild || !f_ob->m_pChild->m_pMesh) continue;
					if (!strcmp(f_ob->m_pChild->m_pMesh->m_pstrMeshName, strVector->first.c_str())) {
						mStr_GameObejcts[strVector->first]->InstanceObjectCopy(pd3dDevice, pd3dCommandList, f_ob);
						bFind = true;
						break;
					}
				}
				if (bFind) break;
			}

			pMesh->SetOriginInstanceObject(dynamic_pointer_cast<CInstanceObject>(mStr_GameObejcts[strVector->first]));
			mStr_GameObejcts[strVector->first]->m_pChild->SetMesh(pMesh);// 루트 객체는 인스턴스 부모객체를 의미 하므로 child에게 메쉬를 부여
			if (!transparentObjects[mStr_GameObejcts[strVector->first]->m_pstrFrameName].empty())
			{
				mStr_GameObejcts[strVector->first]->SetTransparentObjectInfo(transparentObjects[mStr_GameObejcts[strVector->first]->m_pstrFrameName]);
			}
			objects.erase(
				std::remove_if(objects.begin(), objects.end(), [strVector](const shared_ptr<CGameObject>& obj) {
					return obj->m_pstrFrameName == strVector->first;
					}),
				objects.end()
			);

			++strVector;
		}
		objects.clear();
		for (auto& [meshName,insObject] : mStr_GameObejcts) {
			objects.push_back(insObject);
		}
	}
	 
	// 메모리 누수를 방지하기 위해 다시 변환
	m_vShader[INSTANCE_STANDARD_SHADER].reset(InsStShader.release());
	m_vPreRenderShader[PARTITION_SHADER].reset(PtShader.release());
}

void PartisionShaderCollision(unique_ptr<PartitionInsStandardShader>& PtShader, shared_ptr<CGameObject>& pObject)
{
	if (!strcmp(pObject->m_pstrFrameName, "Wall_BoundingBox") /*|| Ins_Bounding_Stair_Start*/) {
		return;
	}

	//객체와 충돌체크를 하여 현재 파티션에 바운딩박스에 있으면 넣음
	for (auto& srcobb : pObject->GetVectorOOBB()) {
		BoundingOrientedBox dstobb;
		srcobb.Transform(dstobb, XMLoadFloat4x4(&pObject->m_xmf4x4World));
		XMStoreFloat4(&dstobb.Orientation, XMQuaternionNormalize(XMLoadFloat4(&dstobb.Orientation)));
		int nPt = 0;
		bool inPart = false;
		for (auto& ptobb : PtShader->GetPartitionBB()) {
			if (dstobb.Intersects(*ptobb)) {
				PtShader->AddPartitionGameObject(pObject, nPt);
				inPart = true;
			}
			nPt += 1;
			if (nPt >= PtShader->GetPartitionBB().size()) break;
		}

		if (inPart ) {
			break;
		}
	}
}

bool StreamReadString(ifstream& in, string& str)
{
	// 문자열의 길이 읽기
	char strLength;
	in.read(reinterpret_cast<char*>(&strLength), sizeof(char));

	// 파일 끝에 도달하면 종료
	if (in.eof()) return false;

	// 문자열 읽기
	char* buffer = new char[strLength + 1]; // 문자열 끝에 NULL 문자('\0')를 추가하기 위해 +1
	in.read(buffer, strLength);
	buffer[strLength] = '\0'; // NULL 문자 추가
	str = buffer;
	delete[] buffer;

	//cout << str << endl;
	return true;
}

template<class T>
void StreamReadVariable(ifstream& in, T& data)
{
	in.read(reinterpret_cast<char*>(&data), sizeof(T));
}

int ReadLightObjectInfo(vector<XMFLOAT3>& positions,vector<XMFLOAT3>& looks)
{
	ifstream in("Asset/Data/LightObject.bin", ios::binary);
	if (!in.is_open()) {
		assert(0);
	}
	// 문자열을 저장할 변수
	string str;
	XMFLOAT3 xmfloat3;
	int objCount{};

	// 파일로부터 문자열 읽기
	while (true) {
		if (!StreamReadString(in, str)) {
			break;
		}
		if (str == "<TotalObject>: ") {
			StreamReadVariable(in, objCount);
		}
		else if ("<GameObjects>:" == str) {
			if (!StreamReadString(in, str)) break;

			if ("<Position>:" == str) {
				StreamReadVariable(in, xmfloat3);
				positions.push_back(xmfloat3);
			}

			if (!StreamReadString(in, str)) break;
			if ("<Look>:" == str) {
				StreamReadVariable(in, xmfloat3);
				looks.push_back(xmfloat3);
			}
		}
	}
	return objCount;
}

void CMainScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);
	m_pcbMappedLights->bias = 0.0011f;
	m_d3dLightCbvGPUDescriptorHandle = CreateConstantBufferViews(pd3dDevice, 1, m_pd3dcbLights.Get(), ncbElementBytes);
}

void CMainScene::PrevRenderTask(ID3D12GraphicsCommandList* pd3dCommandList)
{
	// 그림자맵에 해당하는 텍스처를 렌더타겟으로 변환
	m_pPostProcessingShader->OnShadowPrepareRenderTarget(pd3dCommandList);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pPostProcessingShader->GetDsvCPUDesctriptorHandle(0);

	for (int i = 0; i < m_pPostProcessingShader->GetShadowTexture()->GetTextures(); ++i) {
		D3D12_CPU_DESCRIPTOR_HANDLE shadowRTVDescriptorHandle = m_pPostProcessingShader->GetShadowRtvCPUDescriptorHandle(i);
		{
			pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

			pd3dCommandList->OMSetRenderTargets(1, &shadowRTVDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

			if (i < MAX_SURVIVOR) {
				Render(pd3dCommandList, m_pLightCamera[i], 1/*nPipelinestate*/); // 카메라만 빛의 위치대로 설정해서 렌더링함.
			}
			else {
				ShadowPreRender(pd3dCommandList, m_pLightCamera[i], 1/*nPipelinestate*/);
			}
		}
	}

	// 그림자맵에 해당하는 렌더타겟을 텍스처로 변환
	m_pPostProcessingShader->TransitionShadowMapRenderTargetToCommon(pd3dCommandList);
}

void CMainScene::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (int i = 0; i < m_nLights; ++i) {
		memcpy(&m_pLights[i], m_pLightCamera[i]->m_pLight.get(), sizeof(LIGHT));
	}

	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT)* m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));

	pd3dCommandList->SetGraphicsRootDescriptorTable(2, m_d3dLightCbvGPUDescriptorHandle); //Lights
}

void CMainScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
	}
}

void CMainScene::ReleaseUploadBuffers()
{
	for (auto& m : m_vMesh) 
	{
		m->ReleaseUploadBuffers();
	}

	for (auto& s : m_vShader) 
	{
		s->ReleaseUploadBuffers();
	}
}

void CMainScene::ReleaseObjects()
{

}

bool CMainScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_RBUTTONDOWN:
		m_pMainPlayer->SetRightClick(true);
		m_pMainPlayer->RightClickProcess();
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}

	return true;
}

bool CMainScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	//m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.1f, 0.001f); //손전등  밝기 감도설정
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		//case 'P':
			//	m_pcbMappedTime->gfScale += 0.1f;
			//	break;
			//case 'O':
			//	m_pcbMappedTime->gfScale -= 0.1f;
			//	break;
			//case 'L':
			//	m_pcbMappedTime->gfIntesity += 0.1f;
			//	break;
			//case 'K':
			//	m_pcbMappedTime->gfIntesity -= 0.1f;
			//	break;
		case 'M':
			if (m_pPostProcessingShader->GetPipelineIndex() == 0)
				m_pPostProcessingShader->SetPipelineIndex(1);
			else
				m_pPostProcessingShader->SetPipelineIndex(0);
			break;
		case VK_UP://		m_pcbMappedLights->bias	0.00119999994	float
			//m_pcbMappedLights->bias += 0.0001f;
			testAngle += 1.f;
			break;
		case VK_DOWN:
			testAngle -= 1.f;
			//m_pcbMappedLights->bias -= 0.0001f;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return false;
}

bool CMainScene::ProcessInput(UCHAR* pKeysBuffer)
{
	return(false);
}

void CMainScene::AnimateObjects(float fElapsedTime)
{
	m_fElapsedTime = fElapsedTime;
	m_pcbMappedTime->time += fElapsedTime;

	for (auto& shader : m_vShader)
	{
		shader->AnimateObjects(fElapsedTime);
	}

	for (auto& shader : m_vForwardRenderShader)
	{
		shader->AnimateObjects(fElapsedTime);
	}

	int light_Id{};
	for (int i = 0;i < MAX_CLIENT;++i) {
		if (m_apPlayer[i]->GetClientId() == -1) {
			continue;
		}
		auto player = dynamic_pointer_cast<CBlueSuitPlayer>(m_apPlayer[i]);
		if (player) {
			XMFLOAT4X4* xmf4x4playerLight = player->GetFlashLigthWorldTransform();
			m_pLightCamera[i - 1]->m_pLight->m_xmf3Position = XMFLOAT3(xmf4x4playerLight->_41, xmf4x4playerLight->_42, xmf4x4playerLight->_43);//m_pPlayer->GetCamera()->GetPosition();
			m_pLightCamera[i - 1]->m_pLight->m_xmf3Direction = XMFLOAT3(xmf4x4playerLight->_21, xmf4x4playerLight->_22, xmf4x4playerLight->_23);/*XMFLOAT3(xmf4x4playerLight._31, xmf4x4playerLight._32, xmf4x4playerLight._33);*/ //m_pPlayer->GetCamera()->GetLookVector();

			light_Id++;
		}
	}

	XMFLOAT3 clientCameraPos = m_pMainPlayer->GetCamera()->GetPosition();
	sort(m_pLightCamera.begin() + 4, m_pLightCamera.end(), [clientCameraPos](const shared_ptr<CLightCamera>& A, const shared_ptr<CLightCamera>& B) {
		//const float epsilon = 1e-5f; // 허용 오차
		XMFLOAT3 clToA = Vector3::Subtract(clientCameraPos, A->GetPosition());
		XMFLOAT3 clToB = Vector3::Subtract(clientCameraPos, B->GetPosition());
		return Vector3::Length(clToA) < Vector3::Length(clToB);
		});

	for (auto& cm : m_pLightCamera) {
		cm->Update(cm->GetLookAtPosition(), fElapsedTime);
		if (auto player = cm->GetPlayer().lock()) {
			if (player->GetClientId() == -1) {
				cm->m_pLight->m_bEnable = false;
			}
		}
	}

	// 플레이어간 사운드 조정
	for (auto& pPlayer : m_apPlayer)
	{
		if (pPlayer->GetClientId() == m_pMainPlayer->GetClientId())
		{
			continue;
		}

		XMFLOAT3 xmf3MainPos = m_pMainPlayer->GetPosition();
		XMFLOAT3 xmf3OtherPos = pPlayer->GetPosition();

		if (abs(xmf3MainPos.y - xmf3OtherPos.y) > 4.0f) // 층이 다르면 안들림
		{
			pPlayer->SetPlayerVolume(0.0f);
			continue;
		}

		float fWeight = (4.0f - abs(xmf3MainPos.y - xmf3OtherPos.y)) / 4.0f;

		xmf3MainPos.y = 0.0f;
		xmf3OtherPos.y = 0.0f;
		float fDistance = Vector3::Distance(xmf3MainPos, xmf3OtherPos);
		if (fDistance > WALK_SOUND_DISTANCE)
		{
			pPlayer->SetPlayerVolume(0.0f);
		}
		else
		{
			float fVolume = ((WALK_SOUND_DISTANCE - fDistance) / WALK_SOUND_DISTANCE) * fWeight ;
			pPlayer->SetPlayerVolume(fVolume);
		}
	}
}


void CMainScene::PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature.Get());
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, m_pd3dCbvSrvDescriptorHeap.GetAddressOf());

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);
}

void CMainScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, int nPipelineState)
{
	PrepareRender(pd3dCommandList, pCamera);

	for (auto& shader : m_vShader)
	{
		shader->Render(pd3dCommandList, pCamera, m_pMainPlayer, nPipelineState);
	}
}

void CMainScene::ShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, int nPipelineState)
{

	// 그림자맵에 해당하는 텍스처를 렌더타겟으로 변환
	m_pPostProcessingShader->OnShadowPrepareRenderTarget(pd3dCommandList, m_nLights); //플레이어의 손전등 1개 -> [0] 번째 요소에 들어있음.

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pPostProcessingShader->GetDsvCPUDesctriptorHandle(0);

	//그림자맵에는 플레이어의 메쉬도 그림.
	for (auto& pl : m_apPlayer) {
		if (pl->GetClientId() == -1) continue;
		pl->SetShadowRender(true);
	}

	int lightId{};
	auto zombie = dynamic_pointer_cast<CZombiePlayer>(m_pMainPlayer);
	if (zombie)
	{
		for (; lightId < MAX_CLIENT - 1; ++lightId)
		{
			//m_pScene->m_pLights[lightId].m_bEnable = false;
			m_pLightCamera[lightId]->m_pLight->m_bEnable = false; // 항상 모든 빛을 꺼버림. 그림자맵을 생성하는 빛만 켤것.
		}
	}
	else {
		for (int i = 0; i < MAX_CLIENT; ++i) {
			if (m_apPlayer[i]->GetClientId() == -1) 
			{
				continue;
			}
			auto survivor = dynamic_pointer_cast<CBlueSuitPlayer>(m_apPlayer[i]);
			if (!survivor) 
			{
				continue;
			}

			//lightId 는 좀비를 제외하므로 생존자를 4명으로 생성했기에 항상 0~3 의 인덱스틑 생존자의 빛 카메라
			XMFLOAT4X4* xmf4x4playerLight = survivor->GetFlashLigthWorldTransform();
			XMFLOAT3 xmf3LightPosition = XMFLOAT3(xmf4x4playerLight->_41, xmf4x4playerLight->_42, xmf4x4playerLight->_43);
			XMFLOAT3 xmf3LightLook = XMFLOAT3(xmf4x4playerLight->_21, xmf4x4playerLight->_22, xmf4x4playerLight->_23);
			xmf3LightLook = Vector3::ScalarProduct(xmf3LightLook, -1.0f, false);
			xmf3LightPosition = Vector3::Add(xmf3LightPosition, xmf3LightLook);
			m_pLightCamera[i - 1]->SetPosition(xmf3LightPosition);
			m_pLightCamera[i - 1]->SetLookVector(XMFLOAT3(xmf4x4playerLight->_21, xmf4x4playerLight->_22, xmf4x4playerLight->_23));
			m_pLightCamera[i - 1]->RegenerateViewMatrix();
			m_pLightCamera[i - 1]->MultiplyViewProjection();

			static XMFLOAT4X4 xmf4x4ToTexture = {
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f
			};
			static XMMATRIX xmProjectionToTexture = XMLoadFloat4x4(&xmf4x4ToTexture);

			XMFLOAT4X4 viewProjection = m_pLightCamera[i - 1]->GetViewProjection();
			XMMATRIX xmmtxViewProjection = XMLoadFloat4x4(&viewProjection);
			//XMStoreFloat4x4(&m_pScene->m_pLights[lightId].m_xmf4x4ViewProjection, XMMatrixTranspose(xmmtxViewProjection * xmProjectionToTexture));
			XMStoreFloat4x4(&m_pLightCamera[i - 1]->m_pLight->m_xmf4x4ViewProjection, XMMatrixTranspose(xmmtxViewProjection * xmProjectionToTexture));
			m_pLightCamera[i - 1]->SetFloor(static_cast<int>(floor(survivor->GetPosition().y / 4.5f)));

			lightId++;
		}
	}

	for (int i = 0; i < m_nLights; ++i)
	{
		m_pLightCamera[i]->m_pLight->m_bEnable = false; // 항상 모든 빛을 꺼버림. 그림자맵을 생성하는 빛만 켤것.
		if (pCamera->GetFloor() != m_pLightCamera[i]->GetFloor()) {
			continue;
		}

		XMFLOAT3 playerToLight = Vector3::Subtract(pCamera->GetPosition(), m_pLightCamera[i]->GetPosition());
		if (Vector3::Length(playerToLight) > 25.0f) { // 안개가 가린 위치의 빛에 대해서는 렌더링 하지 않는다.
			continue;
		}

		if (pCamera)
		{
			if (!pCamera->IsInFrustum(m_pLightCamera[i]->GetBoundingFrustum()))
			{
				continue;
			}
		}

		m_pLightCamera[i]->m_pLight->m_bEnable = true; // 항상 모든 빛을 꺼버림. 그림자맵을 생성하는 빛만 켤것.

		D3D12_CPU_DESCRIPTOR_HANDLE shadowRTVDescriptorHandle = m_pPostProcessingShader->GetShadowRtvCPUDescriptorHandle(i);
		pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

		pd3dCommandList->OMSetRenderTargets(1, &shadowRTVDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

		//1차 렌더링
		if (i < MAX_SURVIVOR)
		{
			int pl_id = m_pLightCamera[i]->GetPlayer().lock()->GetClientId();
			if (pl_id == -1) continue;
			m_apPlayer[pl_id]->SetSelfShadowRender(true);
			Render(pd3dCommandList, m_pLightCamera[i], 1); // 카메라만 빛의 위치대로 설정해서 렌더링함.
			m_apPlayer[pl_id]->SetSelfShadowRender(false);
		}
		else
		{
			ShadowPreRender(pd3dCommandList, m_pLightCamera[i], 1);
		}
	}
	// 그림자맵에 해당하는 렌더타겟을 텍스처로 변환
	m_pPostProcessingShader->TransitionShadowMapRenderTargetToCommon(pd3dCommandList, m_nLights); //플레이어의 손전등 1개 -> [0] 번째 요소에 들어있음.

}

void CMainScene::ShadowPreRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, int nPipelineState)
{
	PrepareRender(pd3dCommandList, pCamera);

	for (auto& shader : m_vPreRenderShader)
	{
		shader->PartitionRender(pd3dCommandList, pCamera, nPipelineState);
	}
	m_vShader[SKINNEDANIMATION_STANDARD_SHADER]->Render(pd3dCommandList, pCamera, m_pMainPlayer, nPipelineState);
}

void CMainScene::FinalRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle, int nGameState)
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pPostProcessingShader->GetDsvCPUDesctriptorHandle(0);

	Render(pd3dCommandList, pCamera, 0);

	m_pPostProcessingShader->TransitionRenderTargetToCommon(pd3dCommandList);

	FLOAT ClearValue[4] = { 1.0f,1.0f,1.0f,1.0f };

	ClearValue[3] = 1.0f;
	pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, ClearValue, 0, NULL);

	//OM 최종타겟으로 재설정
	pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);
	//불투명 객체 최종 렌더링
	pd3dCommandList->SetGraphicsRootDescriptorTable(12, m_d3dTimeCbvGPUDescriptorHandle);
	m_pPostProcessingShader->Render(pd3dCommandList, pCamera, m_pMainPlayer);

	// 투명 객체 렌더링
	if (nGameState == GAME_STATE::IN_GAME)
	{
		for (auto& shader : m_vForwardRenderShader) 
		{
			shader->Render(pd3dCommandList, pCamera, m_pMainPlayer);
		}
	}
	else if (nGameState != GAME_STATE::BLUE_SUIT_WIN || GAME_STATE::ZOMBIE_WIN)
	{
		m_vForwardRenderShader[USER_INTERFACE_SHADER]->Render(pd3dCommandList, pCamera, m_pMainPlayer);
	}

}
