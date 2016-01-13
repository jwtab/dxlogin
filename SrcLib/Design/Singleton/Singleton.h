
#ifndef SINGLETON_H_H_
#define SINGLETON_H_H_

/*
	设计模式
		单件模式.
*/

/************************************************************/

/************************************************************/
#include "../Lock/Lock.h"

/************************************************************/
template<class T>
class CSingleton
{
private:	
	CSingleton(){}	
	CSingleton(const CSingleton &){} 
	CSingleton& operator = (CSingleton &){}	

public:	
	static T * Instance();  
	static void DestoryInstance();

private:
	static T * m_pInstance;
	static CLock m_lock;
};

/************************************************************/
template<class T>
T * CSingleton<T>::m_pInstance = NULL;

template<class T>
CLock CSingleton<T>::m_lock;

template<class T>
T * CSingleton<T>::Instance()
{
	if (NULL == m_pInstance)
	{
		m_lock.Lock();

		if (NULL == m_pInstance)
		{
			m_pInstance = new T;
		}		

		m_lock.UnLock();
	}

	return m_pInstance;
}

template<class T>
void CSingleton<T>::DestoryInstance()
{
	if (NULL != m_pInstance)
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

#endif