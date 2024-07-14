#pragma once
#include "stdafx.h"
#include "SharedObject.h"
#include "Timer.h"

void SharedObject::EnableItemGetParticle(const shared_ptr<CGameObject>& object)
{
	XMFLOAT3 pos = object->GetPosition();
	m_vParticleObjects[CParticleMesh::SPARK]->SetParticleInsEnable(-1, true, gGameTimer.GetTotalTime(), pos);
}

void SharedObject::AddParticle(CParticleMesh::TYPE particleType,XMFLOAT3 pos)
{
	m_vParticleObjects[CParticleMesh::ATTACK]->AddParticle(pos);
}