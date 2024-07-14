#pragma once
#include "stdafx.h"
#include "ParticleObject.h"



void CParticleObject::SetMesh(const shared_ptr<CMesh>& pMesh)
{
	m_pParticleMesh = reinterpret_pointer_cast<CParticleMesh>(pMesh);
}


void CSOParticleObject::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pParticleMesh) {
		pd3dCommandList->SetPipelineState(m_vpRefPSO[0].Get());
		m_pParticleMesh->SORenderTask(pd3dCommandList);

		pd3dCommandList->SetPipelineState(m_vpRefPSO[1].Get());
		m_pParticleMesh->Render(pd3dCommandList);
	}
}

void CSOParticleObject::ReadByteTask()
{
	if (m_pParticleMesh) {
		m_pParticleMesh->ReadByteTask();
	}
}

void CGSParticleObject::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pParticleMesh) {
		pd3dCommandList->SetPipelineState(m_vpRefPSO[1].Get());
		//m_pShader->UpdateShaderVariable(pd3dCommandList, &m_xmf4x4world);
		m_vpMaterials[0]->UpdateShaderVariable(pd3dCommandList, nullptr);
		m_pParticleMesh->Render(pd3dCommandList);
	}
}

void CGSParticleObject::Update(float fcurTime)
{
	if (m_pParticleMesh) {
		m_pParticleMesh->Update(fcurTime);
	}
}
