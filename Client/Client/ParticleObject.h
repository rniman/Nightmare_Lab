#pragma once
#include "Object.h"
#include "ParticleMesh.h"

class CParticleObject : public CGameObject {
protected:
	shared_ptr<CParticleMesh> m_pParticleMesh = nullptr;

	vector<ComPtr<ID3D12PipelineState>> m_vpRefPSO;
public:
	CParticleObject() = default;
	// 파티클은 1개의 텍스쳐를 가지도록 한다.
	CParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) 
		: CGameObject(pd3dDevice, pd3dCommandList,1) {}

	~CParticleObject() = default;

	virtual void ReadByteTask() {}

	void SetRefPSO(vector<ComPtr<ID3D12PipelineState>>& pso) {
		m_vpRefPSO = pso;
	}

	virtual void SetMesh(const shared_ptr<CMesh>& pMesh);
	virtual void Update(float fcurTime) {}

	int SetParticleInsEnable(int id, bool val, float fCurTime, XMFLOAT3& pos) {
		return m_pParticleMesh->SetParticleInsEnable(id, val, fCurTime, pos);
	}

	void SetParticlePosition(int id, XMFLOAT3& pos) {
		m_pParticleMesh->SetParticlePosition(id, pos);
	}
	void AddParticle(XMFLOAT3& pos) {
		m_pParticleMesh->AddParticle(pos);
	}
};

class CSOParticleObject : public CParticleObject {
public:
	CSOParticleObject() = default;
	CSOParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
		: CParticleObject(pd3dDevice, pd3dCommandList) {}
	~CSOParticleObject() = default;

	void Render(ID3D12GraphicsCommandList* pd3dCommandList) override;
	void ReadByteTask() override;

};

class CGSParticleObject : public CParticleObject {
public:
	CGSParticleObject() = default;
	CGSParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
		: CParticleObject(pd3dDevice, pd3dCommandList) {}
	~CGSParticleObject() = default;

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Update(float fcurTime);
};