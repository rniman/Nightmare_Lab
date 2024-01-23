#pragma once
#include "Mesh.h"

struct VS_CB_OBJECT_INFO {
	XMFLOAT4X4 xmf4x4world;
};

class  CGameObject
{
public:
	CGameObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	CGameObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,int nMaterials);
	virtual ~CGameObject();

	CGameObject(CGameObject&& other) noexcept;
	CGameObject& operator=(CGameObject&& other) noexcept;

	char							m_pstrFrameName[64];

	CMesh* m_pMesh = NULL;

	int								m_nMaterials = 0;
	//CMaterial** m_ppMaterials = NULL;

	XMFLOAT4X4						m_xmf4x4ToParent;
	XMFLOAT4X4						m_xmf4x4World;

	CGameObject* m_pParent = NULL;
	CGameObject* m_pChild = NULL;
	CGameObject* m_pSibling = NULL;

	ID3D12Resource* m_d3dcbvObject;
	VS_CB_OBJECT_INFO *m_cbMappedObject;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorHandle;

	D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHandle();
	void SetDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle);

	void SetMesh(CMesh* pMesh);
	//void SetShader(CShader* pShader);
	//void SetShader(int nMaterial, CShader* pShader);
	//void SetMaterial(int nMaterial, CMaterial* pMaterial);

	void SetChild(CGameObject* pChild, bool bReferenceUpdate = false);

	virtual void Animate(float fTimeElapsed);

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);

	virtual void ReleaseUploadBuffers();

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	XMFLOAT3 GetToParentPosition();
	void Move(XMFLOAT3 xmf3Offset);

	virtual void SetPosition(float x, float y, float z);
	virtual void SetPosition(XMFLOAT3 xmf3Position);
	void SetScale(float x, float y, float z);

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4* pxmf4Quaternion);

	CGameObject* GetParent() { return(m_pParent); }
	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);
	CGameObject* FindFrame(char* pstrFrameName);

	//CTexture* FindReplicatedTexture(_TCHAR* pstrTextureName);

	//UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0x00); }

	// 애니메이션 관련
	
	//CAnimationController* m_pSkinnedAnimationController = NULL;
	
	//CSkinnedMesh* FindSkinnedMesh(char* pstrSkinnedMeshName);
	//void FindAndSetSkinnedMesh(CSkinnedMesh** ppSkinnedMeshes, int* pnSkinnedMesh);

	//void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);
	//void SetTrackAnimationPosition(int nAnimationTrack, float fPosition);

	//void SetRootMotion(bool bRootMotion) { if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetRootMotion(bRootMotion); }

	//void LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CShader* pShader);

	//static void LoadAnimationFromFile(FILE* pInFile, CLoadedModelInfo* pLoadedModel);
	//static CGameObject* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CGameObject* pParent, FILE* pInFile, CShader* pShader, int* pnSkinnedMeshes);

	//static CLoadedModelInfo* LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader);
	
	//static void PrintFrameInfo(CGameObject* pGameObject, CGameObject* pParent);
};

