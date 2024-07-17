#pragma once
#include "stdafx.h"
#include "Trail.h"
#include "Timer.h"
#include "Scene.h"

Trail::Trail(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList )
{
	m_bStart = false;
	m_fUVTime = 0.0f;

	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	CreateShaderVariable(pd3dDevice, pd3dCommandList);

	testobject = new CGameObject(pd3dDevice, pd3dCommandList);
	testobject->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	testobject->m_xmf4x4ToParent._41 = 2.f;
	m_pMaterial = make_shared<CMaterial>(1);

	static shared_ptr<CTexture> pTexture;
	if (!pTexture) {
		pTexture = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
		pTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/Textures/Trail2.dds", RESOURCE_TEXTURE2D, 0);
	}
	m_pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_pMaterial->SetTexture(pTexture, 0);

	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, 0, 3); // 3 : Albedo
}

void Trail::CreateShaderVariable(ID3D12Device* pd3dDevice,ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_nVertices = 99999;
	TrailVertex* vertices = new TrailVertex[m_nVertices];

	/*vertices[0].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertices[1].position = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertices[2].position = XMFLOAT3(1.0f, 0.0f, 0.0f);

	vertices[3].position = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertices[4].position = XMFLOAT3(1.0f, 1.0f, 0.0f);
	vertices[5].position = XMFLOAT3(1.0f, 0.0f, 0.0f);*/

	m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, vertices, sizeof(TrailVertex) * m_nVertices, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_nVertices = 0;

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = sizeof(TrailVertex);
	m_d3dVertexBufferView.SizeInBytes = sizeof(TrailVertex) * m_nVertices;
}


void Trail::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pMaterial->UpdateShaderVariable(pd3dCommandList,nullptr);

	pd3dCommandList->IASetVertexBuffers(0, 1, &m_d3dVertexBufferView);

	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	pd3dCommandList->DrawInstanced(m_nVertices, 1, 0, 0);

	Update();
}

void Trail::Update()
{
	if (!m_bStart) {
		return;
	}

	if (m_nVertices + 6 >= 99999) {
		m_nVertices = 0;
		m_d3dVertexBufferView.SizeInBytes = sizeof(TrailVertex) * m_nVertices;
		return;
	}

	m_fUVTime += gGameTimer.GetTimeElapsed();
	if (m_fUVTime >= 0.65f) {
		m_fUVTime = 0.0f;
		m_bStart = false;
		return;
	}

	/*XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.0f),
		XMConvertToRadians(360.0f * gGameTimer.GetTimeElapsed()), XMConvertToRadians(0.0f));
	testobject->m_xmf4x4ToParent = Matrix4x4::Multiply(testobject->m_xmf4x4ToParent, mtxRotate);
	testobject->UpdateTransform(nullptr);

	XMFLOAT3 pos = testobject->GetPosition();
	XMFLOAT3 right = testobject->GetRight();
	XMFLOAT3 up = testobject->GetUp();*/


	float interval = 0.25f;

	if (m_nVertices == 0) {
		// 첫 1 삼각형
		FirstTrailGenerate(interval);
	}
	else {
		void* pData;
		m_pd3dVertexBuffer->Map(0, nullptr, &pData);

		auto Tv = reinterpret_cast<TrailVertex*>(pData);

		XMFLOAT3 pos = m_pObject->GetPosition();
		XMFLOAT3 right = m_pObject->GetRight();
		XMFLOAT3 up = m_pObject->GetUp();
		XMFLOAT3 temp;
		// 1 삼각형
		Tv[m_nVertices].position = Tv[m_nVertices - 3].position;//오른쪽 위 --- 3 [이전 삼각형의 왼쪽 위 3 재사용]
		Tv[m_nVertices].uv = XMFLOAT2(m_fUVTime * 2 / 3, 0.0f);
		Tv[m_nVertices].startTime = gGameTimer.GetTotalTime();
		Tv[m_nVertices + 1].position = Tv[m_nVertices - 1].position;//오른쪽 아래 --- 4 [이전 삼각형의 왼쪽아래 4 재사용]
		Tv[m_nVertices + 1].uv = XMFLOAT2(m_fUVTime * 2 / 3, 1.f);
		Tv[m_nVertices + 1].startTime = gGameTimer.GetTotalTime();
		temp = Vector3::Add(pos, right, -interval);
		temp = Vector3::Add(temp, up, interval);
		Tv[m_nVertices + 2].position = temp;//왼쪽 위 --- 5 [새로 생성하는 정점]
		Tv[m_nVertices + 2].uv = XMFLOAT2(m_fUVTime * 2 / 3, 0.f);
		Tv[m_nVertices + 2].startTime = gGameTimer.GetTotalTime();

		// 2 삼각형
		Tv[m_nVertices + 3].position = Tv[m_nVertices + 2].position;//왼쪽 위 --- 5 [5] 재사용
		Tv[m_nVertices + 3].uv = XMFLOAT2(m_fUVTime * 2 / 3, 0.f);
		Tv[m_nVertices + 3].startTime = gGameTimer.GetTotalTime();
		Tv[m_nVertices + 4].position = Tv[m_nVertices + 1].position;//오른쪽 아래 --- 4 [4] 재사용 -> 인덱스가 아니므로 사실상 다른 정점임을 유의하자.
		Tv[m_nVertices + 4].uv = XMFLOAT2(m_fUVTime * 2 / 3, 1.f);
		Tv[m_nVertices + 4].startTime = gGameTimer.GetTotalTime();
		temp = Vector3::Add(pos, right, -interval);
		temp = Vector3::Add(temp, up, -interval);
		Tv[m_nVertices + 5].position = temp;//왼쪽 아래 --- 6 [새 정점]
		Tv[m_nVertices + 5].uv = XMFLOAT2(m_fUVTime * 2 / 3, 1.f);
		Tv[m_nVertices + 5].startTime = gGameTimer.GetTotalTime();

		m_nVertices += 6; // 생성 후 증가시킨다.

		m_pd3dVertexBuffer->Unmap(0, nullptr);
		m_d3dVertexBufferView.SizeInBytes = sizeof(TrailVertex) * m_nVertices;
	}

}

