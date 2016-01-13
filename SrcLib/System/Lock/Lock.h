
#ifndef LOCK_H_H_
#define LOCK_H_H_

/*
	线程安全,临界区资源锁.
*/

/************************************************************/


/************************************************************/
#include <Windows.h>

/************************************************************/
class CLock
{
public:
	CLock();
	~CLock();

public:
	bool Lock();
	bool UnLock();

private:
	void InitCriticalSection();
	void UnInitCriticalSection();

private:
	CRITICAL_SECTION m_cs;
	bool             m_bCreated;
};

#endif