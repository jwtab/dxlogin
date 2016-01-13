// ImpersonateLogon.cpp: implementation of the CImpersonateLogon class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImpersonateLogon.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CImpersonateLogon::CImpersonateLogon()
	:	hKernel32 (NULL)
	,	hWtsapi32 (NULL)
	,	hAdvapi32 (NULL)
	,	lpWTSGetActiveConsoleSessionId (NULL)
	,	lpWTSQueryUserToken (NULL)
	,	lpImpersonateLoggedOnUser(NULL)
	,	lpRevertToSelf (NULL)
	,	lpDuplicateTokenEx (NULL)
	,	lpOpenProcessToken (NULL)
	,	_hToken (NULL)
	,	_hTokenDup (NULL)
	,	lpGetTokenInformation (NULL)
	,	lpRtlConvertSidToUnicodeString (NULL)
	,	lpRtlFreeUnicodeString (NULL)
	,	m_Logon (NULL)
{
	_OsVersion = GetOsVersion();
	InitLibrary();
	memset(m_szSID, 0, 1024*4);
	//SetSpecificPrivilegeInAccessToken("SeTcbPrivilege",TRUE);
}

CImpersonateLogon::CImpersonateLogon(BOOL m_bNeedLogon)
	:	hKernel32 (NULL)
	,	hWtsapi32 (NULL)
	,	hAdvapi32 (NULL)
	,	lpWTSGetActiveConsoleSessionId (NULL)
	,	lpWTSQueryUserToken (NULL)
	,	lpImpersonateLoggedOnUser(NULL)
	,	lpRevertToSelf (NULL)
	,	lpDuplicateTokenEx (NULL)
	,	lpOpenProcessToken (NULL)
	,	_hToken (NULL)
	,	_hTokenDup (NULL)
	,	lpGetTokenInformation (NULL)
	,	lpRtlConvertSidToUnicodeString (NULL)
	,	lpRtlFreeUnicodeString (NULL)
	,	m_Logon (m_bNeedLogon)
{
	_OsVersion = GetOsVersion();

	InitLibrary();

	memset(m_szSID, 0, 1024*4);
	
	SessionLogon();
}

CImpersonateLogon::~CImpersonateLogon()
{
	if(m_Logon)
	{
		LogOut();
	}

	if(_hToken)
	{
		CloseHandle (_hToken);
	}
	
	if(_hTokenDup)
	{
		CloseHandle (_hTokenDup);
	}

	if(hWtsapi32)
	{
		FreeLibrary (hWtsapi32);
	}
	if(hAdvapi32)
	{
		FreeLibrary (hAdvapi32);
	}
}

