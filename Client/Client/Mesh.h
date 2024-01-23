#pragma once
#include "stdafx.h"


class CMesh
{
private:
	int								m_nReferences = 0;
public:
	CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CMesh();

	char							m_pstrMeshName[64] = { 0 };

	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	UINT GetType() { return(m_nType); }

	/*virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) { }
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) { }
	virtual void ReleaseShaderVariables() { }*/

	virtual void ReleaseUploadBuffers();

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);

protected:
	UINT							m_nType = 0x00;

	XMFLOAT3						m_xmf3AABBCenter = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3						m_xmf3AABBExtents = XMFLOAT3(0.0f, 0.0f, 0.0f);

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
};