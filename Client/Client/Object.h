#pragma once
#include "Mesh.h"
#include "Camera.h"
#include "Collision.h"

struct MATERIAL
{
	XMFLOAT4 Ambient;
	XMFLOAT4 Albedo;
	XMFLOAT4 Specular; //a = power
	XMFLOAT4 Emissive;
};

struct VS_CB_OBJECT_INFO {
	XMFLOAT4X4 xmf4x4world;
	MATERIAL material;
	UINT gnTexturesMask;
};

class CTexture
{
public:
	CTexture(int nTextureResources, UINT nResourceType, int nSamplers, int nRootParameters);
	virtual ~CTexture();

private:
	//int								m_nReferences = 0;
	//UINT							m_nTextureType;

	int								m_nTextures = 0;
	vector<ComPtr<ID3D12Resource>> m_vpd3dTextures;
	vector<ComPtr<ID3D12Resource>> m_vpd3dTextureUploadBuffers;

	vector<UINT> m_vnResourceTypes;
	vector<DXGI_FORMAT> m_vdxgiBufferFormats;
	vector<int> m_vnBufferElements;

	int m_nRootParameters = 0;
	vector<UINT> m_vnRootParameterIndices;
	vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_vd3dSrvGpuDescriptorHandles;

	int	m_nSamplers = 0;
	vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_vd3dSamplerGpuDescriptorHandles;

	_TCHAR m_sTextureName[128];
public:
	//void AddRef() { m_nReferences++; }
	//void Release() { if (--m_nReferences <= 0) delete this; }

	void SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle);

	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nParameterIndex, int nTextureIndex);
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseShaderVariables();

	//	void LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nIndex);
	void LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nResourceType, UINT nIndex);
	//	void LoadBufferFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, wchar_t *pszFileName, UINT nIndex);
	void LoadBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT ndxgiFormat, UINT nIndex);
	ID3D12Resource* CreateTexture(ID3D12Device* pd3dDevice, UINT nIndex, UINT nResourceType, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue);

	void SetRootParameterIndex(int nIndex, UINT nRootParameterIndex);
	void SetGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle);

	//리소스를 넘겨받고 리소스를 저장함.
	void SetTextures(UINT nResourceType, ID3D12Resource* pResource, int idx);

	int GetRootParameters() { return(m_nRootParameters); }
	int GetTextures() { return(m_nTextures); }
	ID3D12Resource* GetResource(int nIndex) { return(m_vpd3dTextures[nIndex].Get()); }

	//UINT GetTextureType() { return(m_nTextureType); }
	UINT GetTextureType(int nIndex) { return(m_vnResourceTypes[nIndex]); }
	DXGI_FORMAT GetBufferFormat(int nIndex) { return(m_vdxgiBufferFormats[nIndex]); }
	int GetBufferElements(int nIndex) { return(m_vnBufferElements[nIndex]); }

	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(int nIndex);

	void ReleaseUploadBuffers();

	void SetName(_TCHAR* s);
	_TCHAR* GetName();
};

class  CGameObject;

class CMaterial
{
public:
	CMaterial(int nTextures);
	virtual ~CMaterial();

public:
	static vector<shared_ptr<CTexture>> m_vTextureContainer;

	XMFLOAT4 m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 m_xmf4AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	void SetMaterialType(UINT nType) { m_nType |= nType; }
	void SetTexture(const shared_ptr<CTexture>& pTexture, UINT nTexture = 0);

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, VS_CB_OBJECT_INFO* objectInfo);

	virtual void ReleaseUploadBuffers();
	
	void LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nType, UINT nRootParameter, _TCHAR* pwstrTextureName, shared_ptr<CTexture>& pTexture, shared_ptr<CGameObject> pParent, FILE* pInFile);
public:
	UINT							m_nType = 0x00;

	float							m_fGlossiness = 0.0f;
	float							m_fSmoothness = 0.0f;
	float							m_fSpecularHighlight = 0.0f;
	float							m_fMetallic = 0.0f;
	float							m_fGlossyReflection = 0.0f;

	int 							m_nTextures = 0;
	_TCHAR(*m_ppstrTextureNames)[128] = NULL;
	vector<shared_ptr<CTexture>> m_vpTextures; //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


struct CALLBACKKEY
{
	float m_fTime = 0.0f;
	void* m_pCallbackData = NULL;
};

#define _WITH_ANIMATION_INTERPOLATION

class CAnimationCallbackHandler
{
public:
	CAnimationCallbackHandler() { }
	~CAnimationCallbackHandler() { }

public:
	virtual void HandleCallback(void* pCallbackData, float fTrackPosition) { }
};

//#define _WITH_ANIMATION_SRT

class CAnimationSet
{
public:
	CAnimationSet(float fLength, int nFramesPerSecond, int nKeyFrameTransforms, int nSkinningBones, char* pstrName);
	~CAnimationSet();