BOOL CImpersonateLogon::SessionLogon()
{
	BOOL		bRet		= FALSE;
	HANDLE		hToken		= NULL;
	DWORD		dwProcessID = 0;

	if(NULL == lpImpersonateLoggedOnUser)
	{
		return bRet;
	}

	if(NULL == _hTokenDup)
	{
		switch(_OsVersion)
		{
		case WINDOWS2K:
			{	
				if(NULL == lpOpenProcessToken ||
					NULL == lpDuplicateTokenEx)
				{
					return bRet;
				}

				HWND m_hWnd = FindWindowA("Shell_TrayWnd",NULL);
				
				if(NULL==m_hWnd)
				{
					return bRet;
				}
				
				GetWindowThreadProcessId(m_hWnd,&dwProcessID);
				
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,dwProcessID);
				
				if( !hProcess ) return bRet;
				
				bRet = lpOpenProcessToken(hProcess,TOKEN_ALL_ACCESS,&_hToken);
				
				if(bRet)
				{
					if(lpDuplicateTokenEx (_hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &_hTokenDup))
					{
						if(lpImpersonateLoggedOnUser (_hTokenDup))
						{
							bRet = TRUE;
						}
					}
				}
				
				CloseHandle(hProcess);
				break;
			}
		case XpLater:
			{
				if(NULL == lpWTSQueryUserToken ||
					NULL == lpDuplicateTokenEx ||
					NULL == lpWTSGetActiveConsoleSessionId)
				{
					return bRet;
				}
				DWORD dwSessionId = lpWTSGetActiveConsoleSessionId();
				
				if(lpWTSQueryUserToken (dwSessionId, &_hToken))
				{
					if(lpDuplicateTokenEx (_hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &_hTokenDup))
					{
						if(lpImpersonateLoggedOnUser (_hTokenDup))
						{
							return TRUE;
						}
					}
				}
				else
				{
					if(GetLastError() == 0x522)
					{
						OutputDebugStringA("VRVSC_C.DLL:没有TCB权限！");

						_hToken = GetCurrentTokenByExplorer();
						
						if(_hToken)
						{
							if(lpDuplicateTokenEx (_hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &_hTokenDup))
							{
								if(lpImpersonateLoggedOnUser (_hTokenDup))
								{
									OutputDebugStringA("登陆成功");
									bRet = TRUE;
								}
								else
								{
									OutputDebugStringA("登录失败");
								}
							}
						}
					}
				}
				break;
			}
		}
	}
	else
	{
		if(lpImpersonateLoggedOnUser (_hTokenDup))
		{
			bRet = TRUE;
		}
	}
	return bRet;
}

HANDLE CImpersonateLogon::TokenDup()
{
	return _hTokenDup;
}

void CImpersonateLogon::LogOut()
{
	if(lpRevertToSelf)
	{
		lpRevertToSelf ();
	}
}

BOOL CImpersonateLogon::GetUserSID(char *lpszSID, DWORD m_Size)
{
	if(strlen(m_szSID) == 0)
	{
		if(!QueryUserSID())
		{
			return FALSE;
		}
	}

	if(m_Size < strlen(m_szSID))
	{
		return FALSE;
	}

	strcpy(lpszSID, m_szSID);

	return TRUE;
}

BOOL CImpersonateLogon::QueryUserSID()
{
	char buf[1024*4] = {0};
	DWORD m_Len=1024*4;
	
	if(NULL == lpWTSQueryUserToken ||
		NULL == lpDuplicateTokenEx ||
		NULL == lpImpersonateLoggedOnUser||
		NULL == lpWTSGetActiveConsoleSessionId||
		NULL == lpGetTokenInformation ||
		NULL == lpRtlConvertSidToUnicodeString||
		NULL == lpRtlFreeUnicodeString)
	{
		return FALSE;
	}
	
	if(NULL == _hTokenDup)
	{
		DWORD dwSessionId = lpWTSGetActiveConsoleSessionId();
		
		if(lpWTSQueryUserToken (dwSessionId, &_hToken))
		{
			if(!lpDuplicateTokenEx (_hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &_hTokenDup))
			{
				CloseHandle(_hToken);
				_hToken = NULL;
				_hTokenDup = NULL;
			}
		}
		else
		{
			if(GetLastError() == 0x522)
			{
				OutputDebugStringA("VRVSC_C.DLL:没有TCB权限！");

				_hToken = GetCurrentTokenByExplorer();
				
				if(_hToken)
				{
					if(!lpDuplicateTokenEx (_hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &_hTokenDup))
					{
						CloseHandle(_hToken);
						_hToken = NULL;
						_hTokenDup = NULL;
					}
				}
			}
		}
	}
	
	if(_hTokenDup)
	{
		if(lpGetTokenInformation(_hTokenDup,TokenUser,buf,m_Len,&m_Len))
		{
			TOKEN_USER *LpOwer=(TOKEN_USER*)buf;
			UNICODE_STRING LpString;
			
			if(lpRtlConvertSidToUnicodeString(&LpString,LpOwer->User.Sid,TRUE)==NULL)
			{
				memset(m_szSID, 0, 1024*4);
				
				WideCharToMultiByte(CP_ACP,0,LpString.Buffer,-1,m_szSID,1024*4,NULL,NULL);
				
				lpRtlFreeUnicodeString(&LpString);
				
				return TRUE;
			}
		}
	}
	return FALSE;
}

