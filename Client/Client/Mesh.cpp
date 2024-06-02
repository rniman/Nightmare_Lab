#include "stdafx.h"
#include "Mesh.h"
#include "Object.h"
#include "EnvironmentObject.h"
#include "Scene.h"
#include "Collision.h"


CMesh::CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

CMesh::~CMesh()
{
	ReleaseUploadBuffers();

	if (m_pd3dVertexBuffer) m_pd3dVertexBuffer->Release();

	if (m_nSubMeshes > 0)
	{
		for (int i = 0; i < m_nSubMeshes; i++)
		{
			if (m_vpd3dSubSetIndexBuffers.size() == 0) break;
			if (m_vpd3dSubSetIndexBuffers[i].Get()) m_vpd3dSubSetIndexBuffers[i].Reset();
			if (m_ppnSubSetIndices[i]) delete[] m_ppnSubSetIndices[i];
		}
		if (m_vpd3dSubSetIndexBuffers.size() != 0) {
			if (m_pnSubSetIndices) delete[] m_pnSubSetIndices;
			if (m_ppnSubSetIndices) delete[] m_ppnSubSetIndices;
		}
	}

	if (m_pxmf3ModelPositions) delete[] m_pxmf3ModelPositions;
}

void CMesh::ReleaseUploadBuffers()
{
	if (m_pd3dVertexUploadBuffer.Get())	m_pd3dVertexUploadBuffer.Reset();

	if (m_nSubMeshes > 0)
	{
		for (int i = 0; i < m_nSubMeshes; i++)
		{
			if (m_vpd3dSubSetIndexUploadBuffers.size() == 0) {
				break;
			}
			if (m_vpd3dSubSetIndexUploadBuffers[i].Get()) m_vpd3dSubSetIndexUploadBuffers[i].Reset();
		}
	}
}

void CMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dVertexBufferView);
}

void CMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	UpdateShaderVariables(pd3dCommandList);

	OnPreRender(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_vd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubSet], m_nCntInstance, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, m_nCntInstance, m_nOffset, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// [0504] UI위한 사각형 메쉬

CUserInterfaceRectMesh::CUserInterfaceRectMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth, float fMaxU, float fMaxV, float fMinU, float fMinV)
	: CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 6;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pxmf3ModelPositions = new XMFLOAT3[m_nVertices];
	m_pxmf2UV0 = new XMFLOAT2[m_nVertices];

	float fx = (fWidth * 0.5f), fy = (fHeight * 0.5f), fz = (fDepth * 0.5f);

	m_pxmf3ModelPositions[0] = XMFLOAT3(+fx, +fy, fz); m_pxmf2UV0[0] = XMFLOAT2(fMaxU, fMinV);
	m_pxmf3ModelPositions[1] = XMFLOAT3(+fx, -fy, fz); m_pxmf2UV0[1] = XMFLOAT2(fMaxU, fMaxV);
	m_pxmf3ModelPositions[2] = XMFLOAT3(-fx, -fy, fz); m_pxmf2UV0[2] = XMFLOAT2(fMinU, fMaxV);
	m_pxmf3ModelPositions[3] = XMFLOAT3(-fx, -fy, fz); m_pxmf2UV0[3] = XMFLOAT2(fMinU, fMaxV);
	m_pxmf3ModelPositions[4] = XMFLOAT3(-fx, +fy, fz); m_pxmf2UV0[4] = XMFLOAT2(fMinU, fMinV);
	m_pxmf3ModelPositions[5] = XMFLOAT3(+fx, +fy, fz); m_pxmf2UV0[5] = XMFLOAT2(fMaxU, fMinV);

	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3ModelPositions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dVertexBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dUV0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2UV0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dUV0Buffer);

	m_d3dUV0BufferView.BufferLocation = m_pd3dUV0Buffer->GetGPUVirtualAddress();
	m_d3dUV0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dUV0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
}

CUserInterfaceRectMesh::~CUserInterfaceRectMesh()
{
}

void CUserInterfaceRectMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	//if (m_pd3dUV0UploadBuffer.Get()) m_pd3dUV0UploadBuffer.Reset();
}

void CUserInterfaceRectMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	D3D12_VERTEX_BUFFER_VIEW views[] = { m_d3dVertexBufferView ,m_d3dUV0BufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, views);
}

void CUserInterfaceRectMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	CMesh::Render(pd3dCommandList, nSubSet);
}

inline void CUserInterfaceRectMesh::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT8* pBufferDataBegin = NULL;
	m_pd3dVertexBuffer->Map(0, NULL, (void**)&pBufferDataBegin);
	memcpy(pBufferDataBegin, m_pxmf3ModelPositions, sizeof(XMFLOAT3) * m_nVertices);
	m_pd3dVertexBuffer->Unmap(0, NULL);

	m_pd3dUV0Buffer->Map(0, NULL, (void**)&pBufferDataBegin);
	memcpy(pBufferDataBegin, m_pxmf2UV0, sizeof(XMFLOAT2) * m_nVertices);
	m_pd3dUV0Buffer->Unmap(0, NULL);
}

