
#ifndef __KWOW64FSREDRT_H__  
#define __KWOW64FSREDRT_H__  

#include <Windows.h>

class CKWow64FsRedrt  
{  
public:  
	CKWow64FsRedrt();  
	~CKWow64FsRedrt();  

	BOOL Init();  
	BOOL Open();  
	BOOL Close();  

private:  
	typedef BOOL (__stdcall *WOW64DISABLEWOW64FSREDIRECTION)(PVOID*);  
	typedef BOOL (__stdcall *WOW64REVERTWOW64FSREDIRECTION)(PVOID);   
	
	WOW64DISABLEWOW64FSREDIRECTION  m_pfWow64DisableWow64FsRedirection;  
	WOW64REVERTWOW64FSREDIRECTION  m_pfWow64RevertWow64FsRedirection;  

	PVOID               m_pOldValue;  
	BOOL                m_bCloseRedirection;  
	HMODULE         m_hModKrl;  
};  

#endif /* __KWOW64FSREDRT_H__ */  