void CImpersonateLogon::InitLibrary()
{
	hKernel32 = GetModuleHandleA("Kernel32.DLL");
	if(hKernel32)
	{
		lpWTSGetActiveConsoleSessionId = (PFNWTSGetActiveConsoleSessionId)
			GetProcAddress (hKernel32, ("WTSGetActiveConsoleSessionId"));
	}
	hWtsapi32 = LoadLibraryA ("Wtsapi32.dll");
	if (hWtsapi32)
	{
		lpWTSQueryUserToken = (PFNWTSQueryUserToken)
			GetProcAddress (hWtsapi32, "WTSQueryUserToken"); 
	}
	
	hAdvapi32 = LoadLibraryA ("Advapi32.dll");
	if (hAdvapi32) 
	{
		lpImpersonateLoggedOnUser = (PFNImpersonateLoggedOnUser) 
			GetProcAddress(hAdvapi32, "ImpersonateLoggedOnUser"); 
		lpRevertToSelf = (PFNRevertToSelf) 
			GetProcAddress(hAdvapi32, "RevertToSelf"); 
		lpDuplicateTokenEx = (PFNDuplicateTokenEx) 
			GetProcAddress(hAdvapi32, "DuplicateTokenEx");
		lpGetTokenInformation = (PFNGetTokenInformation)
			GetProcAddress(hAdvapi32,"GetTokenInformation");
		lpOpenProcessToken = (PFNOpenProcessToken)
			GetProcAddress(hAdvapi32, "OpenProcessToken");
	}
	
	hNtdll = GetModuleHandleA( "ntdll.dll" );
	
	if(hNtdll)
	{
		lpRtlConvertSidToUnicodeString =(PFNRtlConvertSidToUnicodeString)
			GetProcAddress( hNtdll,"RtlConvertSidToUnicodeString");
		
		lpRtlFreeUnicodeString = (PFNRtlFreeUnicodeString)
			GetProcAddress(hNtdll,"RtlFreeUnicodeString");
	}
}

OSTYPE CImpersonateLogon::GetOsVersion()
{
	OSVERSIONINFO VersionInfo;  // pointer to version 
	
	VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);
	
	GetVersionEx(&VersionInfo);
	
	if(VersionInfo.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
	{
		return WINDOWS9X;	
	}
	
	if(VersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{	
		if ( VersionInfo.dwMajorVersion == 5 && VersionInfo.dwMinorVersion == 0 )
		{
			return WINDOWS2K;
		}
		else if(VersionInfo.dwMajorVersion > 4)
		{
			return XpLater;
		}
	}
	return OsUnknown;
}

HANDLE CImpersonateLogon::GetCurrentTokenByExplorer()
{
	HANDLE	hToken = NULL;
	DWORD	m_ProcessID = 0;
	
	if(NULL == lpOpenProcessToken)
		return hToken;
	
	HWND	m_hWnd=FindWindowA("Shell_TrayWnd",NULL);
	
	if(NULL==m_hWnd)
	{
		OutputDebugStringA("CVista: EXPLORER NOT FOUND!");
		return hToken;
	}
	
	GetWindowThreadProcessId(m_hWnd,&m_ProcessID);
	
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,m_ProcessID);
	
	if( !hProcess )
	{
		OutputDebugStringA("CImpersonateLogon: OpenProcess EXPLORER Fail!");
		return hToken;
	}
	
	if(!lpOpenProcessToken(hProcess,TOKEN_ALL_ACCESS,&hToken))
	{
		OutputDebugStringA("CImpersonateLogon: OpenProcessToken EXPLORER Fail!");
		CloseHandle(hProcess);
		return hToken;
	}
	
	CloseHandle(hProcess);
	
	return hToken;
}
