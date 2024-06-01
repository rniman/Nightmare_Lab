#pragma once

class CGameObject;
class CInstanceObject;

class CMesh
{
//private:
//	int								m_nReferences = 0;
public:
	CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CMesh();

	char m_pstrMeshName[64] = { 0 };

	int m_nCntInstance = 1;

	//void AddRef() { m_nReferences++; }
	//void Release() { if (--m_nReferences <= 0) delete this; }

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

	// interface
	D3D12_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const { return m_d3dPrimitiveTopology; }
	UINT GetSlot() const { return m_nSlot; }
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const { return m_d3dVertexBufferView; }

	int GetNumOfSubMesh() const { return m_nSubMeshes; }
	int GetNumOfSubSetIndices(int nIndex) const { return m_pnSubSetIndices[nIndex]; }
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView(int nIndex) const { return m_vd3dSubSetIndexBufferViews[nIndex]; }
protected:
	UINT m_nType = 0x00;

	//XMFLOAT3						m_xmf3AABBCenter = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//XMFLOAT3						m_xmf3AABBExtents = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//BoundingOrientedBox				m_OOBB{ BoundingOrientedBox() };
	vector<BoundingOrientedBox>	m_vOOBBs;

	D3D12_PRIMITIVE_TOPOLOGY m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT m_nSlot = 0;
	UINT m_nOffset = 0;

	int	m_nVertices = 0;
	XMFLOAT3* m_pxmf3ModelPositions = NULL;

	ComPtr<ID3D12Resource> m_pd3dVertexBuffer;
	ComPtr<ID3D12Resource> m_pd3dVertexUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3dVertexBufferView;

	int	m_nSubMeshes = 0;
	int* m_pnSubSetIndices = NULL;
	UINT** m_ppnSubSetIndices = NULL;

	vector<ComPtr<ID3D12Resource>> m_vpd3dSubSetIndexBuffers;
	vector<ComPtr<ID3D12Resource>> m_vpd3dSubSetIndexUploadBuffers;
	vector<D3D12_INDEX_BUFFER_VIEW> m_vd3dSubSetIndexBufferViews;

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// [0504] UI위한 사각형 메쉬
class CUserInterfaceRectMesh : public CMesh
{
public:
	CUserInterfaceRectMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth, float fMaxU, float fMaxV, float fMinU, float fMinV);
	virtual ~CUserInterfaceRectMesh();

	virtual void ReleaseUploadBuffers();
	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override;;

	void SetVertexData(float fWidth, float fHeight, float fMaxU, float fMaxV, float fMinU, float fMinV);
protected:
	XMFLOAT2* m_pxmf2UV0 = nullptr;

	ComPtr<ID3D12Resource>			m_pd3dUV0Buffer;
	//ComPtr<ID3D12Resource>			m_pd3dUV0UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dUV0BufferView;
};

///////////////////////////////////////////////////////////////////////////
// HexahedronMesh
class HexahedronMesh : public CMesh {
public:
	HexahedronMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,float xSize ,float ySize,float zSize);
	~HexahedronMesh();

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList);
protected:
	ComPtr<ID3D12Resource>					m_pd3dUV0Buffer;
	ComPtr<ID3D12Resource>					m_pd3dUV0UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW				m_d3dUV0BufferView;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CStandardMesh : public CMesh
{
public:
	// 인스턴스 메쉬를 저장할 컨테이너
	static vector<shared_ptr<CStandardMesh>> g_vAllstandardMesh;

	CStandardMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CStandardMesh();

	D3D12_VERTEX_BUFFER_VIEW GetUV0BufferView() const { return m_d3dUV0BufferView; }
	D3D12_VERTEX_BUFFER_VIEW GetUV1BufferView() const { return m_d3dUV1BufferView; }
	D3D12_VERTEX_BUFFER_VIEW GetNormalBufferView() const { return m_d3dNormalBufferView; }
	D3D12_VERTEX_BUFFER_VIEW GetTangentBufferView() const { return m_d3dTangentBufferView; }
	D3D12_VERTEX_BUFFER_VIEW GetBiTangentBufferView() const { return m_d3dBiTangentBufferView; }

protected:
	XMFLOAT4* m_pxmf4Colors = NULL;
	XMFLOAT3* m_pxmf3Normals = NULL;
	XMFLOAT3* m_pxmf3Tangents = NULL;
	XMFLOAT3* m_pxmf3BiTangents = NULL;

	XMFLOAT2* m_pxmf2UVs0 = NULL;
	XMFLOAT2* m_pxmf2UVs1 = NULL;

	ComPtr<ID3D12Resource> m_pd3dUV0Buffer;
	ComPtr<ID3D12Resource> m_pd3dUV0UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3dUV0BufferView;

	ComPtr<ID3D12Resource> m_pd3dUV1Buffer;
	ComPtr<ID3D12Resource> m_pd3dUV1UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3dUV1BufferView;

	ComPtr<ID3D12Resource> m_pd3dNormalBuffer;
	ComPtr<ID3D12Resource> m_pd3dNormalUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3dNormalBufferView;

	ComPtr<ID3D12Resource>m_pd3dTangentBuffer;
	ComPtr<ID3D12Resource> m_pd3dTangentUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3dTangentBufferView;

	ComPtr<ID3D12Resource> m_pd3dBiTangentBuffer;
	ComPtr<ID3D12Resource> m_pd3dBiTangentUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3dBiTangentBufferView;

public:
	bool LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile);

	virtual void ReleaseUploadBuffers();

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) {};

	static void SaveStandardMesh(shared_ptr<CStandardMesh> pMesh);
};