void Trail::FirstTrailGenerate(float interval)
{
	void* pData;
	m_pd3dVertexBuffer->Map(0, nullptr, &pData);

	auto Tv = reinterpret_cast<TrailVertex*>(pData);

	XMFLOAT3 pos = m_pObject->GetPosition();
	XMFLOAT3 right = m_pObject->GetRight();
	XMFLOAT3 up = m_pObject->GetUp();
	XMFLOAT3 temp;

	temp = Vector3::Add(pos, right, interval);
	temp = Vector3::Add(temp, up, interval);
	Tv[m_nVertices].position = temp;//오른쪽 위 --- 1
	Tv[m_nVertices].uv = XMFLOAT2(0.f, 0.0f);
	Tv[m_nVertices].startTime = gGameTimer.GetTotalTime();
	temp = Vector3::Add(pos, right, interval);
	temp = Vector3::Add(temp, up, -interval);
	Tv[m_nVertices + 1].position = temp;//오른쪽 아래 --- 2
	Tv[m_nVertices + 1].uv = XMFLOAT2(0.f, 0.f);
	Tv[m_nVertices + 1].startTime = gGameTimer.GetTotalTime();
	temp = Vector3::Add(pos, right, -interval);
	temp = Vector3::Add(temp, up, interval);
	Tv[m_nVertices + 2].position = temp;//왼쪽 위 --- 3
	Tv[m_nVertices + 2].uv = XMFLOAT2(0.f, 0.f);
	Tv[m_nVertices + 2].startTime = gGameTimer.GetTotalTime();

	// 첫 2 삼각형
	Tv[m_nVertices + 3].position = Tv[m_nVertices + 2].position;//왼쪽 위 --- 3 [3] 재사용
	Tv[m_nVertices + 3].uv = XMFLOAT2(0.f, 0.f);
	Tv[m_nVertices + 3].startTime = gGameTimer.GetTotalTime();
	Tv[m_nVertices + 4].position = Tv[m_nVertices + 1].position;//오른쪽 아래 --- 2 [2] 재사용
	Tv[m_nVertices + 4].uv = XMFLOAT2(0.f, 0.f);
	Tv[m_nVertices + 4].startTime = gGameTimer.GetTotalTime();
	temp = Vector3::Add(pos, right, -interval);
	temp = Vector3::Add(temp, up, -interval);
	Tv[m_nVertices + 5].position = temp;//왼쪽 아래 --- 4
	Tv[m_nVertices + 5].uv = XMFLOAT2(0.f, 0.f);
	Tv[m_nVertices + 5].startTime = gGameTimer.GetTotalTime();

	m_nVertices += 6; // 생성 후 증가시킨다.

	m_pd3dVertexBuffer->Unmap(0, nullptr);
	m_d3dVertexBufferView.SizeInBytes = sizeof(TrailVertex) * m_nVertices;
}

void Trail::TrailInit()
{
	m_nVertices = 0;
	m_d3dVertexBufferView.SizeInBytes = sizeof(TrailVertex) * m_nVertices;
}

void Trail::TrailStart()
{
	if (m_bStart) {
		return;
	}
	m_bStart = true;

	FirstTrailGenerate(0.25f);
}
