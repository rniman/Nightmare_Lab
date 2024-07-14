#pragma once
#include "Mesh.h"

#define MAX_PARTICLE 999999

class CParticleMesh : public CMesh {
public:
	enum TYPE {
		TP = 0,
		SPARK,
		BUBBLE,
		FOOTPRINT
	};
	struct TYPE_TIME {
		static constexpr float TP = 8.0f;
		static constexpr float BUBBLE = -1.0f;
		static constexpr float GETITEM = 2.0f;
	};
public:
	struct ParticleVertex {
		XMFLOAT3 position;
		XMFLOAT3 velocity;
		float lifeTime;
		unsigned int type;
		float startTime;
	};
	array<ParticleVertex, MAX_PARTICLE> m_particleVertex;
	UINT m_nStride;

	struct ParticleInstanceVertex {
		UINT id;
		XMFLOAT3 start_position;
		float start_Time;
	};

	ComPtr<ID3D12Resource> m_pd3dInstanceParticleBuffer;
	ComPtr<ID3D12Resource> m_pd3dInstanceParticleUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3dInstanceParticleBufferView;

	array<ParticleInstanceVertex, PARITICLE_INSTANCE_COUNT> m_particleInsVertex;

	UINT draw_instance_count;

	struct ParticleTypeInsInfo {
		bool b_enabled = false;
		unsigned int type;
		float startTime = 0.0f;
	};
	array<ParticleTypeInsInfo, PARITICLE_INSTANCE_COUNT> m_ParticleInsInfos;
public:

	CParticleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
		:CMesh(pd3dDevice, pd3dCommandList) {}
	~CParticleMesh() = default;

	virtual void ReadByteTask() {}
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void SORenderTask(ID3D12GraphicsCommandList* pd3dCommandList) {}
	virtual void Update(float fcurtime) {}
	array<ParticleTypeInsInfo, PARITICLE_INSTANCE_COUNT>& GetParticleInsInfo() { return m_ParticleInsInfos; }

	void CreateBufferViewResource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pVertices, UINT nVertices, UINT nStride);
	/// Id가 -1 값일때 남아있는 비활성화 인스턴스 중 하나를 고름
	virtual int SetParticleInsEnable(int id, bool val, float fCurTime, XMFLOAT3& pos);
	void SetParticlePosition(int id, XMFLOAT3& pos);
	void UpdateUploadBuffer(int newCount, int oldCount, vector<int>& v_enable);
	virtual void AddParticle(XMFLOAT3& pos) {}
};

class CSOParticleMesh : public CParticleMesh {
public:
	CSOParticleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
		:CParticleMesh(pd3dDevice, pd3dCommandList) {}
	~CSOParticleMesh() = default;

	UINT64 m_maxParticle;

	ComPtr<ID3D12Resource> m_SOBuffer;
	ComPtr<ID3D12Resource> m_SOUploadBuffer;
	ComPtr<ID3D12Resource> m_RenderBuffer;
	ComPtr<ID3D12Resource> m_RenderUploadBuffer;
	D3D12_STREAM_OUTPUT_BUFFER_VIEW m_d3dSObufferView;

	ComPtr<ID3D12Resource> m_pd3dBufFilledSize;
	ComPtr<ID3D12Resource> m_pd3dBufFilledSizeUploadBuffer;
	ComPtr<ID3D12Resource> m_pd3dInitBufFilledSize;
	ComPtr<ID3D12Resource> m_pd3dInitBufFilledSizeUploadBuffer;
	UINT64* m_pnInitBufFilledSize;
	ComPtr<ID3D12Resource> m_pd3dReadBufFilledSize;
	ComPtr<ID3D12Resource>m_pd3dReadBufFilledSizeUploadBuffer;

	bool m_bInit = true;

	void CreateShaderVariable(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int maxParticle, XMFLOAT3 position);
	void SORenderTask(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReadByteTask();
	int SetParticleInsEnable(int id, bool val, float fCurTime, XMFLOAT3& pos) override;
};

class CDefaultParticleMesh : public CParticleMesh {
public:
	CDefaultParticleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	~CDefaultParticleMesh() = default;

	void CreateShaderVariable(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, TYPE type, UINT nVertices);
	void Update(float fcurtime);
};

class CTPParticleMesh : public CDefaultParticleMesh {
public:
	CTPParticleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	~CTPParticleMesh() = default;

	void CreateShaderVariable(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void Update(float fcurtime);
};

class CFootPrintParticleMesh : public CDefaultParticleMesh {
public:
	CFootPrintParticleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	~CFootPrintParticleMesh() = default;

	void CreateShaderVariable(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void Update(float fcurtime);

	void AddParticle(XMFLOAT3& pos);
public:
};