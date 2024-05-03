#include "stdafx.h"
#include "GameFramework.h"
#include "Object.h"
#include "Player.h"
#include "Scene.h"
#include "TextureBlendObject.h"

TextureBlendObject::TextureBlendObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,shared_ptr<CGameObject> object,shared_ptr<CPlayer> player)
	: CGameObject(pd3dDevice, pd3dCommandList)
{
	m_pMesh = object->m_pMesh;
	m_vpMaterials = object->m_vpMaterials;
	m_nMaterials = object->m_nMaterials;
	memcpy(&m_xmf4x4ToParent, &object->m_xmf4x4ToParent, sizeof(XMFLOAT4X4));
	memcpy(&m_xmf4x4World, &object->m_xmf4x4World, sizeof(XMFLOAT4X4));

	m_pPlayer = player;

	UINT ncbElementBytes = ((sizeof(FrameTimeInfo) + 255) & ~255); //256의 배수
	m_pd3dcbTime = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbTime->Map(0, NULL, (void**)&m_pcbMappedTime);
	m_pcbMappedTime->time = 0.0f;
	m_pcbMappedTime->localTime = 0.0f;
	m_pcbMappedTime->usePattern = -1.0f;
	m_d3dTimeCbvGPUDescriptorHandle = CScene::CreateConstantBufferViews(pd3dDevice, 1, m_pd3dcbTime.Get(), ncbElementBytes);

	m_fLocalTime = 10.0f; // 실행되려면 1.0f 이하여야함 
}

TextureBlendObject::~TextureBlendObject()
{
}

void TextureBlendObject::Animate(float fElapsedTime)
{
	if (m_fLocalTime > 1.0f) {
		return;
	}
	m_fLocalTime += fElapsedTime;

	XMFLOAT3 c_position = m_pPlayer->GetCamera()->GetPosition();

	SetLookAt(c_position);
}

void TextureBlendObject::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_fLocalTime > 1.0f) {
		return;
	}

	m_pcbMappedTime->localTime = m_fLocalTime;

	pd3dCommandList->SetGraphicsRootDescriptorTable(12, m_d3dTimeCbvGPUDescriptorHandle);

	CGameObject::Render(pd3dCommandList);
}
