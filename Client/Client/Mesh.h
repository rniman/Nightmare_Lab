#pragma once

class CGameObject;
class CInstanceObject;

class CMesh
{
private:
	int								m_nReferences = 0;
public:
	CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CMesh();

	char							m_pstrMeshName[64] = { 0 };

	int m_nCntInstance = 1;

	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	UINT GetType() { return(m_nType); }
	//XMFLOAT3 GetAABBCenter()const { return m_xmf3AABBCenter; }
	//XMFLOAT3 GetAABBExtents()const { return m_xmf3AABBExtents; }
	BoundingOrientedBox GetOOBB(int nIndex) const { return m_vOOBBs[nIndex]; }
	vector<BoundingOrientedBox> GetVectorOOBB() const { return m_vOOBBs; }

	/*virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) { }
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) { }
	virtual void ReleaseShaderVariables() { }*/

	virtual void ReleaseUploadBuffers();

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) {};
	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);

protected:
	UINT							m_nType = 0x00;

	//XMFLOAT3						m_xmf3AABBCenter = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//XMFLOAT3						m_xmf3AABBExtents = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//BoundingOrientedBox				m_OOBB{ BoundingOrientedBox() };
	vector<BoundingOrientedBox>		m_vOOBBs;

	D3D12_PRIMITIVE_TOPOLOGY		m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT							m_nSlot = 0;
	UINT							m_nOffset = 0;

	int								m_nVertices = 0;

	XMFLOAT3* m_pxmf3ModelPositions = NULL;

	ID3D12Resource* m_pd3dVertexBuffer = NULL;
	ID3D12Resource* m_pd3dVertexUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dVertexBufferView;

	int								m_nSubMeshes = 0;
	int* m_pnSubSetIndices = NULL;
	UINT** m_ppnSubSetIndices = NULL;

	ID3D12Resource** m_ppd3dSubSetIndexBuffers = NULL;
	ID3D12Resource** m_ppd3dSubSetIndexUploadBuffers = NULL;
	D3D12_INDEX_BUFFER_VIEW* m_pd3dSubSetIndexBufferViews = NULL;

};

///////////////////////////////////////////////////////////////////////////
// HexahedronMesh
class HexahedronMesh : public CMesh {
public:
	HexahedronMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,float xSize ,float ySize,float zSize);
	~HexahedronMesh();

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList);
protected:
	ComPtr<ID3D12Resource>					m_pd3dUV0Buffer = NULL;
	ComPtr<ID3D12Resource>					m_pd3dUV0UploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW				m_d3dUV0BufferView;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CStandardMesh : public CMesh
{
public:
	CStandardMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CStandardMesh();

protected:
	XMFLOAT4* m_pxmf4Colors = NULL;
	XMFLOAT3* m_pxmf3Normals = NULL;
	XMFLOAT3* m_pxmf3Tangents = NULL;
	XMFLOAT3* m_pxmf3BiTangents = NULL;

	XMFLOAT2* m_pxmf2UVs0 = NULL;
	XMFLOAT2* m_pxmf2UVs1 = NULL;

	ID3D12Resource* m_pd3dUV0Buffer = NULL;
	ID3D12Resource* m_pd3dUV0UploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dUV0BufferView;

	ID3D12Resource* m_pd3dUV1Buffer = NULL;
	ID3D12Resource* m_pd3dUV1UploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dUV1BufferView;

	ID3D12Resource* m_pd3dNormalBuffer = NULL;
	ID3D12Resource* m_pd3dNormalUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dNormalBufferView;

	ID3D12Resource* m_pd3dTangentBuffer = NULL;
	ID3D12Resource* m_pd3dTangentUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTangentBufferView;

	ID3D12Resource* m_pd3dBiTangentBuffer = NULL;
	ID3D12Resource* m_pd3dBiTangentUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dBiTangentBufferView;

public:
	bool LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile);
	//virtual void LoadInstanceData(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile, CGameObject* pGameObject) {}

	virtual void ReleaseUploadBuffers();

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) {};

};

class CInstanceStandardMesh : public CStandardMesh
{
public:
	CInstanceStandardMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CInstanceStandardMesh();

	bool LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile, CInstanceObject* pGameObject);
	void LoadInstanceData(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile, CInstanceObject* pGameObject);
	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	// 여기서 하드코딩으로 충돌체에 따른 게임 오브젝트를 만드는 작업을 하게 해둠
	CGameObject* CreateInstanceObjectInfo(char* pstrMeshName, XMFLOAT4X4& xmf4x4WorldMatrix);

protected:
	//XMFLOAT4X4						m_xmf4x4ToParent;
	XMFLOAT4X4* m_pxmf4x4InstanceTransformMatrix = NULL;
	CInstanceObject* m_pOriginInstance = NULL;

	//int m_nStart = 0;	// CollisionManager 시작 인덱스 (임시)

	ID3D12Resource*					m_pd3dInstanceTransformMatrixBuffer = NULL;
	ID3D12Resource*					m_pd3dInstanceTransformMatrixUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dInstanceTransformMatrixBufferView;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define SKINNED_ANIMATION_BONES		256

class CSkinnedMesh : public CStandardMesh
{
public:
	CSkinnedMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CSkinnedMesh();

protected:
	ID3D12Resource* m_pd3dBoneIndexBuffer = NULL;
	ID3D12Resource* m_pd3dBoneIndexUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dBoneIndexBufferView;

	ID3D12Resource* m_pd3dBoneWeightBuffer = NULL;
	ID3D12Resource* m_pd3dBoneWeightUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dBoneWeightBufferView;

protected:
	int								m_nBonesPerVertex = 4;

	XMINT4* m_pxmn4BoneIndices = NULL;
	XMFLOAT4* m_pxmf4BoneWeights = NULL;

	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dCbvBindPoseBoneOffsetsGPUDescriptorHandle;
public:
	int								m_nSkinningBones = 0;

	char(*m_ppstrSkinningBoneNames)[128]; //[m_nSkinningBones]
	CGameObject** m_ppSkinningBoneFrameCaches = NULL; //[m_nSkinningBones]

	XMFLOAT4X4* m_pxmf4x4BindPoseBoneOffsets = NULL; //[m_nSkinningBones], Transposed

	ID3D12Resource* m_pd3dcbBindPoseBoneOffsets = NULL; //[m_nSkinningBones]
	XMFLOAT4X4* m_pcbxmf4x4MappedBindPoseBoneOffsets = NULL; //[m_nSkinningBones]

	ID3D12Resource* m_pd3dcbSkinningBoneTransforms = NULL; //[m_nSkinningBones], Pointer Only
	XMFLOAT4X4* m_pcbxmf4x4MappedSkinningBoneTransforms = NULL; //[m_nSkinningBones]
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dCbvSkinningBoneTransformsGPUDescriptorHandle;

public:
	void PrepareSkinning(CGameObject* pModelRootObject);
	void LoadSkinInfoFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList);
};