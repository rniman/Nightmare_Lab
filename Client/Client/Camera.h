#pragma once

#define ASPECT_RATIO				(float(FRAME_BUFFER_WIDTH) / float(FRAME_BUFFER_HEIGHT))

#define FIRST_PERSON_CAMERA			0x01
#define THIRD_PERSON_CAMERA			0x02

struct VS_CB_CAMERA_INFO
{
	XMFLOAT4X4		m_xmf4x4View;
	XMFLOAT4X4		m_xmf4x4Projection;
	XMFLOAT4X4		m_xmf4x4InverseViewProjection;
	XMFLOAT4		m_xmf4Position;
	XMFLOAT4		m_xmf4FogColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4		m_xmf4FogInfo = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f); //START, RANGE, Density, MOD
};

class CPlayer;

class CCamera
{
public:
	CCamera();
	CCamera(const shared_ptr<CCamera>& pCamera);
	virtual ~CCamera();

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	void GenerateViewMatrix();
	void GenerateViewMatrix(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3LookAt, XMFLOAT3 xmf3Up);
	void RegenerateViewMatrix();

	void MultiplyViewProjection();
	XMFLOAT4X4 GetViewProjection() const { return m_xmf4x4ViewProjection; }

	void GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle);

	void SetViewport(int xTopLeft, int yTopLeft, int nWidth, int nHeight, float fMinZ = 0.0f, float fMaxZ = 1.0f);
	void SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom);

	virtual void SetViewportsAndScissorRects(ID3D12GraphicsCommandList* pd3dCommandList);

	void SetPlayer(const shared_ptr<CPlayer>& pPlayer) { m_pPlayer = pPlayer; }
	weak_ptr<CPlayer> GetPlayer() { return(m_pPlayer); }

	void SetMode(DWORD nMode) { m_nMode = nMode; }
	DWORD GetMode() { return(m_nMode); }

	void SetPosition(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	XMFLOAT3& GetPosition() { return(m_xmf3Position); }
	//XMFLOAT3 GetPosition() const { return(m_xmf3Position); }

	void SetLookVector(XMFLOAT3 look) { m_xmf3Look = look; }

	void SetLookAtPosition(XMFLOAT3 xmf3LookAtWorld) { m_xmf3LookAtWorld = xmf3LookAtWorld; }
	XMFLOAT3& GetLookAtPosition() { return(m_xmf3LookAtWorld); }

	XMFLOAT3& GetRightVector() { return(m_xmf3Right); }
	XMFLOAT3& GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3& GetLookVector() { return(m_xmf3Look); }

	float& GetPitch() { return(m_fPitch); }
	float& GetRoll() { return(m_fRoll); }
	float& GetYaw() { return(m_fYaw); }

	void SetOffset(XMFLOAT3 xmf3Offset) { m_xmf3Offset = xmf3Offset; }
	XMFLOAT3& GetOffset() { return(m_xmf3Offset); }

	void SetTimeLag(float fTimeLag) { m_fTimeLag = fTimeLag; }
	float GetTimeLag() { return(m_fTimeLag); }

	XMFLOAT4X4 GetViewMatrix() { return(m_xmf4x4View); }
	XMFLOAT4X4 GetProjectionMatrix() { return(m_xmf4x4Projection); }
	D3D12_VIEWPORT GetViewport() { return(m_d3dViewport); }
	D3D12_RECT GetScissorRect() { return(m_d3dScissorRect); }

	virtual void Move(const XMFLOAT3& xmf3Shift) { m_xmf3Position.x += xmf3Shift.x; m_xmf3Position.y += xmf3Shift.y; m_xmf3Position.z += xmf3Shift.z; }
	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f) { }
	virtual void Update(XMFLOAT3& xmf3LookAt, float fElapsedTime);
	virtual void SetLookAt(XMFLOAT3& xmf3LookAt) { }

	D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHandle();
	void SetDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle);

	void SetFogColor(const XMFLOAT4& xmf4FogColor) { m_xmf4FogColor = xmf4FogColor; }
	void SetFogInfo(const XMFLOAT4& xmf4FogInfo) { m_xmf4FogInfo = xmf4FogInfo; }

	void SetPartition(int p) { m_iPartitionPos = p; }
	int GetPartitionPos() { return m_iPartitionPos; }

	void SetFloor(int val) { m_Floor = val; }
	int GetFloor() { return m_Floor; }

	void SetUpdateUseRotate(bool val) { m_bUpdateUseRotate = val; }
