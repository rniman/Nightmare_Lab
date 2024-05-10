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

vector<unique_ptr<CShader>> CScene::m_vShader;
//CShader* CScene::m_pRefShader;

int ReadLightObjectInfo(vector<XMFLOAT3>& positions, vector<XMFLOAT3>& looks);

CScene::CScene()
{

}
CScene::~CScene()
{

}

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
	pd3dDescriptorRanges[11].NumDescriptors = 30;
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

ComPtr<ID3D12RootSignature> CScene::GetGraphicsRootSignature()
{
	return m_pd3dGraphicsRootSignature;
}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int mainPlayerId)
{
	BuildLights();
	CreateGraphicsRootSignature(pd3dDevice);

	// CBV(Default) :  카메라(1), 플레이어(1) , 라이트카메라(2(임시))
	// CBC(Scene Load): 66
	// CBV(RootObject) : //육면체(1), 오브젝트(1), DeskObject(1), DoorObject(1), flashLight(1), 서버인원예상(20), fuse(3)
	// CBV(Model) : Zom(72),  Zom_Controller(2 * N),// BlueSuit(85), BlueSuit_Controller(2 * N), Desk(3), Door(5), flashLight(1), Fuse(6), 레이더(5),텔레포트아이템(1),지뢰(1)
	int nCntCbv = 1 + 1 + 2 + 66 +
		(72 + 2) + (85 + 2) * MAX_CLIENT + 2 + 7 + 10 + 5 * MAX_CLIENT + 1 * MAX_CLIENT + 120 + 1 + 1+500;
	// SRV(Default) : 디퍼드렌더링텍스처(ADD_RENDERTARGET_COUNT로 정의된 개수임)
	// SRV(Scene Load) : 79
	// SRV: Zombie(3), // BlueSuit(6), 육면체(1), 엘런(8(오클루젼맵제거), Desk(3), Door(9), flashLight(3) , m_nLights,지뢰(4),Electiric
	int nCntSrv = ADD_RENDERTARGET_COUNT + 6 * MAX_CLIENT + 79 + 3 + 3 + 3 * MAX_CLIENT + m_nLights + 4 * MAX_CLIENT + 1 + 10*4+10+200;
	
	CreateCbvSrvDescriptorHeaps(pd3dDevice, nCntCbv, nCntSrv);

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
	m_vForwardRenderShader.push_back(make_unique<CUserInterfaceShader>());
	m_vForwardRenderShader[USER_INTERFACE_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), 1, nullptr, DXGI_FORMAT_D24_UNORM_S8_UINT);

	LoadScene(pd3dDevice, pd3dCommandList);

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
				m_vForwardRenderShader.push_back(make_unique<COutLineShader>());
				m_vForwardRenderShader[OUT_LINE_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), 1, nullptr, DXGI_FORMAT_D24_UNORM_S8_UINT);

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
				m_vForwardRenderShader[OUT_LINE_SHADER]->AddGameObject(m_apPlayer[i]);
			}
		}

		//플래시라이트모델 로드
		shared_ptr<CTeleportObject> flashLight = make_shared<CTeleportObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pflashLightModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/Flashlight.bin", MeshType::Standard);
		flashLight->ObjectCopy(pd3dDevice, pd3dCommandList, flashLight, pflashLightModel->m_pModelRootObject);
		//flashLight->LoadModelAndAnimation(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), pflashLightModel);
		m_vShader[STANDARD_SHADER]->AddGameObject(flashLight);

		//레이더모델 로드
		shared_ptr<CRadarObject> pRaderObject = make_shared<CRadarObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pRaderModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/Rader.bin", MeshType::Standard);
		pRaderObject->ObjectCopy(pd3dDevice, pd3dCommandList, pRaderObject, pRaderModel->m_pModelRootObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pRaderObject);

		shared_ptr<CTeleportObject> pTeleportObject = make_shared<CTeleportObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pTeleportItemModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/TeleportItem.bin", MeshType::Standard);
		pTeleportObject->ObjectCopy(pd3dDevice, pd3dCommandList, pTeleportObject, pTeleportItemModel->m_pModelRootObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pTeleportObject);

		shared_ptr<CMineObject> pMineObject = make_shared<CMineObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pMineItemModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/Item_Mine.bin", MeshType::Standard);
		pMineObject->ObjectCopy(pd3dDevice, pd3dCommandList, pMineObject, pMineItemModel->m_pModelRootObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pMineObject);

		shared_ptr<CFuseObject> pFuseObject = make_shared<CFuseObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pFuseModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/fuse_hi-obj.bin", MeshType::Standard);
		pFuseObject->ObjectCopy(pd3dDevice, pd3dCommandList, pFuseObject, pFuseModel->m_pModelRootObject);
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
	
	auto surviveMainPlayer = dynamic_pointer_cast<CBlueSuitPlayer>(m_apPlayer[mainPlayerId]);
	auto zombieMainPlayer = dynamic_pointer_cast<CZombiePlayer>(m_apPlayer[mainPlayerId]);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// 아이템
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// 아이템 개수를 고정할지는 상의해봐야할듯? 일단 고정으로 간다치고 만듬
	for (int i = 0; i < 10; ++i)
	{
		shared_ptr<CFuseObject> pFuseObject = make_shared<CFuseObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pFuseModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/fuse_hi-obj.bin", MeshType::Standard);
		pFuseObject->ObjectCopy(pd3dDevice, pd3dCommandList, pFuseObject, pFuseModel->m_pModelRootObject);
		
		g_collisionManager.AddCollisionObject(pFuseObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pFuseObject);
	}
	for (int i = 0; i < 10; ++i)
	{
		shared_ptr<CTeleportObject> pTeleportObject = make_shared<CTeleportObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pTeleportModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/TeleportItem.bin", MeshType::Standard);
		pTeleportObject->ObjectCopy(pd3dDevice, pd3dCommandList, pTeleportObject, pTeleportModel->m_pModelRootObject);
		
		g_collisionManager.AddCollisionObject(pTeleportObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pTeleportObject);
	}

	for (int i = 0; i < 10; ++i)
	{
		//레이더모델 로드
		shared_ptr<CRadarObject> pRaderObject = make_shared<CRadarObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pRaderModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/Rader.bin", MeshType::Standard);
		pRaderObject->ObjectCopy(pd3dDevice, pd3dCommandList, pRaderObject, pRaderModel->m_pModelRootObject);

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
		pMineObject->ObjectCopy(pd3dDevice, pd3dCommandList, pMineObject, pMineModel->m_pModelRootObject);

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
	m_vForwardRenderShader[USER_INTERFACE_SHADER]->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}