	XMFLOAT4X4 GetSRT(int nBone, float fPosition);
public:
	char							m_pstrAnimationSetName[64];

	float m_fLength = 0.0f;
	int	m_nFramesPerSecond = 0; //m_fTicksPerSecond

	int	m_nKeyFrames = 0;
	vector<float> m_vfKeyFrameTimes;
	vector<vector<XMFLOAT4X4>> m_vvxmf4x4KeyFrameTransforms;

#ifdef _WITH_ANIMATION_SRT
	int								m_nKeyFrameScales = 0;
	float* m_pfKeyFrameScaleTimes = NULL;
	XMFLOAT3** m_ppxmf3KeyFrameScales = NULL;
	int								m_nKeyFrameRotations = 0;
	float* m_pfKeyFrameRotationTimes = NULL;
	XMFLOAT4** m_ppxmf4KeyFrameRotations = NULL;
	int								m_nKeyFrameTranslations = 0;
	float* m_pfKeyFrameTranslationTimes = NULL;
	XMFLOAT3** m_ppxmf3KeyFrameTranslations = NULL;
#endif
};

class CAnimationSets
{
public:
	CAnimationSets(int nAnimationSets);
	~CAnimationSets();

public:
	int								m_nAnimationSets = 0;
	vector<shared_ptr<CAnimationSet>> m_vpAnimationSets;

	int								m_nBoneFrames = 0;
	vector<shared_ptr<CGameObject>> m_vpBoneFrameCaches; //[m_nBoneFrames]
};

class CAnimationTrack
{
public:
	CAnimationTrack() { }
	~CAnimationTrack();

	void SetAnimationSet(int nAnimationSet) { m_nAnimationSet = nAnimationSet; }

	void SetEnable(bool bEnable) { m_bEnable = bEnable; }
	void SetSpeed(float fSpeed) { m_fSpeed = fSpeed; }
	void SetWeight(float fWeight) { m_fWeight = fWeight; }

	void SetPosition(float fPosition) { m_fPosition = fPosition; }
	float UpdatePosition(float fTrackPosition, float fTrackElapsedTime, float fAnimationLength);

	void SetCallbackKeys(int nCallbackKeys);
	void SetCallbackKey(int nKeyIndex, float fTime, void* pData);
	void SetAnimationCallbackHandler(const shared_ptr<CAnimationCallbackHandler>& pCallbackHandler);
	void HandleCallback();

public:
	BOOL 							m_bEnable = true;
	float 							m_fSpeed = 1.0f;
	float 							m_fPosition = -ANIMATION_CALLBACK_EPSILON;
	float 							m_fWeight = 1.0f;

	int 							m_nAnimationSet = 0;

	int 							m_nType = ANIMATION_TYPE_LOOP; //Once, Loop, PingPong

	int 							m_nCallbackKeys = 0;
	CALLBACKKEY*					m_pCallbackKeys = NULL;

	shared_ptr<CAnimationCallbackHandler> m_pAnimationCallbackHandler;
};

class CLoadedModelInfo
{
public:
	CLoadedModelInfo() {}
	~CLoadedModelInfo();

	shared_ptr<CGameObject> m_pModelRootObject;

	int m_nSkinnedMeshes = 0;
	vector<shared_ptr<CSkinnedMesh>> m_vpSkinnedMeshes; //[SkinnedMeshes], Skinned Mesh Cache

	shared_ptr<CAnimationSets> m_pAnimationSets;
public:
	void PrepareSkinning();
};

class CAnimationTransition
{
public:
	void SetTransitionAnimationTrack(int nNowState, int nNextState)
	{
		m_nNowState = nNowState;
		m_nNextState = nNextState;
	}

	void SetExitTime(float fExitTime) { m_fExitTime = fExitTime; }
	void SetTransitionStartTime(float fTransitionStart) { m_fTransitionStart = fTransitionStart; }
	void SetTransitionDuration(float fTransitionDuration) { m_fTransitionDuration = fTransitionDuration; }
	bool IsTransition(int nNowState, int nNextState)
	{
		if (nNowState == m_nNowState && nNextState == m_nNextState)
		{
			return true;
		}
		return false;
	}

	void SetAnimationTransition(float fExitTime, float fTransitionStart, float fTransitionDuration)
	{
		SetExitTime(fExitTime);
		SetTransitionStartTime(fTransitionStart);
		SetTransitionDuration(fTransitionDuration);
		m_fTransitionTime = 0.0f;
	}

public:
	int m_nNowState;
	int m_nNextState;

	float m_fExitTime = 0.0f;	//상태전이를 시작할수있는 시점(전체 애니메이션 길이에서(0 ~ 1))
	float m_fTransitionStart = 0.0f;

