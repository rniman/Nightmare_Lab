#pragma once
#include "Player.h"


class Trail
{
public:
	CGameObject* testobject;

	shared_ptr<CMaterial> m_pMaterial;

	shared_ptr<CGameObject> m_pObject;

	struct TrailVertex {
		XMFLOAT3 position;
		XMFLOAT2 uv;
		float startTime;
	};

	void SetObject(const shared_ptr<CGameObject>& object){
		m_pObject = object;
	}

public:
	// 쉐이더를 가지고 있다는 것에 유의, 쉐이더 렌더타겟 변경시 생성자 수정해야함.
	Trail(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	~Trail(){}

	void CreateShaderVariable(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList);
	void Update();
	void TrailInit();
	void TrailStart();

	void FirstTrailGenerate(float interval);

	D3D12_PRIMITIVE_TOPOLOGY m_d3dPrimitiveTopology;

	ComPtr<ID3D12Resource> m_pd3dVertexBuffer;
	ComPtr<ID3D12Resource> m_pd3dVertexUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3dVertexBufferView;
	UINT m_nVertices;

	bool m_bStart;
	float m_fUVTime;
};