void CScene::AddDefaultObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ObjectType type, XMFLOAT3 position, int shader, int mesh)
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

void CScene::BuildLights()
{
	m_nLights = ReadLightObjectInfo(m_xmf3lightPositions, m_xmf3lightLooks) + MAX_SURVIVOR/*플레이어 조명*/;
	
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	for (int i = MAX_SURVIVOR; i < m_xmf3lightPositions.size() + MAX_SURVIVOR;++i) {
		m_pLights[i].m_bEnable = true;
		m_pLights[i].m_nType = SPOT_LIGHT;
		m_pLights[i].m_fRange = 30.0f;
		m_pLights[i].m_xmf4Ambient = XMFLOAT4(0.6f, 0.0f, 0.0f, 0.0f);
		m_pLights[i].m_xmf4Diffuse = XMFLOAT4(0.6f, 0.0f, 0.0f, 0.0f);
		m_pLights[i].m_xmf4Specular = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
		m_pLights[i].m_xmf3Position = m_xmf3lightPositions[i - MAX_SURVIVOR];
		m_pLights[i].m_xmf3Direction = m_xmf3lightLooks[i - MAX_SURVIVOR];
		m_pLights[i].m_xmf3Attenuation = XMFLOAT3(1.0f, -0.1f, 0.01f);
		m_pLights[i].m_fFalloff = 1.0f;
		m_pLights[i].m_fPhi = (float)cos(XMConvertToRadians(45.0f));
		m_pLights[i].m_fTheta = (float)cos(XMConvertToRadians(35.0f));
	}

	for (int i = 0; i < MAX_SURVIVOR;++i) {
		m_xmf3lightPositions.insert(m_xmf3lightPositions.begin(), XMFLOAT3(0.0f, 3.0f, 0.0f)); // m_xmf3lightPositions을 가지고 카메라를 만들것임
		m_xmf3lightLooks.insert(m_xmf3lightLooks.begin(), XMFLOAT3(0.0f, 0.0f, 1.0f));

		m_pLights[i].m_bEnable = true;
		m_pLights[i].m_nType = SPOT_LIGHT;
		m_pLights[i].m_fRange = 30.0f;
		m_pLights[i].m_xmf4Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_pLights[i].m_xmf4Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_pLights[i].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_pLights[i].m_xmf3Position = XMFLOAT3(0.0f, 3.0f, 0.0f);
		m_pLights[i].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
		m_pLights[i].m_xmf3Attenuation = XMFLOAT3(1.0f, -0.1f, 0.01f);
		m_pLights[i].m_fFalloff = 1.0f;
		m_pLights[i].m_fPhi = (float)cos(XMConvertToRadians(35.0f));
		m_pLights[i].m_fTheta = (float)cos(XMConvertToRadians(25.0f));
	}
	m_pLights[0].m_bEnable = true;
}

