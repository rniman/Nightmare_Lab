#pragma once
#include "stdafx.h"
#include "ParticleMesh.h"
#include "Timer.h"


void CParticleMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW views[] = { m_d3dVertexBufferView ,m_d3dInstanceParticleBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, _countof(views), views);
	pd3dCommandList->DrawInstanced(m_nVertices, draw_instance_count, m_nOffset, 0);
}

void CParticleMesh::CreateBufferViewResource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pVertices, UINT nVertices, UINT nStride)
{
	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices,
		nStride * nVertices, D3D12_HEAP_TYPE_UPLOAD,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = nStride;
	m_d3dVertexBufferView.SizeInBytes = nStride * nVertices;

	for (int i = 0; i < PARITICLE_INSTANCE_COUNT;++i) {
		m_particleInsVertex[i].id = i;
		m_particleInsVertex[i].start_position = XMFLOAT3(0.0f, 0.f, 0.f);
		m_particleInsVertex[i].start_Time = 0.0f;
	}
	m_pd3dInstanceParticleBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, &m_particleInsVertex,
		sizeof(ParticleInstanceVertex) * PARITICLE_INSTANCE_COUNT, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);

	m_d3dInstanceParticleBufferView.BufferLocation = m_pd3dInstanceParticleBuffer->GetGPUVirtualAddress();
	m_d3dInstanceParticleBufferView.StrideInBytes = sizeof(ParticleInstanceVertex);
	m_d3dInstanceParticleBufferView.SizeInBytes = sizeof(ParticleInstanceVertex) * PARITICLE_INSTANCE_COUNT;
}

int CParticleMesh::SetParticleInsEnable(int id, bool val, float fCurTime, XMFLOAT3& pos)
{
	int pi = id;
	if (pi == -1) {
		for (int i = 0;i < m_ParticleInsInfos.size();++i) {
			if (!m_ParticleInsInfos[i].b_enabled) {
				pi = i;
				break;
			}
		}
		if (pi == -1) {
			return -1;
		}
	}
	m_ParticleInsInfos[pi].b_enabled = val;
	m_ParticleInsInfos[pi].startTime = fCurTime;

	void* pData = nullptr;
	m_pd3dInstanceParticleBuffer->Map(0, nullptr, (void**)&pData);
	ParticleInstanceVertex* ins_vertex = reinterpret_cast<ParticleInstanceVertex*>(pData);
	ins_vertex[pi].start_position = pos;
	ins_vertex[pi].start_Time = fCurTime;
	m_pd3dInstanceParticleBuffer->Unmap(0, nullptr);

	return pi;
}

void CParticleMesh::SetParticlePosition(int id, XMFLOAT3& pos)
{
	void* pData = nullptr;
	m_pd3dInstanceParticleBuffer->Map(0, nullptr, (void**)&pData);
	ParticleInstanceVertex* ins_vertex = reinterpret_cast<ParticleInstanceVertex*>(pData);
	ins_vertex[id].start_position = pos;
	m_pd3dInstanceParticleBuffer->Unmap(0, nullptr);
}

void CParticleMesh::UpdateUploadBuffer(int newCount, int oldCount, std::vector<int>& v_enable)
{
	if (newCount != oldCount) { // 비활성화된 파티클이 존재한다면 인스턴스 객체 맨 앞으로
		void* pData = nullptr;
		m_pd3dInstanceParticleBuffer->Map(0, nullptr, (void**)&pData);
		ParticleInstanceVertex* ins_vertex = reinterpret_cast<ParticleInstanceVertex*>(pData);

		for (auto& id : v_enable) {
			m_ParticleInsInfos[id].b_enabled = false;
			for (int i = 0; i < m_ParticleInsInfos.size(); ++i) {
				if (m_ParticleInsInfos[i].b_enabled) { // 활성화되어 있다면 이값은 갱신x
					continue;
				}
				m_ParticleInsInfos[i].b_enabled = true;
				m_ParticleInsInfos[i].startTime = m_ParticleInsInfos[id].startTime;

				ins_vertex[i].start_position = ins_vertex[id].start_position;
				ins_vertex[i].start_Time = ins_vertex[id].start_Time;
				break;
			}
		}
		m_pd3dInstanceParticleBuffer->Unmap(0, nullptr);
	}
}