void CUserInterfaceRectMesh::SetVertexData(float fWidth, float fHeight, float fMaxU, float fMaxV, float fMinU, float fMinV)
{
	float fx = (fWidth * 0.5f), fy = (fHeight * 0.5f), fz = (0.0f * 0.5f);
	m_pxmf3ModelPositions[0] = XMFLOAT3(+fx, +fy, fz); m_pxmf2UV0[0] = XMFLOAT2(fMaxU, fMinV);
	m_pxmf3ModelPositions[1] = XMFLOAT3(+fx, -fy, fz); m_pxmf2UV0[1] = XMFLOAT2(fMaxU, fMaxV);
	m_pxmf3ModelPositions[2] = XMFLOAT3(-fx, -fy, fz); m_pxmf2UV0[2] = XMFLOAT2(fMinU, fMaxV);
	m_pxmf3ModelPositions[3] = XMFLOAT3(-fx, -fy, fz); m_pxmf2UV0[3] = XMFLOAT2(fMinU, fMaxV);
	m_pxmf3ModelPositions[4] = XMFLOAT3(-fx, +fy, fz); m_pxmf2UV0[4] = XMFLOAT2(fMinU, fMinV);
	m_pxmf3ModelPositions[5] = XMFLOAT3(+fx, +fy, fz); m_pxmf2UV0[5] = XMFLOAT2(fMaxU, fMinV);
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

	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, +fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, +fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, -fy, -fz);

	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, +fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, -fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, -fy, -fz);

	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, +fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, +fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, +fy, -fz);

	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, +fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, +fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, +fy, -fz);

	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, -fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, -fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, +fy, +fz);

	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, -fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, +fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, +fy, +fz);

	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, -fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, -fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, -fy, +fz);

	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, -fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, -fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, -fy, +fz);

	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, +fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, +fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, -fy, -fz);

	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, +fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, -fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(-fx, -fy, +fz);

	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, +fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, +fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, -fy, +fz);

	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, +fy, -fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, -fy, +fz);
	m_pxmf3ModelPositions[i++] = XMFLOAT3(+fx, -fy, -fz);

	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3ModelPositions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dVertexBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	XMFLOAT2 pxmf2TexCoords[36];
	i = 0;
	pxmf2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	pxmf2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	pxmf2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	pxmf2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	pxmf2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	pxmf2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	pxmf2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	pxmf2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	pxmf2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	pxmf2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	pxmf2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	pxmf2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	pxmf2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	m_pd3dUV0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pxmf2TexCoords, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dUV0UploadBuffer);

	m_d3dUV0BufferView.BufferLocation = m_pd3dUV0Buffer->GetGPUVirtualAddress();
	m_d3dUV0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dUV0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
}

HexahedronMesh::~HexahedronMesh()
{

}

void HexahedronMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	D3D12_VERTEX_BUFFER_VIEW views[] = { m_d3dVertexBufferView ,m_d3dUV0BufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, views);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
CStandardMesh::CStandardMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CMesh(pd3dDevice, pd3dCommandList)
{
}

CStandardMesh::~CStandardMesh()
{
	if (m_pd3dUV0Buffer) m_pd3dUV0Buffer->Release();
	if (m_pd3dUV1Buffer) m_pd3dUV1Buffer->Release();
	if (m_pd3dNormalBuffer) m_pd3dNormalBuffer->Release();
	if (m_pd3dTangentBuffer) m_pd3dTangentBuffer->Release();
	if (m_pd3dBiTangentBuffer) m_pd3dBiTangentBuffer->Release();

	if (m_pxmf4Colors) delete[] m_pxmf4Colors;
	if (m_pxmf3Normals) delete[] m_pxmf3Normals;
	if (m_pxmf3Tangents) delete[] m_pxmf3Tangents;
	if (m_pxmf3BiTangents) delete[] m_pxmf3BiTangents;
	if (m_pxmf2UVs0) delete[] m_pxmf2UVs0;
	if (m_pxmf2UVs1) delete[] m_pxmf2UVs1;
}

void CStandardMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	if (m_pd3dUV0UploadBuffer.Get()) m_pd3dUV0UploadBuffer.Reset();

	if (m_pd3dNormalUploadBuffer.Get()) m_pd3dNormalUploadBuffer.Reset();
	
	if (m_pd3dTangentUploadBuffer.Get()) m_pd3dTangentUploadBuffer.Reset();
	
	if (m_pd3dBiTangentUploadBuffer.Get()) m_pd3dBiTangentUploadBuffer.Reset();
	
}
void CStandardMesh::SaveStandardMesh(shared_ptr<CStandardMesh> pMesh)
{
	for (auto& mesh : CStandardMesh::g_vAllstandardMesh) {
		if (!strcmp(mesh->m_pstrMeshName, pMesh->m_pstrMeshName/*@제외 비교*/)) {
			pMesh->m_d3dVertexBufferView = mesh->m_d3dVertexBufferView;
			pMesh->m_vd3dSubSetIndexBufferViews = mesh->m_vd3dSubSetIndexBufferViews;
			pMesh->m_nSubMeshes = mesh->m_nSubMeshes;
			pMesh->m_pnSubSetIndices = mesh->m_pnSubSetIndices;
			pMesh->m_ppnSubSetIndices = mesh->m_ppnSubSetIndices;
			pMesh->m_d3dUV0BufferView = mesh->m_d3dUV0BufferView;
			pMesh->m_d3dUV1BufferView = mesh->m_d3dUV1BufferView;
			pMesh->m_d3dNormalBufferView = mesh->m_d3dNormalBufferView;
			pMesh->m_d3dTangentBufferView = mesh->m_d3dTangentBufferView;
			pMesh->m_d3dBiTangentBufferView = mesh->m_d3dBiTangentBufferView;
			pMesh->m_nSlot = mesh->m_nSlot;
			pMesh->m_nType = mesh->m_nType;
			pMesh->m_vOOBBs = mesh->m_vOOBBs;

			return; // 찾았다면 로직을 수행하고 리턴
		}
	}

	// 컨테이너에 메쉬가 없다면 처음 만들어진 메쉬
	CStandardMesh::g_vAllstandardMesh.push_back(pMesh);
}

