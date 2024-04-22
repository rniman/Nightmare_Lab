#include "stdafx.h"
#include "TextureBlendMesh.h"

TextureBlendMesh::TextureBlendMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
	: CStandardMesh(pd3dDevice, pd3dCommandList)
{

}

TextureBlendMesh::~TextureBlendMesh()
{
}

void TextureBlendMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[3] = { m_d3dVertexBufferView, m_d3dUV0BufferView, m_d3dNormalBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 3, pVertexBufferViews);
}
