#pragma once
#include "Mesh.h"
//position, uv, normal 만 가지고 렌더링하는 메쉬

class TextureBlendMesh : public CStandardMesh
{
public:
	TextureBlendMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	~TextureBlendMesh();

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList);
};