void CSOParticleMesh::CreateShaderVariable(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int maxParticle, XMFLOAT3 position)
{
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
	m_maxParticle = maxParticle;

	ParticleVertex pVertex[1];
	pVertex[0].lifeTime = 1.0f;
	pVertex[0].position = position; // 이 위치는 상대적이므로 받아오도록 바꿔야함.
	pVertex[0].type = BUBBLE;
	pVertex[0].velocity = XMFLOAT3(0.0f, 0.1f, 0.0f);
	pVertex[0].startTime = 0.0f;
	m_nStride = sizeof(ParticleVertex);

	m_nVertices = 1;

	CreateBufferViewResource(pd3dDevice, pd3dCommandList, pVertex, m_nVertices, m_nStride);
	draw_instance_count = 0;

	m_SOBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, (m_nStride * m_maxParticle),
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_STREAM_OUT, NULL);
	m_RenderBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, (m_nStride * m_maxParticle),
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	UINT64 nBufFilledSize = 0;
	//default
	m_pd3dBufFilledSize = ::CreateBufferResource(pd3dDevice, pd3dCommandList, &nBufFilledSize, sizeof(UINT64), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_STREAM_OUT, NULL);
	//upload
	m_pd3dInitBufFilledSize = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(UINT64), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);
	m_pd3dInitBufFilledSize->Map(0, NULL, (void**)&m_pnInitBufFilledSize);

	m_pd3dReadBufFilledSize = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(UINT64), D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST, NULL);
}

void CSOParticleMesh::SORenderTask(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if ((m_nVertices == 0) || (m_nVertices >= MAX_PARTICLE)) {
		m_bInit = true;
	}

	if (m_bInit)
	{
		m_bInit = false;

		m_nVertices = 1;

		m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
		m_d3dVertexBufferView.StrideInBytes = m_nStride;
		m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
	}
	else
	{
		m_d3dVertexBufferView.BufferLocation = m_RenderBuffer->GetGPUVirtualAddress();
		m_d3dVertexBufferView.StrideInBytes = m_nStride;
		m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
	}
	m_d3dSObufferView.BufferLocation = m_SOBuffer->GetGPUVirtualAddress();
	m_d3dSObufferView.SizeInBytes = m_nStride * m_maxParticle;
	m_d3dSObufferView.BufferFilledSizeLocation = m_pd3dBufFilledSize->GetGPUVirtualAddress();

	*m_pnInitBufFilledSize = 0;

	SynchronizeResourceTransition(pd3dCommandList, m_pd3dBufFilledSize.Get(), D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_COPY_DEST);
	pd3dCommandList->CopyResource(m_pd3dBufFilledSize.Get(), m_pd3dInitBufFilledSize.Get());
	SynchronizeResourceTransition(pd3dCommandList, m_pd3dBufFilledSize.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_STREAM_OUT);

	D3D12_STREAM_OUTPUT_BUFFER_VIEW pSOBufferViews[1] = { m_d3dSObufferView };
	pd3dCommandList->SOSetTargets(0, 1, pSOBufferViews);

	//Stream Output to m_pd3dStreamOutputBuffer
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dVertexBufferView);
	pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);

	SynchronizeResourceTransition(pd3dCommandList, m_pd3dBufFilledSize.Get(), D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_COPY_SOURCE);
	pd3dCommandList->CopyResource(m_pd3dReadBufFilledSize.Get(), m_pd3dBufFilledSize.Get());
	SynchronizeResourceTransition(pd3dCommandList, m_pd3dBufFilledSize.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_STREAM_OUT);
}

void CSOParticleMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	SynchronizeResourceTransition(pd3dCommandList, m_SOBuffer.Get(), D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	SynchronizeResourceTransition(pd3dCommandList, m_RenderBuffer.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_STREAM_OUT);
	SwapResourcePointer(m_RenderBuffer.GetAddressOf(), m_SOBuffer.GetAddressOf());

	m_d3dVertexBufferView.BufferLocation = m_RenderBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

	pd3dCommandList->SOSetTargets(0, 1, NULL);

	CParticleMesh::Render(pd3dCommandList);
}


#define DEBUG_STREAM_OUTPUT_VERTEX

void CSOParticleMesh::ReadByteTask()
{
	UINT64* pnReadBufFilledSize = NULL;
	m_pd3dReadBufFilledSize->Map(0, NULL, (void**)&pnReadBufFilledSize);
	m_nVertices = UINT(*pnReadBufFilledSize) / m_nStride;
	m_pd3dReadBufFilledSize->Unmap(0, NULL);

#ifdef DEBUG_STREAM_OUTPUT_VERTEX
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("Stream Output Vertices = %d\n"), m_nVertices);
	OutputDebugString(pstrDebug);
#endif
}

int CSOParticleMesh::SetParticleInsEnable(int id, bool val, float fCurTime, XMFLOAT3& pos)
{
	int retval = CParticleMesh::SetParticleInsEnable(id, val, fCurTime, pos);
	if (retval != -1) {
		draw_instance_count += 1;
	}
	return retval;
}

CDefaultParticleMesh::CDefaultParticleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CParticleMesh(pd3dDevice, pd3dCommandList)
{
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
	draw_instance_count = 1;
}

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<int> rdPos{ -100,100 };
std::uniform_int_distribution<int> rdSpeed{ -10,10 };
std::uniform_int_distribution<int> rdLife{ 2,5 };
std::normal_distribution<> normalDist(0.0, 0.3); // 0.0 평균 , 1.0의 표준편차. 대략 +- *3(+-3.0)에 해당하는 값이 도출됨.
std::normal_distribution<> normalDist2(5.0, 1.0);

void CDefaultParticleMesh::CreateShaderVariable(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, TYPE type, UINT nVertices)
{
	m_nVertices = nVertices;
	m_nStride = sizeof(ParticleVertex);

	for (int i = 0; i < m_nVertices;++i) {
		m_particleVertex[i].position.x = 0.0f;
		m_particleVertex[i].position.y = 0.0f;
		m_particleVertex[i].position.z = 0.0f;
		m_particleVertex[i].velocity.x = rdSpeed(gen);
		m_particleVertex[i].velocity.y = rdSpeed(gen);
		m_particleVertex[i].velocity.z = rdSpeed(gen);
		m_particleVertex[i].lifeTime = normalDist2(gen);
		m_particleVertex[i].type = type;
	}

	CreateBufferViewResource(pd3dDevice, pd3dCommandList, (void*)&m_particleVertex, m_nVertices, m_nStride);

	for (auto& infos : m_ParticleInsInfos) {
		infos.type = type;
	}
}