void CScene::LoadScene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	ifstream is("Asset/Data/투명객체.txt");
	// 이름 , 재질개수 , 재질인덱스
	if (!is) {
		assert(0);
	}
	unordered_map<string, vector<int>> transparentObjects;

	string name;
	while (is >> name)
	{
		int count{};
		is >> count;
		for (int i = 0; i < count; ++i) {
			int mtNum;
			is >> mtNum;
			transparentObjects[name].push_back(mtNum);
		}
	}

	FILE* pInFile = NULL;
	::fopen_s(&pInFile, (char*)"Asset/Model/Scene.bin", "rb");
	::rewind(pInFile);
	int fileEnd{};
	while (true)
	{
		shared_ptr<CLoadedModelInfo> pLoadedModel = make_shared<CLoadedModelInfo>();

		char pstrToken[128] = { '\0' };

		for (; ; )
		{
			if (::ReadStringFromFile(pInFile, pstrToken))
			{
				if (!strcmp(pstrToken, "<Hierarchy>:"))
				{
					pLoadedModel->m_pModelRootObject = CGameObject::LoadInstanceFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), NULL, pInFile, &pLoadedModel->m_nSkinnedMeshes);
					::ReadStringFromFile(pInFile, pstrToken); //"</Hierarchy>"
					pLoadedModel->m_pModelRootObject->Rotate(0.0f, 0.0f, 0.0f);
					if (!strcmp(pLoadedModel->m_pModelRootObject->m_pstrFrameName, "Zom_1"))
					{ // 씬을 바이너리로 쓸때 스키닝 정보는 넣지 않음(그러므로 이 객체는 정보 x)
						m_vShader[SKINNEDANIMATION_STANDARD_SHADER]->AddGameObject(pLoadedModel->m_pModelRootObject);
					}
					else if (!transparentObjects[pLoadedModel->m_pModelRootObject->m_pstrFrameName].empty()) {
						pLoadedModel->m_pModelRootObject->SetTransparentObjectInfo(transparentObjects[pLoadedModel->m_pModelRootObject->m_pstrFrameName]);
						m_vShader[INSTANCE_STANDARD_SHADER]->AddGameObject((pLoadedModel->m_pModelRootObject));
						m_vForwardRenderShader[TRANSPARENT_SHADER]->AddGameObject((pLoadedModel->m_pModelRootObject));
						// 첫번째 쉐이더는 불투명한 재질들만 렌더링, 두번째 쉐이더는 투명한 재질들만 렌더링 분류를 위함이고 마지막에 렌더링해야하기 떄문에 두 쉐이더에 모두 포함한다. 
					}
					else
					{
						m_vShader[INSTANCE_STANDARD_SHADER]->AddGameObject((pLoadedModel->m_pModelRootObject));
					}
				}
				else if (!strcmp(pstrToken, "<Animation>:"))
				{
					CGameObject::LoadAnimationFromFile(pInFile, pLoadedModel);
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

void CScene::SetPlayer(shared_ptr<CPlayer> pPlayer, int nIndex)
{
	m_apPlayer[nIndex] = pPlayer;
}

void CScene::SetMainPlayer(const shared_ptr<CPlayer>& pMainPlayer)
{
	m_pMainPlayer = pMainPlayer;
}

void CScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);

	m_d3dLightCbvGPUDescriptorHandle = CreateConstantBufferViews(pd3dDevice, 1, m_pd3dcbLights.Get(), ncbElementBytes);
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	int light_Id{};
	for (int i = 0;i < MAX_CLIENT;++i) {
		if (m_apPlayer[i]->GetClientId() == -1) {
			continue;
		}
		auto player = dynamic_pointer_cast<CBlueSuitPlayer>(m_apPlayer[i]);
		if (player) {
			XMFLOAT4X4* xmf4x4playerLight = player->GetFlashLigthWorldTransform();
			m_pLights[light_Id].m_xmf3Position = XMFLOAT3(xmf4x4playerLight->_41, xmf4x4playerLight->_42, xmf4x4playerLight->_43);//m_pPlayer->GetCamera()->GetPosition();
			m_pLights[light_Id].m_xmf3Direction = XMFLOAT3(xmf4x4playerLight->_21, xmf4x4playerLight->_22, xmf4x4playerLight->_23);/*XMFLOAT3(xmf4x4playerLight._31, xmf4x4playerLight._32, xmf4x4playerLight._33);*/ //m_pPlayer->GetCamera()->GetLookVector();

			light_Id++;
		}

	}
	
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT)* m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));

	pd3dCommandList->SetGraphicsRootDescriptorTable(2, m_d3dLightCbvGPUDescriptorHandle); //Lights
}

void CScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
	}
}

void CScene::ReleaseUploadBuffers()
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

int CScene::m_nCntCbv = 0;
int CScene::m_nCntSrv = 0;

void CScene::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dCbvSrvDescriptorHeap);

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

void CScene::ReleaseObjects()
{

}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	//m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.1f, 0.001f); //손전등  밝기 감도설정
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_UP:
			m_pLights[0].m_xmf3Attenuation.y -= 0.1f;
			break;
		case VK_DOWN:
			m_pLights[0].m_xmf3Attenuation.y += 0.1f;
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

bool CScene::ProcessInput(UCHAR* pKeysBuffer)
{
	return(false);
}

void CScene::AnimateObjects(float fElapsedTime)
{
	m_fElapsedTime = fElapsedTime;

	for (auto& shader : m_vShader)
	{
		shader->AnimateObjects(fElapsedTime);
	}

	for (auto& shader : m_vForwardRenderShader)
	{
		shader->AnimateObjects(fElapsedTime);
	}
}


void CScene::PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature.Get());
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, m_pd3dCbvSrvDescriptorHeap.GetAddressOf());

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, int nPipelineState)
{
	PrepareRender(pd3dCommandList, pCamera);

	for (auto& shader : m_vShader) 
	{
		shader->Render(pd3dCommandList, pCamera, nPipelineState);
	}
}