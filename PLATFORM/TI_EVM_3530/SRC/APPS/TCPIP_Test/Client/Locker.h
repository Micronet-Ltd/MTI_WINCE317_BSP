#pragma once

class Locker
{
public:
	Locker(CRITICAL_SECTION &criticalSection) : m_CriticalSection(criticalSection) { EnterCriticalSection(&m_CriticalSection); };
	~Locker() { LeaveCriticalSection(&m_CriticalSection); };

protected:
	CRITICAL_SECTION m_CriticalSection;
};