class CInstanceStandardMesh : public CStandardMesh
{
public:
	CInstanceStandardMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CInstanceStandardMesh();

	bool LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile, const shared_ptr<CInstanceObject>& pGameObject);
	void LoadInstanceData(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile);
	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	// 여기서 하드코딩으로 충돌체에 따른 게임 오브젝트를 만드는 작업을 하게 해둠
	void CreateInstanceObjectInfo(char* pstrMeshName, XMFLOAT4X4& xmf4x4WorldMatrix);

	XMFLOAT4X4* GetInstanceTransformMatrix() { return m_pxmf4x4InstanceTransformMatrix; }
	void SetInstanceTransformMatrix(XMFLOAT4X4* transform) {  m_pxmf4x4InstanceTransformMatrix = transform; }
	ComPtr<ID3D12Resource>& GetInstanceTransformMatrixBuffer() { return m_pd3dInstanceTransformMatrixBuffer; }
	D3D12_VERTEX_BUFFER_VIEW GetInstanceTransformMatrixBufferView() const { return m_d3dInstanceTransformMatrixBufferView; }

	void SetInstanceMatrixBufferView(D3D12_VERTEX_BUFFER_VIEW view) { m_d3dInstanceTransformMatrixBufferView = view; }
	void SetOriginInstanceObject(const shared_ptr<CInstanceObject>& pGameObject) { m_pOriginInstance = pGameObject; }
protected:
	//XMFLOAT4X4						m_xmf4x4ToParent;
	XMFLOAT4X4* m_pxmf4x4InstanceTransformMatrix = NULL;
	weak_ptr<CInstanceObject> m_pOriginInstance;

	//int m_nStart = 0;	// CollisionManager 시작 인덱스 (임시)

	ComPtr<ID3D12Resource> m_pd3dInstanceTransformMatrixBuffer;
	ComPtr<ID3D12Resource> m_pd3dInstanceTransformMatrixUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3dInstanceTransformMatrixBufferView;
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
	ComPtr<ID3D12Resource> m_pd3dBoneIndexBuffer;
	ComPtr<ID3D12Resource> m_pd3dBoneIndexUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3dBoneIndexBufferView;

	ComPtr<ID3D12Resource> m_pd3dBoneWeightBuffer;
	ComPtr<ID3D12Resource> m_pd3dBoneWeightUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3dBoneWeightBufferView;

protected:
	int	m_nBonesPerVertex = 4;

	XMINT4* m_pxmn4BoneIndices = NULL;
	XMFLOAT4* m_pxmf4BoneWeights = NULL;

	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dCbvBindPoseBoneOffsetsGPUDescriptorHandle;
public:
	int								m_nSkinningBones = 0;

	char(*m_ppstrSkinningBoneNames)[128]; //[m_nSkinningBones]
	vector<weak_ptr<CGameObject>> m_vpSkinningBoneFrameCaches; //[m_nSkinningBones]

	XMFLOAT4X4* m_pxmf4x4BindPoseBoneOffsets = NULL; //[m_nSkinningBones], Transposed

	ComPtr<ID3D12Resource> m_pd3dcbBindPoseBoneOffsets; //[m_nSkinningBones]
	XMFLOAT4X4* m_pcbxmf4x4MappedBindPoseBoneOffsets = NULL; //[m_nSkinningBones]

	ComPtr<ID3D12Resource> m_pd3dcbSkinningBoneTransforms = NULL; //[m_nSkinningBones], Pointer Only
	XMFLOAT4X4* m_pcbxmf4x4MappedSkinningBoneTransforms = NULL; //[m_nSkinningBones]
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dCbvSkinningBoneTransformsGPUDescriptorHandle;

public:
	void PrepareSkinning(const shared_ptr<CGameObject>& pModelRootObject);
	void LoadSkinInfoFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList);
};