void CDefaultParticleMesh::Update(float fcurtime)
{
	int oldcount = draw_instance_count;
	int newcount = 0;

	std::vector<int> v_enable;
	for (int i = 0; i < PARITICLE_INSTANCE_COUNT; ++i) {
		if (!m_ParticleInsInfos[i].b_enabled) continue;

		++newcount;// 활성화 되어 있는 객체 개수
		v_enable.push_back(i);
		if (fcurtime - m_ParticleInsInfos[i].startTime > TYPE_TIME::GETITEM) {
			m_ParticleInsInfos[i].b_enabled = false;
		}
	}

	UpdateUploadBuffer(newcount, oldcount, v_enable);

	draw_instance_count = newcount;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTPParticleMesh::CTPParticleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CDefaultParticleMesh(pd3dDevice, pd3dCommandList)
{

}


void CTPParticleMesh::CreateShaderVariable(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_nVertices = 5000;
	m_nStride = sizeof(ParticleVertex);

	for (int i = 0; i < m_nVertices;++i) {
		{// position range
			m_particleVertex[i].position.x = normalDist(gen) * 10;
			m_particleVertex[i].position.y = normalDist(gen) * 10;
			m_particleVertex[i].position.z = normalDist(gen) * 10;
		}
		m_particleVertex[i].velocity.x = rdSpeed(gen);
		m_particleVertex[i].velocity.y = rdSpeed(gen);
		m_particleVertex[i].velocity.z = rdSpeed(gen);
		m_particleVertex[i].lifeTime = normalDist2(gen);
		m_particleVertex[i].type = TP;
	}

	CreateBufferViewResource(pd3dDevice, pd3dCommandList, (void*)&m_particleVertex, m_nVertices, m_nStride);

	for (auto& infos : m_ParticleInsInfos) {
		infos.b_enabled = false;
		infos.type = TP;
	}
	m_ParticleInsInfos[0].b_enabled = true;
}

void CTPParticleMesh::Update(float fcurtime)
{
	int oldcount = draw_instance_count;
	int newcount = 0;

	std::vector<int> v_enable;
	for (int i = 0; i < PARITICLE_INSTANCE_COUNT; ++i) {
		if (!m_ParticleInsInfos[i].b_enabled) continue;

		++newcount;// 활성화 되어 있는 객체 개수
		v_enable.push_back(i);
		if (fcurtime - m_ParticleInsInfos[i].startTime > TYPE_TIME::TP) {
			m_ParticleInsInfos[i].b_enabled = false;
		}
	}

	UpdateUploadBuffer(newcount, oldcount, v_enable);

	draw_instance_count = newcount;
}

CAttackParticleMesh::CAttackParticleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CDefaultParticleMesh(pd3dDevice, pd3dCommandList)
{
}

void CAttackParticleMesh::CreateShaderVariable(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_nVertices = 99999;
	m_nStride = sizeof(ParticleVertex);

	m_particleVertex[0].position.x = 0.0f;
	m_particleVertex[0].position.y = 0.0f;
	m_particleVertex[0].position.z = 0.0f;
	m_particleVertex[0].velocity.x = 0.0f;
	m_particleVertex[0].velocity.y = 0.0f;
	m_particleVertex[0].velocity.z = 0.0f;
	m_particleVertex[0].lifeTime = 2.0f;
	m_particleVertex[0].type = ATTACK;

	CreateBufferViewResource(pd3dDevice, pd3dCommandList, (void*)&m_particleVertex, m_nVertices, m_nStride);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * 1; // 생성 할때는 미리 공간을 예약한다고 생각하면 됨.

	m_nVertices = 1;

	for (auto& infos : m_ParticleInsInfos) {
		infos.b_enabled = false;
		infos.type = ATTACK;
	}
	m_ParticleInsInfos[0].b_enabled = true;
	draw_instance_count = 1; // Attack은 인스턴싱 X
}

void CAttackParticleMesh::Update(float fcurtime)
{

}

void CAttackParticleMesh::AddParticle(XMFLOAT3& pos)
{
	if (m_nVertices >= 99999) {
		m_nVertices = 0;
	}

	void* pData = nullptr;
	m_pd3dVertexBuffer->Map(0, nullptr, (void**)&pData);
	ParticleVertex* vertices = reinterpret_cast<ParticleVertex*>(pData);

	vertices[m_nVertices].lifeTime = gGameTimer.GetTotalTime() + 2.0f;
	vertices[m_nVertices].position.x = pos.x;
	vertices[m_nVertices].position.y = pos.y;//vertices[m_nVertices - 1].position.y + 1.0f;
	vertices[m_nVertices].position.z = pos.z;
	vertices[m_nVertices].velocity.x = rdSpeed(gen) * 0.3f / 10.0f;
	vertices[m_nVertices].velocity.y = 0.3f;
	vertices[m_nVertices].velocity.z = rdSpeed(gen) * 0.3f / 10.0f;
	vertices[m_nVertices].type = ATTACK;

	m_pd3dVertexBuffer->Unmap(0, nullptr);

	m_nVertices += 1; // 파티클 개수

	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
}