protected:
	XMFLOAT3						m_xmf3Position;
	XMFLOAT3						m_xmf3Right;
	XMFLOAT3						m_xmf3Up;
	XMFLOAT3						m_xmf3Look;

	float           				m_fPitch;
	float           				m_fRoll;
	float           				m_fYaw;

	DWORD							m_nMode;

	XMFLOAT3						m_xmf3LookAtWorld;
	XMFLOAT3						m_xmf3Offset;
	float           				m_fTimeLag;

	XMFLOAT4X4						m_xmf4x4View;
	XMFLOAT4X4						m_xmf4x4Projection;

	XMFLOAT4X4						m_xmf4x4ViewProjection;

	D3D12_VIEWPORT					m_d3dViewport;
	D3D12_RECT						m_d3dScissorRect;

	ComPtr<ID3D12Resource> m_pd3dcbCamera;
	VS_CB_CAMERA_INFO* m_pcbMappedCamera = NULL;

	D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorHandle;

	weak_ptr<CPlayer> m_pPlayer;

	int m_iPartitionPos;
	int m_Floor;

	//Fog
	XMFLOAT4 m_xmf4FogColor;
	XMFLOAT4 m_xmf4FogInfo; //START, RANGE, Density, MOD

	//[0513] Frustum
	BoundingFrustum m_xmFrustum;

	bool m_bUpdateUseRotate = true;
public:
	void GenerateFrustum()
	{
		m_xmFrustum.CreateFromMatrix(m_xmFrustum, XMLoadFloat4x4(&m_xmf4x4Projection));
		XMMATRIX xmmtxInversView = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4View));
		m_xmFrustum.Transform(m_xmFrustum, xmmtxInversView);
	}

	BoundingFrustum GetBoundingFrustum()const { return m_xmFrustum; }

	bool IsInFrustum(const BoundingOrientedBox& xmOOBB)
	{
		return m_xmFrustum.Intersects(xmOOBB);
	}
	bool IsInFrustum(const BoundingFrustum& xmFrustum)
	{
		return m_xmFrustum.Intersects(xmFrustum);
	}
};

class CFirstPersonCamera : public CCamera
{
public:
	CFirstPersonCamera(const shared_ptr<CCamera>& pCamera);
	virtual ~CFirstPersonCamera() { }

	virtual void Update(XMFLOAT3& xmf3LookAt, float fElapsedTime);
	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);
};

class CThirdPersonCamera : public CCamera
{
public:
	CThirdPersonCamera(const shared_ptr<CCamera>& pCamera);
	virtual ~CThirdPersonCamera() { }

	virtual void Update(XMFLOAT3& xmf3LookAt, float fElapsedTime);
	virtual void SetLookAt(XMFLOAT3& vLookAt);
};

typedef struct LIGHT;

class CLightCamera : public CCamera
{
public:
	CLightCamera();
	virtual ~CLightCamera() { }

	//bool operator<(const CLightCamera& A) {
	//	const float epsilon = 1e-5f; // 허용 오차
	//	//XMFLOAT3 clToA = Vector3::Subtract(clientCamera->GetPosition(), A->GetPosition());
	//	//XMFLOAT3 clToB = Vector3::Subtract(clientCamera->GetPosition(), B->GetPosition());
	//	//return Vector3::Length(clToA) - Vector3::Length(clToB) < epsilon;
	//}
	shared_ptr<LIGHT> m_pLight;
	//shared_ptr<CCamera> m_pPlayerCamera;
};


