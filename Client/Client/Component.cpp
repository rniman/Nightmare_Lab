#include "stdafx.h"
#include "Component.h"
#include "Timer.h"

ComponentTimeOnOff::ComponentTimeOnOff()
{
	m_iComponentType = TIMEONOFF;

	m_bSwitch = nullptr;

	m_fSetTime = 0.0f;
	m_fTime = 0.0f;
}

void ComponentTimeOnOff::Init()
{
	m_fTime = m_fSetTime;
}

void ComponentTimeOnOff::Update()
{
	if (*m_bSwitch == false) {
		return;
	}

	m_fTime -= gGameTimer.GetTimeElapsed();

	if (m_fTime <= 0.0f) {
		m_fTime = m_fSetTime;
		*m_bSwitch = false;
	}
}

void ComponentTimeOnOff::RegisterVariable(void* variable)
{
	m_bSwitch = (bool*)variable;
	//*m_bSwitch = true;
}

void ComponentTimeOnOff::SetLimitTime(float value)
{
	m_fSetTime = value;
	m_fTime = m_fSetTime;
}
