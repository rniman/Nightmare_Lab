#pragma once
#include "ParticleObject.h"

#define sharedobject SharedObject::GetInstance()

// 모든 객체가 접근 가능한 객체를 갖도록한다.
// 객체들에게 중복되는 데이터를 갖도록 하지 않고 sharedobject를 통해서 데이터를 접근한다.

class SharedObject
{
private:
	SharedObject() {}
public:
	static SharedObject& GetInstance() {
		static SharedObject instance;
		return instance;
	}

	void EnableItemGetParticle(const shared_ptr<CGameObject>& object);
	void AddParticle(CParticleMesh::TYPE particleType, XMFLOAT3 pos);

	vector<shared_ptr<CParticleObject>> m_vParticleObjects;
};