bool CStandardMesh::LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile)
{
	char pstrToken[64] = { '\0' };
	int nPositions = 0, nColors = 0, nNormals = 0, nTangents = 0, nBiTangents = 0, nUVs = 0, nIndices = 0, nSubMeshes = 0, nSubIndices = 0;

	UINT nReads = (UINT)::fread(&m_nVertices, sizeof(int), 1, pInFile);

	::ReadStringFromFile(pInFile, m_pstrMeshName);
	if (m_pstrMeshName[0] == '@') 
	{
		return false;
	}
	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Bounds>:"))
		{
			XMFLOAT3 xmf3AABBCenter, xmf3AABBExtents;
			nReads = (UINT)::fread(&xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
			m_vOOBBs.push_back(BoundingOrientedBox(xmf3AABBCenter, xmf3AABBExtents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)));
		}
		else if (!strcmp(pstrToken, "<BoxColliders>:"))
		{
			int nBoxColliders = 0;
			nReads = fread(&nBoxColliders, sizeof(int), 1, pInFile);
			m_vOOBBs.reserve(nBoxColliders);
		}
		else if (!strcmp(pstrToken, "<Bound>:"))
		{
			int nIndex = 0;
			XMFLOAT3 xmf3AABBCenter, xmf3AABBExtents;
			nReads = fread(&nIndex, sizeof(int), 1, pInFile);
			nReads = (UINT)::fread(&xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
			m_vOOBBs.emplace_back(xmf3AABBCenter, xmf3AABBExtents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		}
		else if (!strcmp(pstrToken, "<Positions>:"))
		{
			nReads = (UINT)::fread(&nPositions, sizeof(int), 1, pInFile);
			if (nPositions > 0)
			{
				m_nType |= VERTEXT_POSITION;
				m_pxmf3ModelPositions = new XMFLOAT3[nPositions];
				nReads = (UINT)::fread(m_pxmf3ModelPositions, sizeof(XMFLOAT3), nPositions, pInFile);

				m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3ModelPositions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

				m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
				m_d3dVertexBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dVertexBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Colors>:"))
		{
			nReads = (UINT)::fread(&nColors, sizeof(int), 1, pInFile);
			if (nColors > 0)
			{
				m_nType |= VERTEXT_COLOR;
				m_pxmf4Colors = new XMFLOAT4[nColors];
				nReads = (UINT)::fread(m_pxmf4Colors, sizeof(XMFLOAT4), nColors, pInFile);
			}
		}
		else if (!strcmp(pstrToken, "<TextureCoords0>:"))
		{
			nReads = (UINT)::fread(&nUVs, sizeof(int), 1, pInFile);
			if (nUVs > 0)
			{
				m_nType |= VERTEXT_TEXTURE_COORD0;
				m_pxmf2UVs0 = new XMFLOAT2[nUVs];
				nReads = (UINT)::fread(m_pxmf2UVs0, sizeof(XMFLOAT2), nUVs, pInFile);

				m_pd3dUV0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2UVs0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dUV0UploadBuffer);

				m_d3dUV0BufferView.BufferLocation = m_pd3dUV0Buffer->GetGPUVirtualAddress();
				m_d3dUV0BufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_d3dUV0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<TextureCoords1>:"))
		{
			nReads = (UINT)::fread(&nUVs, sizeof(int), 1, pInFile);
			if (nUVs > 0)
			{
				m_nType |= VERTEXT_TEXTURE_COORD1;
				m_pxmf2UVs1 = new XMFLOAT2[nUVs];
				nReads = (UINT)::fread(m_pxmf2UVs1, sizeof(XMFLOAT2), nUVs, pInFile);

				m_pd3dUV1Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2UVs1, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dUV1UploadBuffer);

				m_d3dUV1BufferView.BufferLocation = m_pd3dUV1Buffer->GetGPUVirtualAddress();
				m_d3dUV1BufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_d3dUV1BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Normals>:"))
		{
			nReads = (UINT)::fread(&nNormals, sizeof(int), 1, pInFile);
			if (nNormals > 0)
			{
				m_nType |= VERTEXT_NORMAL;
				m_pxmf3Normals = new XMFLOAT3[nNormals];
				nReads = (UINT)::fread(m_pxmf3Normals, sizeof(XMFLOAT3), nNormals, pInFile);

				m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);

				m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
				m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Tangents>:"))
		{
			nReads = (UINT)::fread(&nTangents, sizeof(int), 1, pInFile);
			if (nTangents > 0)
			{
				m_nType |= VERTEXT_TANGENT;
				m_pxmf3Tangents = new XMFLOAT3[nTangents];
				nReads = (UINT)::fread(m_pxmf3Tangents, sizeof(XMFLOAT3), nTangents, pInFile);

				m_pd3dTangentBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Tangents, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTangentUploadBuffer);

				m_d3dTangentBufferView.BufferLocation = m_pd3dTangentBuffer->GetGPUVirtualAddress();
				m_d3dTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<BiTangents>:"))
		{
			nReads = (UINT)::fread(&nBiTangents, sizeof(int), 1, pInFile);
			if (nBiTangents > 0)
			{
				m_pxmf3BiTangents = new XMFLOAT3[nBiTangents];
				nReads = (UINT)::fread(m_pxmf3BiTangents, sizeof(XMFLOAT3), nBiTangents, pInFile);

				m_pd3dBiTangentBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3BiTangents, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBiTangentUploadBuffer);

				m_d3dBiTangentBufferView.BufferLocation = m_pd3dBiTangentBuffer->GetGPUVirtualAddress();
				m_d3dBiTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dBiTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<SubMeshes>:"))
		{
			nReads = (UINT)::fread(&(m_nSubMeshes), sizeof(int), 1, pInFile);
			if (m_nSubMeshes > 0)
			{
				m_pnSubSetIndices = new int[m_nSubMeshes];
				m_ppnSubSetIndices = new UINT * [m_nSubMeshes];

				m_vpd3dSubSetIndexBuffers.reserve(m_nSubMeshes);
				m_vpd3dSubSetIndexUploadBuffers.reserve(m_nSubMeshes);
				m_vd3dSubSetIndexBufferViews.reserve(m_nSubMeshes);
				for (int i = 0; i < m_nSubMeshes; ++i)
				{
					m_vpd3dSubSetIndexBuffers.emplace_back();
					m_vpd3dSubSetIndexUploadBuffers.emplace_back();
					m_vd3dSubSetIndexBufferViews.emplace_back();
				}

				for (int i = 0; i < m_nSubMeshes; i++)
				{
					::ReadStringFromFile(pInFile, pstrToken);
					if (!strcmp(pstrToken, "<SubMesh>:"))
					{
						int nIndex = 0;
						nReads = (UINT)::fread(&nIndex, sizeof(int), 1, pInFile); //i
						nReads = (UINT)::fread(&(m_pnSubSetIndices[i]), sizeof(int), 1, pInFile);
						if (m_pnSubSetIndices[i] > 0)
						{
							m_ppnSubSetIndices[i] = new UINT[m_pnSubSetIndices[i]];
							nReads = (UINT)::fread(m_ppnSubSetIndices[i], sizeof(UINT), m_pnSubSetIndices[i], pInFile);

							m_vpd3dSubSetIndexBuffers[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[i], sizeof(UINT) * m_pnSubSetIndices[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, m_vpd3dSubSetIndexUploadBuffers[i].GetAddressOf());

							m_vd3dSubSetIndexBufferViews[i].BufferLocation = m_vpd3dSubSetIndexBuffers[i]->GetGPUVirtualAddress();
							m_vd3dSubSetIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
							m_vd3dSubSetIndexBufferViews[i].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[i];
						}
					}
				}
			}
		}
		//else if (!strcmp(pstrToken, "<InstanceTransformMatrix>:"))
		//{
		//	LoadInstanceData(pd3dDevice, pd3dCommandList, pInFile);
		//}
		else if (!strcmp(pstrToken, "</Mesh>"))
		{
			break;
		}
	}
	return true;
}

void CStandardMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[5] = { m_d3dVertexBufferView, m_d3dUV0BufferView, m_d3dNormalBufferView, m_d3dTangentBufferView, m_d3dBiTangentBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 5, pVertexBufferViews);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

CInstanceStandardMesh::CInstanceStandardMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) 
	: CStandardMesh(pd3dDevice,pd3dCommandList)
{

}

CInstanceStandardMesh::~CInstanceStandardMesh()
{
	if (m_pxmf4x4InstanceTransformMatrix) delete[] m_pxmf4x4InstanceTransformMatrix;
}

bool CInstanceStandardMesh::LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile, const shared_ptr<CInstanceObject>& pGameObject)
{
	m_pOriginInstance = pGameObject;
	char pstrToken[64] = { '\0' };
	int nPositions = 0, nColors = 0, nNormals = 0, nTangents = 0, nBiTangents = 0, nUVs = 0, nIndices = 0, nSubMeshes = 0, nSubIndices = 0;

	UINT nReads = (UINT)::fread(&m_nVertices, sizeof(int), 1, pInFile);

	::ReadStringFromFile(pInFile, m_pstrMeshName);
	if (m_pstrMeshName[0] != '@') {
		//return false;
	}
	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Bounds>:"))
		{
			XMFLOAT3 xmf3AABBCenter, xmf3AABBExtents;
			nReads = (UINT)::fread(&xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
			m_vOOBBs.push_back(BoundingOrientedBox(xmf3AABBCenter, xmf3AABBExtents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)));
		}
		else if (!strcmp(pstrToken, "<BoxColliders>:"))
		{
			int nBoxColliders = 0;
			nReads = fread(&nBoxColliders, sizeof(int), 1, pInFile);
			m_vOOBBs.reserve(nBoxColliders);
		}
		else if (!strcmp(pstrToken, "<Bound>:"))
		{
			int nIndex = 0;
			XMFLOAT3 xmf3AABBCenter, xmf3AABBExtents;
			nReads = fread(&nIndex, sizeof(int), 1, pInFile);
			nReads = (UINT)::fread(&xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
			m_vOOBBs.emplace_back(xmf3AABBCenter, xmf3AABBExtents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		}
		else if (!strcmp(pstrToken, "<Positions>:"))
		{
			nReads = (UINT)::fread(&nPositions, sizeof(int), 1, pInFile);
			if (nPositions > 0)
			{
				m_nType |= VERTEXT_POSITION;
				m_pxmf3ModelPositions = new XMFLOAT3[nPositions];
				nReads = (UINT)::fread(m_pxmf3ModelPositions, sizeof(XMFLOAT3), nPositions, pInFile);

				m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3ModelPositions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

				m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
				m_d3dVertexBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dVertexBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Colors>:"))
		{
			nReads = (UINT)::fread(&nColors, sizeof(int), 1, pInFile);
			if (nColors > 0)
			{
				m_nType |= VERTEXT_COLOR;
				m_pxmf4Colors = new XMFLOAT4[nColors];
				nReads = (UINT)::fread(m_pxmf4Colors, sizeof(XMFLOAT4), nColors, pInFile);
			}
		}
		else if (!strcmp(pstrToken, "<TextureCoords0>:"))
		{
			nReads = (UINT)::fread(&nUVs, sizeof(int), 1, pInFile);
			if (nUVs > 0)
			{
				m_nType |= VERTEXT_TEXTURE_COORD0;
				m_pxmf2UVs0 = new XMFLOAT2[nUVs];
				nReads = (UINT)::fread(m_pxmf2UVs0, sizeof(XMFLOAT2), nUVs, pInFile);

				m_pd3dUV0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2UVs0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dUV0UploadBuffer);

				m_d3dUV0BufferView.BufferLocation = m_pd3dUV0Buffer->GetGPUVirtualAddress();
				m_d3dUV0BufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_d3dUV0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<TextureCoords1>:"))
		{
			nReads = (UINT)::fread(&nUVs, sizeof(int), 1, pInFile);
			if (nUVs > 0)
			{
				m_nType |= VERTEXT_TEXTURE_COORD1;
				m_pxmf2UVs1 = new XMFLOAT2[nUVs];
				nReads = (UINT)::fread(m_pxmf2UVs1, sizeof(XMFLOAT2), nUVs, pInFile);

				m_pd3dUV1Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2UVs1, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dUV1UploadBuffer);

				m_d3dUV1BufferView.BufferLocation = m_pd3dUV1Buffer->GetGPUVirtualAddress();
				m_d3dUV1BufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_d3dUV1BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Normals>:"))
		{
			nReads = (UINT)::fread(&nNormals, sizeof(int), 1, pInFile);
			if (nNormals > 0)
			{
				m_nType |= VERTEXT_NORMAL;
				m_pxmf3Normals = new XMFLOAT3[nNormals];
				nReads = (UINT)::fread(m_pxmf3Normals, sizeof(XMFLOAT3), nNormals, pInFile);

				m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);

				m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
				m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Tangents>:"))
		{
			nReads = (UINT)::fread(&nTangents, sizeof(int), 1, pInFile);
			if (nTangents > 0)
			{
				m_nType |= VERTEXT_TANGENT;
				m_pxmf3Tangents = new XMFLOAT3[nTangents];
				nReads = (UINT)::fread(m_pxmf3Tangents, sizeof(XMFLOAT3), nTangents, pInFile);

				m_pd3dTangentBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Tangents, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTangentUploadBuffer);

				m_d3dTangentBufferView.BufferLocation = m_pd3dTangentBuffer->GetGPUVirtualAddress();
				m_d3dTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<BiTangents>:"))
		{
			nReads = (UINT)::fread(&nBiTangents, sizeof(int), 1, pInFile);
			if (nBiTangents > 0)
			{
				m_pxmf3BiTangents = new XMFLOAT3[nBiTangents];
				nReads = (UINT)::fread(m_pxmf3BiTangents, sizeof(XMFLOAT3), nBiTangents, pInFile);

				m_pd3dBiTangentBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3BiTangents, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBiTangentUploadBuffer);

				m_d3dBiTangentBufferView.BufferLocation = m_pd3dBiTangentBuffer->GetGPUVirtualAddress();
				m_d3dBiTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dBiTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<SubMeshes>:"))
		{
			nReads = (UINT)::fread(&(m_nSubMeshes), sizeof(int), 1, pInFile);
			if (m_nSubMeshes > 0)
			{
				m_pnSubSetIndices = new int[m_nSubMeshes];
				m_ppnSubSetIndices = new UINT * [m_nSubMeshes];

				m_vpd3dSubSetIndexBuffers.reserve(m_nSubMeshes);
				m_vpd3dSubSetIndexUploadBuffers.reserve(m_nSubMeshes);
				m_vd3dSubSetIndexBufferViews.reserve(m_nSubMeshes);
				for (int i = 0; i < m_nSubMeshes; ++i)
				{
					m_vpd3dSubSetIndexBuffers.emplace_back();
					m_vpd3dSubSetIndexUploadBuffers.emplace_back();
					m_vd3dSubSetIndexBufferViews.emplace_back();
				}

				for (int i = 0; i < m_nSubMeshes; i++)
				{
					::ReadStringFromFile(pInFile, pstrToken);
					if (!strcmp(pstrToken, "<SubMesh>:"))
					{
						int nIndex = 0;
						nReads = (UINT)::fread(&nIndex, sizeof(int), 1, pInFile); //i
						nReads = (UINT)::fread(&(m_pnSubSetIndices[i]), sizeof(int), 1, pInFile);
						if (m_pnSubSetIndices[i] > 0)
						{
							m_ppnSubSetIndices[i] = new UINT[m_pnSubSetIndices[i]];
							nReads = (UINT)::fread(m_ppnSubSetIndices[i], sizeof(UINT), m_pnSubSetIndices[i], pInFile);

							m_vpd3dSubSetIndexBuffers[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[i], sizeof(UINT) * m_pnSubSetIndices[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, m_vpd3dSubSetIndexUploadBuffers[i].GetAddressOf());

							m_vd3dSubSetIndexBufferViews[i].BufferLocation = m_vpd3dSubSetIndexBuffers[i]->GetGPUVirtualAddress();
							m_vd3dSubSetIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
							m_vd3dSubSetIndexBufferViews[i].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[i];
						}
					}
				}
			}
		}
		else if (!strcmp(pstrToken, "<InstanceTransformMatrix>:"))
		{
			//LoadInstanceData(pd3dDevice, pd3dCommandList, pInFile, pGameObject);
			LoadInstanceData(pd3dDevice, pd3dCommandList, pInFile);
		}
		else if (!strcmp(pstrToken, "</Mesh>"))
		{
			break;
		}
	}
	return true;
}

bool g_InstanceMeshNotAddCollision = false;

void CInstanceStandardMesh::LoadInstanceData(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile/*, CInstanceObject* pGameObject*/)
{
	int nReads = (UINT)::fread(&m_nCntInstance, sizeof(int), 1, pInFile);
	
	if (m_nCntInstance > 0)
	{
		m_pxmf4x4InstanceTransformMatrix = new XMFLOAT4X4[m_nCntInstance];
		nReads = (UINT)::fread(m_pxmf4x4InstanceTransformMatrix, sizeof(XMFLOAT4X4), m_nCntInstance, pInFile);

		m_pd3dInstanceTransformMatrixBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf4x4InstanceTransformMatrix, 
			sizeof(XMFLOAT4X4) * m_nCntInstance, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
		
		m_d3dInstanceTransformMatrixBufferView.BufferLocation = m_pd3dInstanceTransformMatrixBuffer->GetGPUVirtualAddress();
		m_d3dInstanceTransformMatrixBufferView.StrideInBytes = sizeof(XMFLOAT4X4);
		m_d3dInstanceTransformMatrixBufferView.SizeInBytes = sizeof(XMFLOAT4X4) * m_nCntInstance;

		if (g_InstanceMeshNotAddCollision) {
			return;
		}
		//오브젝트를 만들어서 넘겨준다? -> 인스터싱 오브젝트들
		for (int i = 0; i < m_nCntInstance; ++i)
		{
			XMFLOAT4X4 xmf4x4WorldMatrix = Matrix4x4::Transpose(m_pxmf4x4InstanceTransformMatrix[i]);
			
			//이거 만들때 m_pxmf4x4InstanceTransformMatrix에 대한 인덱스를 알아야 할듯 그래야 해당 오브젝트에 대해서만 문이 회전한다던지 수행시킬수있다.
			CreateInstanceObjectInfo(m_pstrMeshName, xmf4x4WorldMatrix);
		}
	}
}

void CInstanceStandardMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[6] = { m_d3dVertexBufferView, m_d3dUV0BufferView, m_d3dNormalBufferView, m_d3dTangentBufferView, m_d3dBiTangentBufferView, m_d3dInstanceTransformMatrixBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 6, pVertexBufferViews);
}

void CInstanceStandardMesh::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	shared_ptr<CInstanceObject> pOriginInstance = m_pOriginInstance.lock();
	int i = 0;
	for (auto& object : pOriginInstance->m_vInstanceObjectInfo)
	{	 // 0312 CJI - 정적인객체들까지 계속 갱신 해줄 필요가 있을까 싶음.(최적화) -> 완료
		if (object->IsStatic())
		{
			return; // [0528 CJI] 첫 오브젝트가 정적이면 이 후의 객체들도 정적인 객체들일것.
		}
		m_pxmf4x4InstanceTransformMatrix[i++] = Matrix4x4::Transpose(object->m_xmf4x4World);
	}

	UINT8* pBufferDataBegin = NULL;
	m_pd3dInstanceTransformMatrixBuffer->Map(0, NULL, (void**)&pBufferDataBegin);
	memcpy(pBufferDataBegin, m_pxmf4x4InstanceTransformMatrix, sizeof(XMFLOAT4X4) * m_nCntInstance);
	m_pd3dInstanceTransformMatrixBuffer->Unmap(0, NULL);
}

void CInstanceStandardMesh::CreateInstanceObjectInfo(char* pstrMeshName, XMFLOAT4X4& xmf4x4WorldMatrix)
{
	shared_ptr<CInstanceObject> pOriginInstance = m_pOriginInstance.lock();
	if (!pOriginInstance)
	{
		return;
	}

	shared_ptr<CGameObject> pInstanceObjectInfo;

	if (!strcmp(pstrMeshName, "Door_1"))
	{
		pInstanceObjectInfo = make_shared<CDoorObject>(m_pstrMeshName, xmf4x4WorldMatrix, this, pOriginInstance);
		pInstanceObjectInfo->SetStatic(false);
		pOriginInstance->m_vInstanceObjectInfo.push_back(pInstanceObjectInfo);
		size_t nLastIndex = pOriginInstance->m_vInstanceObjectInfo.size() - 1;
		g_collisionManager.AddCollisionObject(pOriginInstance->m_vInstanceObjectInfo[nLastIndex]);
		pOriginInstance->AddInstanceNumber();
	}
	else if (!strcmp(pstrMeshName, "Drawer_1") || !strcmp(pstrMeshName, "Drawer_2"))
	{
		pInstanceObjectInfo = make_shared<CDrawerObject>(m_pstrMeshName, xmf4x4WorldMatrix, this, pOriginInstance);
		pInstanceObjectInfo->SetStatic(false);
		pOriginInstance->m_vInstanceObjectInfo.push_back(pInstanceObjectInfo);
		size_t nLastIndex = pOriginInstance->m_vInstanceObjectInfo.size() - 1;
		g_collisionManager.AddCollisionObject(pOriginInstance->m_vInstanceObjectInfo[nLastIndex]);
	}
	else if (!strcmp(pstrMeshName, "Door1"))
	{
		pInstanceObjectInfo = make_shared<CElevatorDoorObject>(m_pstrMeshName, xmf4x4WorldMatrix, this, pOriginInstance);
		pInstanceObjectInfo->SetStatic(false);
		pOriginInstance->m_vInstanceObjectInfo.push_back(pInstanceObjectInfo);
		size_t nLastIndex = pOriginInstance->m_vInstanceObjectInfo.size() - 1;
		g_collisionManager.AddCollisionObject(pOriginInstance->m_vInstanceObjectInfo[nLastIndex]);
	}
	//else if (!strcmp(pstrMeshName, "Emergency_Handle"))
	//{
	//	pInstanceObjectInfo = make_shared<CElevatorDoorObject>(m_pstrMeshName, xmf4x4WorldMatrix, this);
	//	pInstanceObjectInfo->SetStatic(false);
	//	pOriginInstance->m_vInstanceObjectInfo.push_back(pInstanceObjectInfo);
	//	size_t nLastIndex = pOriginInstance->m_vInstanceObjectInfo.size() - 1;
	//	g_collisionManager.AddCollisionObject(pOriginInstance->m_vInstanceObjectInfo[nLastIndex]);
	//}
	else if (!strcmp(pstrMeshName, "Laboratory_Wall_1_Corner_1") || !strcmp(pstrMeshName, "Laboratory_Wall_1_Corner_2"))
	{
		pInstanceObjectInfo = make_shared<CEnvironmentObject>(m_pstrMeshName, xmf4x4WorldMatrix, this);
		pOriginInstance->m_vInstanceObjectInfo.push_back(pInstanceObjectInfo);
		size_t nLastIndex = pOriginInstance->m_vInstanceObjectInfo.size() - 1;
		g_collisionManager.AddCollisionObject(pOriginInstance->m_vInstanceObjectInfo[nLastIndex]);
	}
	else if (!strcmp(pstrMeshName, "Laboratory_Wall_1_Corner") || !strcmp(pstrMeshName, "Laboratory_Wall_1_Corner2")
		|| !strcmp(pstrMeshName, "Laboratory_Wall_1") || !strcmp(pstrMeshName, "Cube"))
	{
		pInstanceObjectInfo = make_shared<CEnvironmentObject>(m_pstrMeshName, xmf4x4WorldMatrix, this);
		pOriginInstance->m_vInstanceObjectInfo.push_back(pInstanceObjectInfo);
		size_t nLastIndex = pOriginInstance->m_vInstanceObjectInfo.size() - 1;	
		g_collisionManager.AddCollisionObject(pOriginInstance->m_vInstanceObjectInfo[nLastIndex]);
	}
	else if (!strcmp(pstrMeshName, "Laboratory_Wall_Door_1") || !strcmp(pstrMeshName, "Laboratory_Wall_Door_1_2") 
			|| !strcmp(pstrMeshName, "Laboratory_Tunnel_1") || !strcmp(pstrMeshName, "Laboratory_Table_1"))
	{
		pInstanceObjectInfo = make_shared<CEnvironmentObject>(m_pstrMeshName, xmf4x4WorldMatrix, this);
		pOriginInstance->m_vInstanceObjectInfo.push_back(pInstanceObjectInfo);
		size_t nLastIndex = pOriginInstance->m_vInstanceObjectInfo.size() - 1;
		g_collisionManager.AddCollisionObject(pOriginInstance->m_vInstanceObjectInfo[nLastIndex]);
	}
	else if (!strcmp(pstrMeshName, "Biological_Capsule_1") || !strcmp(pstrMeshName, "Laboratory_Stool_1"))
	{
		pInstanceObjectInfo = make_shared<CEnvironmentObject>(m_pstrMeshName, xmf4x4WorldMatrix, this);
		pOriginInstance->m_vInstanceObjectInfo.push_back(pInstanceObjectInfo);
		size_t nLastIndex = pOriginInstance->m_vInstanceObjectInfo.size() - 1;
		g_collisionManager.AddCollisionObject(pOriginInstance->m_vInstanceObjectInfo[nLastIndex]);
	}
	else if (!strcmp(pstrMeshName, "Laboratory_Tunnel_1_Stairs") || !strcmp(pstrMeshName, "Laboratory_Desk_Drawers_1"))
	{
		pInstanceObjectInfo = make_shared<CEnvironmentObject>(m_pstrMeshName, xmf4x4WorldMatrix, this);
		pOriginInstance->m_vInstanceObjectInfo.push_back(pInstanceObjectInfo);
		size_t nLastIndex = pOriginInstance->m_vInstanceObjectInfo.size() - 1;
		g_collisionManager.AddCollisionObject(pOriginInstance->m_vInstanceObjectInfo[nLastIndex]);
	}
	else if (!strcmp(pstrMeshName, "SM_Prop_Vents_Straight_01") || !strcmp(pstrMeshName, "SM_Prop_Crate_01")
		|| !strcmp(pstrMeshName, "SM_Prop_Pipe_Curve_02") || !strcmp(pstrMeshName, "SM_Prop_Billboard_Roof_01")
		|| !strcmp(pstrMeshName, "SM_Prop_Roof_Aircon_03") || !strcmp(pstrMeshName, "SM_Prop_Vents_End_01")
		|| !strcmp(pstrMeshName, "SM_Prop_ShopInterior_Table_01") || !strcmp(pstrMeshName, "SM_Prop_Couch_01")
		|| !strcmp(pstrMeshName, "SM_Prop_PotPlant_02") || !strcmp(pstrMeshName, "Table1of10"))
	{
		pInstanceObjectInfo = make_shared<CEnvironmentObject>(m_pstrMeshName, xmf4x4WorldMatrix, this);
		pOriginInstance->m_vInstanceObjectInfo.push_back(pInstanceObjectInfo);
		size_t nLastIndex = pOriginInstance->m_vInstanceObjectInfo.size() - 1;
		g_collisionManager.AddCollisionObject(pOriginInstance->m_vInstanceObjectInfo[nLastIndex]);
	}
	else
	{
		static int noneboxCount = 0;
		if (!strcmp(pstrMeshName, "Laboratory_Floor_1")) {
			int x = 0;
		}
		pInstanceObjectInfo = make_shared<CGameObject>(m_pstrMeshName, xmf4x4WorldMatrix, this);
		pOriginInstance->m_vInstanceObjectInfo.push_back(pInstanceObjectInfo);
		//pGameObject->m_vInstanceObjectInfo.push_back(CGameObject(m_pstrMeshName, xmf4x4WorldMatrix, this));
		size_t nLastIndex = pOriginInstance->m_vInstanceObjectInfo.size() - 1;
		g_collisionManager.AddNonCollisionObject(pOriginInstance->m_vInstanceObjectInfo[nLastIndex]); // 서버와는 상관없는 객체들
	}
	pInstanceObjectInfo->SetInstance(true);
	//pInstanceObjectInfo->
	//m_vOOBBs
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkinnedMesh::CSkinnedMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CStandardMesh(pd3dDevice, pd3dCommandList)
{
}

CSkinnedMesh::~CSkinnedMesh()
{
	if (m_pxmn4BoneIndices) delete[] m_pxmn4BoneIndices;
	if (m_pxmf4BoneWeights) delete[] m_pxmf4BoneWeights;

	//if (m_vpSkinningBoneFrameCaches) delete[] m_ppSkinningBoneFrameCaches;
	if (m_ppstrSkinningBoneNames) delete[] m_ppstrSkinningBoneNames;

	if (m_pxmf4x4BindPoseBoneOffsets) delete[] m_pxmf4x4BindPoseBoneOffsets;
	if (m_pd3dcbBindPoseBoneOffsets) m_pd3dcbBindPoseBoneOffsets->Release();

	if (m_pd3dBoneIndexBuffer) m_pd3dBoneIndexBuffer->Release();
	if (m_pd3dBoneWeightBuffer) m_pd3dBoneWeightBuffer->Release();

	ReleaseShaderVariables();
}

void CSkinnedMesh::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255); //256의 배수
	m_pd3dcbBindPoseBoneOffsets = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbBindPoseBoneOffsets->Map(0, NULL, (void**)&m_pcbxmf4x4MappedBindPoseBoneOffsets);

	for (int i = 0; i < m_nSkinningBones; i++)
	{
		XMStoreFloat4x4(&m_pcbxmf4x4MappedBindPoseBoneOffsets[i], XMMatrixTranspose(XMLoadFloat4x4(&m_pxmf4x4BindPoseBoneOffsets[i])));
	}

	m_d3dCbvBindPoseBoneOffsetsGPUDescriptorHandle = CScene::CreateConstantBufferViews(pd3dDevice, 1, m_pd3dcbBindPoseBoneOffsets.Get(), ncbElementBytes);
}

void CSkinnedMesh::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pd3dcbBindPoseBoneOffsets)
	{
		//D3D12_GPU_VIRTUAL_ADDRESS d3dcbBoneOffsetsGpuVirtualAddress = m_pd3dcbBindPoseBoneOffsets->GetGPUVirtualAddress();
		//pd3dCommandList->SetGraphicsRootConstantBufferView(8, d3dcbBoneOffsetsGpuVirtualAddress);
		pd3dCommandList->SetGraphicsRootDescriptorTable(8, m_d3dCbvBindPoseBoneOffsetsGPUDescriptorHandle);  //Skinned Bone Offsets
	}

	if (m_pd3dcbSkinningBoneTransforms)
	{
		//D3D12_GPU_VIRTUAL_ADDRESS d3dcbBoneTransformsGpuVirtualAddress = m_pd3dcbSkinningBoneTransforms->GetGPUVirtualAddress();
		//pd3dCommandList->SetGraphicsRootConstantBufferView(9, d3dcbBoneTransformsGpuVirtualAddress); 
		pd3dCommandList->SetGraphicsRootDescriptorTable(9, m_d3dCbvSkinningBoneTransformsGPUDescriptorHandle); //Skinned Bone Transforms
		for (int j = 0; j < m_nSkinningBones; j++)
		{
			shared_ptr<CGameObject> pSkinningBoneFrameCache = m_vpSkinningBoneFrameCaches[j].lock();

			XMStoreFloat4x4(&m_pcbxmf4x4MappedSkinningBoneTransforms[j], XMMatrixTranspose(XMLoadFloat4x4(&pSkinningBoneFrameCache->m_xmf4x4World)));
		}
	}
}

void CSkinnedMesh::ReleaseShaderVariables()
{
}

void CSkinnedMesh::ReleaseUploadBuffers()
{
	CStandardMesh::ReleaseUploadBuffers();

	if (m_pd3dBoneIndexUploadBuffer.Get()) m_pd3dBoneIndexUploadBuffer.Reset();

	if (m_pd3dBoneWeightUploadBuffer.Get()) m_pd3dBoneWeightUploadBuffer.Reset();

}

void CSkinnedMesh::PrepareSkinning(const shared_ptr<CGameObject>& pModelRootObject)
{
	for (int j = 0; j < m_nSkinningBones; j++)
	{
		m_vpSkinningBoneFrameCaches[j] = pModelRootObject->FindFrame(m_ppstrSkinningBoneNames[j]);
	}
}

void CSkinnedMesh::LoadSkinInfoFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile)
{
	char pstrToken[128] = { '\0' };
	UINT nReads = 0;

	::ReadStringFromFile(pInFile, m_pstrMeshName);

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<BonesPerVertex>:"))
		{
			m_nBonesPerVertex = ::ReadIntegerFromFile(pInFile);
		}
		else if (!strcmp(pstrToken, "<Bounds>:"))
		{
			XMFLOAT3 xmf3AABBCenter, xmf3AABBExtents;
			nReads = (UINT)::fread(&xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
			//m_vOOBBs.push_back(BoundingOrientedBox(xmf3AABBCenter, xmf3AABBExtents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)));
			//nReads = (UINT)::fread(&m_xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			//nReads = (UINT)::fread(&m_xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
			//m_OOBB = BoundingOrientedBox(m_xmf3AABBCenter, m_xmf3AABBExtents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		}
		else if (!strcmp(pstrToken, "<BoxColliders>:"))
		{
			int nBoxColliders = 0;
			nReads = fread(&nBoxColliders, sizeof(int), 1, pInFile);
			m_vOOBBs.reserve(nBoxColliders);
		}
		else if (!strcmp(pstrToken, "<Bound>:"))
		{
			int nIndex = 0;
			XMFLOAT3 xmf3AABBCenter, xmf3AABBExtents;
			nReads = fread(&nIndex, sizeof(int), 1, pInFile);
			nReads = (UINT)::fread(&xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
			m_vOOBBs.emplace_back(xmf3AABBCenter, xmf3AABBExtents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		}
		else if (!strcmp(pstrToken, "<BoneNames>:"))
		{
			m_nSkinningBones = ::ReadIntegerFromFile(pInFile);
			if (m_nSkinningBones > 0)
			{
				m_vpSkinningBoneFrameCaches.reserve(m_nSkinningBones);

				m_ppstrSkinningBoneNames = new char[m_nSkinningBones][128];
				//m_vpSkinningBoneFrameCaches = new CGameObject * [m_nSkinningBones];
				for (int i = 0; i < m_nSkinningBones; i++)
				{
					m_vpSkinningBoneFrameCaches.emplace_back();
					::ReadStringFromFile(pInFile, m_ppstrSkinningBoneNames[i]);
					//m_vpSkinningBoneFrameCaches[i] = NULL;
				}
			}
		}
		else if (!strcmp(pstrToken, "<BoneOffsets>:"))
		{
			m_nSkinningBones = ::ReadIntegerFromFile(pInFile);
			if (m_nSkinningBones > 0)
			{
				m_pxmf4x4BindPoseBoneOffsets = new XMFLOAT4X4[m_nSkinningBones];
				nReads = (UINT)::fread(m_pxmf4x4BindPoseBoneOffsets, sizeof(XMFLOAT4X4), m_nSkinningBones, pInFile);

				CreateShaderVariables(pd3dDevice, pd3dCommandList);
			}
		}
		else if (!strcmp(pstrToken, "<BoneIndices>:"))
		{
			m_nType |= VERTEXT_BONE_INDEX_WEIGHT;

			m_nVertices = ::ReadIntegerFromFile(pInFile);
			if (m_nVertices > 0)
			{
				m_pxmn4BoneIndices = new XMINT4[m_nVertices];

				nReads = (UINT)::fread(m_pxmn4BoneIndices, sizeof(XMINT4), m_nVertices, pInFile);
				m_pd3dBoneIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmn4BoneIndices, sizeof(XMINT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBoneIndexUploadBuffer);

				m_d3dBoneIndexBufferView.BufferLocation = m_pd3dBoneIndexBuffer->GetGPUVirtualAddress();
				m_d3dBoneIndexBufferView.StrideInBytes = sizeof(XMINT4);
				m_d3dBoneIndexBufferView.SizeInBytes = sizeof(XMINT4) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<BoneWeights>:")) // 테스트 파일이 이런식으로 불러오도록 저장됨.
		{
			//m_nType |= VERTEXT_BONE_INDEX_WEIGHT;

			//nReads = (UINT)::fread(&m_nVertices, sizeof(int), 1, pInFile);
			//if (m_nVertices > 0)
			//{
			//	m_pxmn4BoneIndices = new XMINT4[m_nVertices];
			//	m_pxmf4BoneWeights = new XMFLOAT4[m_nVertices];

			//	nReads = (UINT)::fread(m_pxmn4BoneIndices, sizeof(XMINT4), m_nVertices, pInFile);
			//	m_pd3dBoneIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmn4BoneIndices, sizeof(XMINT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBoneIndexUploadBuffer);

			//	m_d3dBoneIndexBufferView.BufferLocation = m_pd3dBoneIndexBuffer->GetGPUVirtualAddress();
			//	m_d3dBoneIndexBufferView.StrideInBytes = sizeof(XMINT4);
			//	m_d3dBoneIndexBufferView.SizeInBytes = sizeof(XMINT4) * m_nVertices;

			//	nReads = (UINT)::fread(m_pxmf4BoneWeights, sizeof(XMFLOAT4), m_nVertices, pInFile);
			//	m_pd3dBoneWeightBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf4BoneWeights, sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBoneWeightUploadBuffer);

			//	m_d3dBoneWeightBufferView.BufferLocation = m_pd3dBoneWeightBuffer->GetGPUVirtualAddress();
			//	m_d3dBoneWeightBufferView.StrideInBytes = sizeof(XMFLOAT4);
			//	m_d3dBoneWeightBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;
			//}

			m_nType |= VERTEXT_BONE_INDEX_WEIGHT;

			m_nVertices = ::ReadIntegerFromFile(pInFile);
			if (m_nVertices > 0)
			{
				m_pxmf4BoneWeights = new XMFLOAT4[m_nVertices];

				nReads = (UINT)::fread(m_pxmf4BoneWeights, sizeof(XMFLOAT4), m_nVertices, pInFile);
				m_pd3dBoneWeightBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf4BoneWeights, sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBoneWeightUploadBuffer);

				m_d3dBoneWeightBufferView.BufferLocation = m_pd3dBoneWeightBuffer->GetGPUVirtualAddress();
				m_d3dBoneWeightBufferView.StrideInBytes = sizeof(XMFLOAT4);
				m_d3dBoneWeightBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "</SkinningInfo>"))
		{
			break;
		}
	}
}

void CSkinnedMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[7] = { m_d3dVertexBufferView, m_d3dUV0BufferView, m_d3dNormalBufferView, m_d3dTangentBufferView, m_d3dBiTangentBufferView, m_d3dBoneIndexBufferView, m_d3dBoneWeightBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 7, pVertexBufferViews);
}

