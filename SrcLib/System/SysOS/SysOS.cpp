
#include "stdafx.h"

#include "SysOS.h"
using namespace SystemX;

BOOL CXSysOS::IsBit64System()
{
	BOOL bBit64Sys = FALSE;

	SYSTEM_INFO si = {0};

	FUN_GetNativeSystemInfo pfn = (FUN_GetNativeSystemInfo)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "GetNativeSystemInfo");
	if (NULL != pfn)
	{
		pfn(&si);

		if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||    
			si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 )
		{
			bBit64Sys = TRUE;
		}
	}
	
	return bBit64Sys;
}

BOOL CXSysOS::IsVistaOS()
{
	BOOL bVistaOS = FALSE;

	OSVERSIONINFO osver;
	osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	FUN_RtlGetVersion pFunRtlGetVersion = (FUN_RtlGetVersion)GetProcAddress(
		GetModuleHandleW(L"ntdll.dll"),"RtlGetVersion");
	if(NULL != pFunRtlGetVersion)
	{
		pFunRtlGetVersion(&osver);
	}

	if ((osver.dwPlatformId == VER_PLATFORM_WIN32_NT) && 
		(osver.dwMajorVersion >= 6 ))
	{
		bVistaOS = TRUE;
	}

	return bVistaOS;
}

int CXSysOS::GetOSType()
{
	int nOSType = OS_UNK;

	OSVERSIONINFO osver = {0};
	osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	FUN_RtlGetVersion pFunRtlGetVersion = (FUN_RtlGetVersion)GetProcAddress(
		GetModuleHandleW(L"ntdll.dll"),"RtlGetVersion");
	if(NULL != pFunRtlGetVersion)
	{
		pFunRtlGetVersion(&osver);
	}

	if (5 == osver.dwMajorVersion && 0 == osver.dwMinorVersion)
	{
		nOSType = OS_2K;
	}
	else if (5 == osver.dwMajorVersion && 1 == osver.dwMinorVersion)
	{
		nOSType = OS_XP;
	}
	else if(5 == osver.dwMajorVersion && 2 == osver.dwMinorVersion)
	{
		nOSType = OS_2K3;
	}
	else if (6 == osver.dwMajorVersion && 0 == osver.dwMinorVersion)
	{
		nOSType = OS_VISTA;
	}
	else if (6 == osver.dwMajorVersion && 1 == osver.dwMinorVersion)
	{
		nOSType = OS_WIN7;
	}
	else if (6 == osver.dwMajorVersion && 2 == osver.dwMinorVersion)
	{
		nOSType = OS_WIN8;
	}
	else if (10 == osver.dwMajorVersion && 0 == osver.dwMinorVersion)
	{
		nOSType = OS_WIN10;
	}
	
	return nOSType;
}

BOOL CXSysOS::RunProcessByParent(wchar_t * command,int nShow,BOOL bWait)
{
	BOOL bRet = FALSE;

	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si,sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = nShow;

	ZeroMemory(&pi,sizeof(pi));

	if(CreateProcessW(NULL,command,NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,NULL,              
		NULL,&si,&pi)) 
	{		
		bRet = TRUE;

		if (bWait)
		{
			::WaitForSingleObject(pi.hProcess,INFINITE);
		}		

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	
	return bRet;
}

BOOL CXSysOS::GetRandomGUID(wchar_t * pwszGuid)
{
	BOOL bCreated = FALSE;

	CoInitialize(NULL);

	GUID guid = {0};
	HRESULT hr = CoCreateGuid(&guid);
	if (SUCCEEDED(hr))
	{
		wsprintf(pwszGuid,L"%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X",
			guid.Data1, 
			guid.Data2, 
			guid.Data3, 
			guid.Data4[0], guid.Data4[1], 
			guid.Data4[2], guid.Data4[3],
			guid.Data4[4], guid.Data4[5], 
			guid.Data4[6], guid.Data4[7]);

		bCreated = TRUE;
	}

	return bCreated;
}

void CXSysOS::CharToWChar(const char * pszSrc,wchar_t * pwszDest,int iSrcLen,UINT CodePage)
{
	int nLen = 0;
	if (0 == iSrcLen)
	{
		nLen = strlen(pszSrc) + 1;
	}
	else
	{
		nLen = iSrcLen;
	}	

	MultiByteToWideChar(CodePage, 0,pszSrc,nLen,pwszDest,nLen); 
}

void CXSysOS::WCharToChar(const wchar_t * pwszSrc,char * pszDest,int iSrcLen,UINT CodePage)
{
	int nLen = 0;
	if (0 == iSrcLen)
	{
		nLen = wcslen(pwszSrc) + 1;
	}
	else
	{
		nLen = iSrcLen;
	}	

	WideCharToMultiByte(CodePage,0,pwszSrc,-1,pszDest,nLen,NULL,NULL);
}

int CXSysOS::GeWindowstBootType()
{
	int nBootType = WIN_BOOT_TYPE_NORMAL;
	int nRetCode = GetSystemMetrics(SM_CLEANBOOT);
	if (0 == nRetCode)
	{
		nBootType = WIN_BOOT_TYPE_NORMAL;
	}
	else if (1 == nRetCode)
	{
		nBootType = WIN_BOOT_TYPE_SAFE;
	}
	else if (2 == nRetCode)
	{
		nBootType = WIN_BOOT_TYPE_SAFE_NET;
	}

	return nBootType;
}

void CXSysOS::GetWorkingDir(wchar_t * pwszWorkDir)
{
	wchar_t wszExeFilePath[MAX_PATH] = {0};

	GetModuleFileNameW(NULL,wszExeFilePath,MAX_PATH);

	PathRemoveFileSpecW(wszExeFilePath);
}