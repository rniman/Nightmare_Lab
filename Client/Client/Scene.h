#pragma once
#include "Timer.h"
#include "Shader.h"
#include "TCPClient.h"
//#include <stdexcept>

// m_vShader 쉐이더에 AddDefaultObject 시에 접근할 각 쉐이더 인덱스를 의미
#define STANDARD_SHADER 0
#define INSTANCE_STANDARD_SHADER 1
#define SKINNEDANIMATION_STANDARD_SHADER 2
#define TRANSPARENT_SHADER 0 // 투명객체에 대한 쉐이더는 항상 후순위로 배치
//#define NOTRENDERING_SHADER 3

// m_vMesh 메쉬에 접근할 각 인덱스를 의미
#define HEXAHEDRONMESH 0

#define MAX_LIGHTS						25

#define POINT_LIGHT						1
#define SPOT_LIGHT						2
#define DIRECTIONAL_LIGHT				3

struct LIGHT
{
	XMFLOAT4X4							m_xmf4x4ViewProjection = Matrix4x4::Identity();
	XMFLOAT4							m_xmf4Ambient;
	XMFLOAT4							m_xmf4Diffuse;
	XMFLOAT4							m_xmf4Specular;
	XMFLOAT3							m_xmf3Position;
	float 								m_fFalloff;
	XMFLOAT3							m_xmf3Direction;
	float 								m_fTheta; //cos(m_fTheta)
	XMFLOAT3							m_xmf3Attenuation;
	float								m_fPhi; //cos(m_fPhi)
	bool								m_bEnable;
	int									m_nType;
	float								m_fRange;
	float								padding;
};

struct LIGHTS
{
	LIGHT								m_pLights[MAX_LIGHTS];
	XMFLOAT4							m_xmf4GlobalAmbient;
	int									m_nLights;
};

class CPlayer;
class CLoadedModelInfo;
class CTeleportObject;

class CScene
{
public:
	CScene();
	~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ComPtr<ID3D12RootSignature> GetGraphicsRootSignature();

	void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void LoadScene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	//오브젝트 소멸 관련
	void ReleaseObjects();
	void ReleaseShaderVariables();
	void ReleaseUploadBuffers();

	//씬 업데이트 관련
	bool ProcessInput(UCHAR* pKeysBuffer);
	void AnimateObjects(float fElapsedTime);

	//렌더링 관련
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera,int nPipelineState);

	void AddDefaultObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ObjectType type, XMFLOAT3 position,int shader, int mesh);
	
	// 빛 관련
	D3D12_GPU_DESCRIPTOR_HANDLE			m_d3dLightCbvGPUDescriptorHandle;
	LIGHT*								m_pLights = nullptr;
	int									m_nLights = 0;
	ComPtr<ID3D12Resource>				m_pd3dcbLights;
	LIGHTS*								m_pcbMappedLights = NULL;
	XMFLOAT4							m_xmf4GlobalAmbient;
	vector<XMFLOAT3>					m_xmf3lightPositions, m_xmf3lightLooks;
	vector<XMFLOAT3>& GetLightPositions() { return m_xmf3lightPositions; }
	vector<XMFLOAT3>& GetLightLooks() { return m_xmf3lightLooks; }
	void BuildLights();

	void SetPlayer(shared_ptr<CPlayer> pPlayer, int nIndex);
	void SetMainPlayer(const shared_ptr<CPlayer>& pMainplayer);

	//씬 내 오브젝트(쉐이더)
	static vector<unique_ptr<CShader>> m_vShader;
	vector<unique_ptr<CShader>> m_vForwardRenderShader;
	
	std::array<shared_ptr<CPlayer>, MAX_CLIENT> m_apPlayer;
	std::shared_ptr<CPlayer> m_pMainPlayer;
	//메쉬 저장
	vector<shared_ptr<CMesh>>			m_vMesh;

	float m_fElapsedTime = 0.0f;
protected:
	ComPtr<ID3D12RootSignature>			m_pd3dGraphicsRootSignature;
	//루트 시그너처를 나타내는 인터페이스 포인터

	static ComPtr<ID3D12DescriptorHeap> m_pd3dCbvSrvDescriptorHeap;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorNextHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorNextHandle;

public:
	// Cbv Count
	static int m_nCntCbv;
	static int m_nCntSrv;

	static void CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews);

	static D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride);
	static void CreateShaderResourceViews(ID3D12Device* pd3dDevice, const shared_ptr<CTexture>& pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceView(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dResource, DXGI_FORMAT dxgiSrvFormat);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorNextHandle() { return(m_d3dCbvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorNextHandle() { return(m_d3dCbvGPUDescriptorNextHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorNextHandle() { return(m_d3dSrvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorNextHandle() { return(m_d3dSrvGPUDescriptorNextHandle); }
};

