
#include "stdafx.h"

#include "Lock.h"

CLock::CLock()
{
	m_bCreated = false;

	InitCriticalSection();
}

CLock::~CLock()
{
	UnInitCriticalSection();
}

bool CLock::Lock()
{
	if (!m_bCreated)
	{
		InitCriticalSection();
	}

	if (m_bCreated)
	{
		EnterCriticalSection(&m_cs);

		return true;
	}

	return false;
}

bool CLock::UnLock()
{
	if (m_bCreated)
	{
		LeaveCriticalSection(&m_cs);

		return true;
	}

	return false;
}

void CLock::InitCriticalSection()
{
	InitializeCriticalSection(&m_cs);
	m_bCreated = true;
}

void CLock::UnInitCriticalSection()
{
	DeleteCriticalSection(&m_cs);
	m_bCreated = false;
}