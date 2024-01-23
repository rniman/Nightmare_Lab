#include "Mesh.h"


CMesh::CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

CMesh::~CMesh()
{
	if (m_pd3dVertexBuffer) m_pd3dVertexBuffer->Release();

	if (m_nSubMeshes > 0)
	{
		for (int i = 0; i < m_nSubMeshes; i++)
		{
			if (m_ppd3dSubSetIndexBuffers[i]) m_ppd3dSubSetIndexBuffers[i]->Release();
			if (m_ppnSubSetIndices[i]) delete[] m_ppnSubSetIndices[i];
		}
		if (m_ppd3dSubSetIndexBuffers) delete[] m_ppd3dSubSetIndexBuffers;
		if (m_pd3dSubSetIndexBufferViews) delete[] m_pd3dSubSetIndexBufferViews;

		if (m_pnSubSetIndices) delete[] m_pnSubSetIndices;
		if (m_ppnSubSetIndices) delete[] m_ppnSubSetIndices;
	}

	if (m_pxmf3ModelPositions) delete[] m_pxmf3ModelPositions;
}

void CMesh::ReleaseUploadBuffers()
{
	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer->Release();
	m_pd3dVertexUploadBuffer = NULL;

	if ((m_nSubMeshes > 0) && m_ppd3dSubSetIndexUploadBuffers)
	{
		for (int i = 0; i < m_nSubMeshes; i++)
		{
			if (m_ppd3dSubSetIndexUploadBuffers[i]) m_ppd3dSubSetIndexUploadBuffers[i]->Release();
		}
		if (m_ppd3dSubSetIndexUploadBuffers) delete[] m_ppd3dSubSetIndexUploadBuffers;
		m_ppd3dSubSetIndexUploadBuffers = NULL;
	}
}

void CMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dVertexBufferView);
}

void CMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	OnPreRender(pd3dCommandList);
	
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
//// HexahedronMesh
HexahedronMesh::HexahedronMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float xSize, float ySize, float zSize) : CMesh(pd3dDevice,pd3dCommandList)
{
	m_nVertices = 36;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pxmf3ModelPositions = new XMFLOAT3[m_nVertices];

	float fx = xSize * 0.5f, fy = ySize * 0.5f, fz = zSize * 0.5f;

	int i = 0;

	//정점 버퍼 데이터는 삼각형 리스트이므로 36개의 정점 데이터를 준비한다. 
	//ⓐ 앞면(Front) 사각형의 위쪽 삼각형
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, +fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, +fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, -fy, -fz);
	//ⓑ 앞면(Front) 사각형의 아래쪽 삼각형
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, +fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, -fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, -fy, -fz);
	//ⓒ 윗면(Top) 사각형의 위쪽 삼각형
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, +fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, +fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, +fy, -fz);
	//ⓓ 윗면(Top) 사각형의 아래쪽 삼각형
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, +fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, +fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, +fy, -fz);
	//ⓔ 뒷면(Back) 사각형의 위쪽 삼각형
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, -fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, -fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, +fy, +fz);
	//ⓕ 뒷면(Back) 사각형의 아래쪽 삼각형
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, -fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, +fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, +fy, +fz);
	//ⓖ 아래면(Bottom) 사각형의 위쪽 삼각형
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, -fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, -fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, -fy, +fz);
	//ⓗ 아래면(Bottom) 사각형의 아래쪽 삼각형
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, -fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, -fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, -fy, +fz);
	//ⓘ 옆면(Left) 사각형의 위쪽 삼각형
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, +fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, +fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, -fy, -fz);
	//ⓙ 옆면(Left) 사각형의 아래쪽 삼각형
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, +fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, -fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, -fy, +fz);
	//ⓚ 옆면(Right) 사각형의 위쪽 삼각형
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, +fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, +fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, -fy, +fz);
	//ⓛ 옆면(Right) 사각형의 아래쪽 삼각형
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, +fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, -fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, -fy, -fz);

	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3ModelPositions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dVertexBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
}

HexahedronMesh::~HexahedronMesh()
{

}