	float m_fTransitionDuration = 0.0f;	//상태전이에 걸리는 시간
	float m_fTransitionTime = 0.0f;	//현재까지 상태전이에 걸린 시간
};

class CAnimationController
{
public:
	CAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, const shared_ptr<CLoadedModelInfo>& pModel);
	~CAnimationController();

	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);

	void SetTrackEnable(int nAnimationTrack, bool bEnable);
	void SetTrackPosition(int nAnimationTrack, float fPosition);
	void SetTrackSpeed(int nAnimationTrack, float fSpeed);
	void SetTrackWeight(int nAnimationTrack, float fWeight);

	void SetCallbackKeys(int nAnimationTrack, int nCallbackKeys);
	void SetCallbackKey(int nAnimationTrack, int nKeyIndex, float fTime, void* pData);
	void SetAnimationCallbackHandler(int nAnimationTrack, const shared_ptr<CAnimationCallbackHandler>& pCallbackHandler);

	void AddBlendWeight(float fBlendWeight);
	void SetBlendWeight(int nBlendIndex, float fBlendWeight);

	virtual void AdvanceTime(float fElapsedTime, CGameObject* pRootGameObject);

	void SetRootMotion(bool bRootMotion) { m_bRootMotion = bRootMotion; }

	virtual void OnRootMotion(CGameObject* pRootGameObject) { }
	virtual void OnAnimationIK(CGameObject* pRootGameObject) { }

	//
	XMFLOAT4X4 GetBoneFrameTransform(int index);
	XMFLOAT3 GetBoneFramePositionVector(int index);
	XMFLOAT3 GetBoneFrameLookVector(int index);
	XMFLOAT3 GetBoneFrameRightVector(int index);
	XMFLOAT3 GetBoneFrameUpVector(int index);
public:
	float 							m_fTime = 0.0f;

	int	m_nAnimationTracks = 0;
	vector<CAnimationTrack> m_vAnimationTracks;

	shared_ptr<CAnimationSets> m_pAnimationSets;

	int m_nSkinnedMeshes = 0;
	vector<shared_ptr<CSkinnedMesh>> m_vpSkinnedMeshes; //[SkinnedMeshes], Skinned Mesh Cache

	vector<ComPtr<ID3D12Resource>> m_vpd3dcbSkinningBoneTransforms; //[SkinnedMeshes]
	vector<shared_ptr<XMFLOAT4X4>> m_vpcbxmf4x4MappedSkinningBoneTransforms; //[SkinnedMeshes]
	
	// 블렌드위한 가중치
	std::vector<float> m_vfBlendWeight;

	int m_nState = 0;	// 상태 개수
	int m_nTransition = 0;	// 상태전이 개수
	vector<CAnimationTransition> m_vAnimationTransitions;
	bool m_bTransition = false;
	int m_nNowState;
	int m_nNextState;

	vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_vd3dCbvSkinningBoneTransformsGPUDescriptorHandle;

	bool m_bRootMotion = false;
	shared_ptr<CGameObject> m_pModelRootObject;
	weak_ptr<CGameObject> m_pRootMotionObject;	//지금 사용안하는 중
	XMFLOAT3 m_xmf3FirstRootMotionPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);

};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 //Rendering을 위한 오브젝트임 결국 CShader에 포함될 운명임,  근데 인스턴싱 오브젝트의 경우 따로 정보를 결국 저장해둬야함
class CGameObject : public std::enable_shared_from_this<CGameObject>
{
public:
	CGameObject(char* pstrFrameName, XMFLOAT4X4& xmf4x4World, CMesh* pMesh);
	CGameObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	CGameObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nMaterials);
	virtual ~CGameObject();
	
	D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHandle();
	void SetDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle);

	virtual void Animate(float fElapsedTime);
	virtual void Collide(float fElapsedTime, const shared_ptr<CGameObject>& pCollidedObject);

	//Rendering
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);
	//불투명한 재질의 메쉬만 렌더링
	virtual void RenderOpaque(ID3D12GraphicsCommandList* pd3dCommandList);
	//투명한 재질의 메쉬만 렌더링
	virtual void RenderTransparent(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);

	virtual void ReleaseShaderVariables();
	virtual void ReleaseUploadBuffers();

	void SetMesh(const shared_ptr<CMesh>& pMesh);
	void SetMaterial(int nMaterial, const shared_ptr<CMaterial>& pMaterial);
	void SetChild(const shared_ptr<CGameObject>& pChild, bool bReferenceUpdate = false);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();
	XMFLOAT3 GetToParentPosition();

	virtual void SetPosition(float x, float y, float z);
	virtual void SetPosition(XMFLOAT3 xmf3Position);
	void SetScale(float x, float y, float z);

	void Move(XMFLOAT3 xmf3Offset);
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4* pxmf4Quaternion);

	weak_ptr<CGameObject> GetParent() { return(m_pParent); }
	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);
	shared_ptr<CGameObject> FindFrame(const char* pstrFrameName);
	shared_ptr<CGameObject> FindFrame(char* pstrFrameName);

	shared_ptr<CTexture> FindReplicatedTexture(_TCHAR* pstrTextureName);

	UINT GetMeshType();

	void LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<CGameObject> pParent, FILE* pInFile);
	static shared_ptr<CGameObject> LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, shared_ptr<CGameObject> pParent, FILE* pInFile, int* pnSkinnedMeshes);
	static shared_ptr<CGameObject> LoadInstanceFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, shared_ptr<CGameObject> pParent, FILE* pInFile, int* pnSkinnedMeshes);
	static void PrintFrameInfo(const shared_ptr<CGameObject>& pGameObject, const shared_ptr<CGameObject>& pParent);
	
	shared_ptr<CSkinnedMesh> FindSkinnedMesh(char* pstrSkinnedMeshName);
	void FindAndSetSkinnedMesh(vector<shared_ptr<CSkinnedMesh>>& vpSkinnedMeshes, int* pnSkinnedMesh);

	void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);
	void SetTrackAnimationPosition(int nAnimationTrack, float fPosition);

	void SetRootMotion(bool bRootMotion) { if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetRootMotion(bRootMotion); }
	
	static void LoadAnimationFromFile(FILE* pInFile, const shared_ptr<CLoadedModelInfo>& pLoadedModel);
	static shared_ptr<CLoadedModelInfo> LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const char* pstrFileName);

	virtual void LoadModelAndAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const shared_ptr<CLoadedModelInfo>& pLoadModelInfo) {};

	char* GetFrameName() { return m_pstrFrameName; };

	virtual void SetOOBB() {};
	virtual void AnimateOOBB();;
	void AddOOBB(const vector<XMFLOAT3>& vxmf3Center, const vector<XMFLOAT3>& vxmf3Extents);
	BoundingOrientedBox GetOOBB(int nIndex) const { return m_voobbOrigin[nIndex]; };
	vector<BoundingOrientedBox> GetVectorOOBB() const { return m_voobbOrigin; };

	void LoadBoundingBox(vector<BoundingOrientedBox>& voobbOrigin);

	// Picking
	bool GetCollision()const { return m_bCollsion; }

	bool CheckPicking(const weak_ptr<CGameObject>& pGameObject, const XMFLOAT3& xmf3PickPosition, const XMFLOAT4X4& mxf4x4ViewMatrix, float& fDistance);
	virtual void UpdatePicking() {};
	virtual void UpdateUsing(const shared_ptr<CGameObject>& pGameObject) {};

	int GetCollisionType() const { return m_nCollisionType; }
	int GetCollisionNum() const { return m_nCollisionNum; }

	void SetTransparentObjectInfo(vector<int> vNumbers);
