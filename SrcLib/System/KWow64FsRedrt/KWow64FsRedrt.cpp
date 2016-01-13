
#include "stdafx.h"

#include "KWow64FsRedrt.h"

CKWow64FsRedrt::CKWow64FsRedrt()  
{  
	m_pfWow64DisableWow64FsRedirection = NULL;  
	m_pfWow64RevertWow64FsRedirection = NULL; 

	m_pOldValue = NULL;  
	m_bCloseRedirection = FALSE;  

	m_hModKrl = NULL;  
}  

CKWow64FsRedrt::~CKWow64FsRedrt()  
{  
	if (NULL != m_hModKrl)  
	{  
		FreeLibrary(m_hModKrl);  
		m_hModKrl = NULL;  
	}  
}  

BOOL CKWow64FsRedrt::Init()  
{  
	BOOL bRet = FALSE;  
	m_hModKrl  =  LoadLibraryW(L"Kernel32.dll");  
	if (NULL == m_hModKrl)  
	{ 
		goto Exit0;  
	}  

	m_pfWow64DisableWow64FsRedirection  =  (WOW64DISABLEWOW64FSREDIRECTION)GetProcAddress(m_hModKrl,"Wow64DisableWow64FsRedirection");   
	m_pfWow64RevertWow64FsRedirection  =    (WOW64REVERTWOW64FSREDIRECTION)GetProcAddress(m_hModKrl,"Wow64RevertWow64FsRedirection");  
	if (!m_pfWow64DisableWow64FsRedirection || !m_pfWow64RevertWow64FsRedirection)  
	{  
		goto Exit0;  
	}  

	bRet = TRUE;  

Exit0:  
	return bRet;  
}  

BOOL CKWow64FsRedrt::Close()  
{  
	int bRet = 0;  

	if (m_bCloseRedirection)  
	{  
		return TRUE;  
	}  

	if (!m_pfWow64DisableWow64FsRedirection)  
	{
		return FALSE;  
	}

	bRet = m_pfWow64DisableWow64FsRedirection(&m_pOldValue);  
	if (bRet)  
	{  
		m_bCloseRedirection = TRUE;  
		return TRUE;  
	}  

	return FALSE;  
}  

BOOL CKWow64FsRedrt::Open()  
{  
	BOOL bRet = 0;  

	if (!m_bCloseRedirection)  
	{  
		return TRUE;  
	}  

	if (!m_pfWow64RevertWow64FsRedirection)  
	{
		return FALSE;  
	}

	bRet = m_pfWow64RevertWow64FsRedirection(m_pOldValue);  
	if (bRet)  
	{  
		m_bCloseRedirection = FALSE;  
		return TRUE;  
	}  

	return FALSE;  
}  