public:
	//중복된 메쉬를 없애기 위해 최초 메쉬들을 이곳에 저장한다.
	static vector<shared_ptr<CMesh>> m_vMeshContainer;

	char m_pstrFrameName[64];
	shared_ptr<CMesh> m_pMesh;

	int	m_nMaterials = 0;
	vector<shared_ptr<CMaterial>> m_vpMaterials;

	XMFLOAT4X4 m_xmf4x4ToParent;
	XMFLOAT4X4 m_xmf4x4World;

	weak_ptr<CGameObject> m_pParent;
	shared_ptr<CGameObject> m_pChild;
	shared_ptr<CGameObject> m_pSibling;

	ComPtr<ID3D12Resource> m_d3dcbvObject;
	VS_CB_OBJECT_INFO* m_cbMappedObject = NULL;// m_d3dcbvObject가 제거되는 시점에 같이 제거되므로 별도 해제 X
	D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorHandle;

	// 애니메이션 관련
	shared_ptr<CAnimationController> m_pSkinnedAnimationController;

	vector<BoundingOrientedBox> m_voobbOrigin;
	bool m_bCollsion = true;
	int m_nCollisionType = 0; // 0:None, 1:Standard, 2:Picking
	int m_nCollisionNum = -1;

	// 투명 오브젝트 분류
	bool m_bThisContainTransparent = false;
	vector<int> m_vTransparentMaterialNumbers;
};


class CHexahedronObject : public CGameObject
{
public:
	CHexahedronObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nMaterials);
	~CHexahedronObject();
};

int cntCbvModelObject(const shared_ptr<CGameObject>& pGameObject, int nCnt);

/// <CGameObject - CHexahedronObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CInstanceObject>

class CInstanceObject : public CGameObject
{
public:
	CInstanceObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CInstanceObject() {};

	virtual void Animate(float fElapsedTime) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);

public:
	vector<shared_ptr<CGameObject>> m_vInstanceObjectInfo;
};