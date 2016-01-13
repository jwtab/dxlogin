// VistaCreateProcess.cpp: implementation of the CVistaCreateProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include <stdio.h>
#include "VistaCreateProcess.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruc/tion
//////////////////////////////////////////////////////////////////////

CVistaCreateProcess::CVistaCreateProcess()
: lpGetTokenInformation (NULL),
lpRtlConvertSidToUnicodeString (NULL),
lpRtlFreeUnicodeString (NULL),
lpLoadUserProfile (NULL),
lpUnloadUserProfile (NULL),
lpCreateProcessAsUserA (NULL),
LpfnWTSQueryUserToken (NULL),
lpfnDestroyEnvironmentBlock (NULL)
{
	LpWTSGetActiveConsoleSessionId = NULL;
	LpfnOpenProcessToken = NULL;
	LpfnDuplicateTokenEx = NULL;
	LpfnSetTokenInformation = NULL;
	LpfnCreateEnvironmentBlock = NULL;
	LpfnCreateProcessAsUserW = NULL;
	lpLookupAccountSidA = NULL;
	m_hDll = NULL;
	m_hDll2 = NULL;
	m_hDll3 = NULL;
}

CVistaCreateProcess::~CVistaCreateProcess()
{
	if(m_hDll)
	{
		FreeLibrary(m_hDll);
	}
	
	if(m_hDll2)
	{
		FreeLibrary(m_hDll2);
	}
	
	if(m_hDll3)
	{
		FreeLibrary(m_hDll3);
	}
	
	if(m_hDll4)
	{
		FreeLibrary(m_hDll4);
	}
}

BOOL CVistaCreateProcess::IsVista()
{
	OSVERSIONINFO osver;
	
	osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	
	if (	::GetVersionEx( &osver ) && 
		osver.dwPlatformId == VER_PLATFORM_WIN32_NT && 
		(osver.dwMajorVersion >= 6 ) )
		return TRUE;
	
	return FALSE;
}

BOOL CVistaCreateProcess::MyShellExec(HWND hwnd, LPCSTR pszVerb, LPCSTR pszPath, LPCSTR pszParameters, LPCSTR pszDirectory)
{
	SHELLEXECUTEINFOA shex;
	
	memset( &shex, 0, sizeof( shex) );
	
	shex.cbSize			= sizeof( SHELLEXECUTEINFO ); 
	shex.fMask			= 0; 
	shex.hwnd			= hwnd;
	shex.lpVerb			= pszVerb; 
	shex.lpFile			= pszPath; 
	shex.lpParameters	= pszParameters; 
	shex.lpDirectory	= pszDirectory; 
	shex.nShow			= SW_NORMAL; 
	
	return ::ShellExecuteExA(&shex);
}

BOOL CVistaCreateProcess::RunElevated(HWND hwnd, LPCSTR pszPath, LPCSTR pszParameters, LPCSTR pszDirectory)
{
	return MyShellExec( hwnd, "runas", pszPath, pszParameters, pszDirectory );
}

BOOL CVistaCreateProcess::InitFunctionVar()
{
	if(NULL == LpWTSGetActiveConsoleSessionId)
	{
		LpWTSGetActiveConsoleSessionId = (fnWTSGetActiveConsoleSessionId) GetProcAddress(GetModuleHandleA("Kernel32.dll"), "WTSGetActiveConsoleSessionId");
		if(!LpWTSGetActiveConsoleSessionId)
		{
			OutputDebugStringA("WTSGetActiveConsoleSessionId Fail!\r\n");
		}
	}
	
	m_hDll = LoadLibraryA("Advapi32.dll");
	if(m_hDll)
	{
		if(NULL == LpfnOpenProcessToken)
		{
			LpfnOpenProcessToken = (fnOpenProcessToken) GetProcAddress (m_hDll, "OpenProcessToken");
			if(!LpfnOpenProcessToken)
			{
				OutputDebugStringA("OpenProcessToken Fail!\r\n");
			}
		}
		
		if(NULL == LpfnDuplicateTokenEx)
		{
			LpfnDuplicateTokenEx = (fnDuplicateTokenEx) GetProcAddress (m_hDll, "DuplicateTokenEx");
			if(!LpfnDuplicateTokenEx)
			{
				OutputDebugStringA("DuplicateTokenEx Fail!\r\n");
			}
		}
		
		if(NULL == LpfnSetTokenInformation)
		{
			LpfnSetTokenInformation = (fnSetTokenInformation) GetProcAddress (m_hDll, "SetTokenInformation");
			if(!LpfnSetTokenInformation)
			{
				OutputDebugStringA("SetTokenInformation Fail!\r\n");
			}
		}
		
		if(NULL == LpfnCreateProcessAsUserW)
		{
			LpfnCreateProcessAsUserW = (fnCreateProcessAsUserW) GetProcAddress (m_hDll, "CreateProcessAsUserW");
			if(!LpfnCreateProcessAsUserW)
			{
				OutputDebugStringA("CreateProcessAsUserW Fail!\r\n");
			}
		}
		
		if(NULL == lpCreateProcessAsUserA)
		{
			lpCreateProcessAsUserA = (PFNCreateProcessAsUserA) GetProcAddress (m_hDll, "CreateProcessAsUserA");
			if(!lpCreateProcessAsUserA)
			{
				OutputDebugStringA("Get CreateProcessAsUserA Fail!\r\n");
			}
		}
		
		if(NULL == lpGetTokenInformation)
		{
			if(!(lpGetTokenInformation = (PFNGetTokenInformation)
				GetProcAddress(m_hDll,"GetTokenInformation")))
			{
				OutputDebugStringA("Get GetTokenInformation Fail!\r\n");
			}
		}
		
		if(NULL == lpLookupAccountSidA)
		{
			if(!(lpLookupAccountSidA = (PFNLookupAccountSidA)
				GetProcAddress(m_hDll, "LookupAccountSidA")))
			{
				OutputDebugStringA("Get LookupAccountSid Fail !\r\n");
			}
		}
	}
	else
	{
		OutputDebugStringA("LoadLibrary(Advapi32.dll) Fail!\r\n");
	}
	
	m_hDll2 = LoadLibraryA("Userenv.dll");
	if(m_hDll2)
	{
		if(NULL == LpfnCreateEnvironmentBlock)
		{
			LpfnCreateEnvironmentBlock = (fnCreateEnvironmentBlock) GetProcAddress (m_hDll2, "CreateEnvironmentBlock");
			if(!LpfnCreateEnvironmentBlock)
			{
				OutputDebugStringA("CreateEnvironmentBlock Fail!\r\n");
			}
		}
		
		if(NULL == lpfnDestroyEnvironmentBlock)
		{
			lpfnDestroyEnvironmentBlock = (fnDestroyEnvironmentBlock) GetProcAddress(m_hDll2, "DestroyEnvironmentBlock");
			
			if(!lpfnDestroyEnvironmentBlock)
			{
				OutputDebugStringA("DestroyEnvironmentBlock Fail!\r\n");
			}
		}
		
		if(NULL == lpUnloadUserProfile)
		{
			lpUnloadUserProfile = (PFNUnloadUserProfile) GetProcAddress(m_hDll2, "UnloadUserProfile");
			if(!lpUnloadUserProfile)
			{
				OutputDebugStringA("UnloadUserProfile Fail!\r\n");
			}
		}
		
		if(NULL == lpLoadUserProfile)
		{
			lpLoadUserProfile = (PFNLoadUserProfile) GetProcAddress( m_hDll2, "LoadUserProfileA");
			if(!lpLoadUserProfile)
			{
				OutputDebugStringA("LoadUserProfile Fail!\r\n");
			}
		}
	}
	else
	{
		OutputDebugStringA("LoadLibrary(Userenv.dll) Fail!\r\n");
	}
	
	m_hDll3 = LoadLibraryA("Wtsapi32.dll");
	if(m_hDll3)
	{
		if(NULL == LpfnWTSQueryUserToken)
		{
			LpfnWTSQueryUserToken = (fnWTSQueryUserToken) GetProcAddress(m_hDll3,"WTSQueryUserToken");
			if(!LpfnWTSQueryUserToken)
			{
				OutputDebugStringA("WTSQueryUserToken Fail!\r\n");
			}
		}
	}
	else
	{
		OutputDebugStringA("LoadLibrary(Wtsapi32.dll) Fail!\r\n");
	}
	
	m_hDll4 = LoadLibraryA("ntdll.dll");
	if(m_hDll4)
	{
		if(NULL == lpRtlConvertSidToUnicodeString)
		{
			lpRtlConvertSidToUnicodeString = (PFNRtlConvertSidToUnicodeString)GetProcAddress( m_hDll4, "RtlConvertSidToUnicodeString");
			if(lpRtlConvertSidToUnicodeString == NULL)
			{
				OutputDebugStringA("Get RtlConvertSidToUnicodeString Fail!\r\n");
			}
		}
		
		if(NULL == lpRtlFreeUnicodeString)
		{
			lpRtlFreeUnicodeString = (PFNRtlFreeUnicodeString)
				GetProcAddress( m_hDll4, "RtlFreeUnicodeString");
			
			if(lpRtlFreeUnicodeString == NULL)
			{
				OutputDebugStringA("Get RtlFreeUnicodeString Fail!\r\n");
			}
		}
	}
	
	if(NULL == LpWTSGetActiveConsoleSessionId||
		NULL == LpfnOpenProcessToken ||
		NULL == LpfnDuplicateTokenEx ||
		NULL == LpfnSetTokenInformation ||
		NULL == LpfnCreateProcessAsUserW ||
		NULL == LpfnCreateEnvironmentBlock||
		NULL == lpfnDestroyEnvironmentBlock ||
		NULL == LpfnWTSQueryUserToken  ||  
		NULL == lpGetTokenInformation  || 
		NULL == lpRtlConvertSidToUnicodeString  ||  
		NULL == lpRtlFreeUnicodeString  ||  
		NULL == lpLookupAccountSidA)
	{
		return FALSE;
	}
	
	return TRUE;
}

BOOL CVistaCreateProcess::CreateProcessFromService(LPCSTR m_Command, BOOL bShow,BOOL bWaiting, DWORD *lpExitCode)
{
	WCHAR m_NewCommand[MAX_PATH * 2 +1] = {0};
	HANDLE	hTokenThis = NULL;
	HANDLE	hTokenDup = NULL;
	BOOL	m_Ret = FALSE;
	
	if(NULL == m_Command) return FALSE;
	
	if(!InitFunctionVar())
	{
		OutputDebugStringA("Vista:初始化函数失败！");
		return m_Ret;
	}
	
	if(!LpfnOpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hTokenThis))
	{
		OutputDebugStringA("CVista:OpenProcessToken Fail!\r\n");
		return m_Ret;
	}
	
	if(!LpfnDuplicateTokenEx(hTokenThis, MAXIMUM_ALLOWED,NULL, SecurityIdentification, TokenPrimary, &hTokenDup))
	{
		CloseHandle(hTokenThis);
		OutputDebugStringA("CVista:DuplicateTokenEx Fail!\r\n");
		return m_Ret;
	}
	
	DWORD dwSessionId = LpWTSGetActiveConsoleSessionId();
	
	if(!LpfnSetTokenInformation(hTokenDup, TokenSessionId, &dwSessionId, sizeof(DWORD)))
	{
		CloseHandle(hTokenThis);
		CloseHandle(hTokenDup);
		char szLog[1024] = {0};
		sprintf(szLog,"CVista:SetTokenInformation Fail : %d!\r\n.",GetLastError());
		OutputDebugStringA(szLog);
		return m_Ret;
	}
	
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(STARTUPINFOW));
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	si.cb = sizeof(STARTUPINFOW);
	si.lpDesktop = L"WinSta0\\Default";
	
	//隐藏窗口执行
	if (bShow)
	{
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_SHOW;
	}
	else
	{
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
	}
	
	LPVOID pEnv = NULL;
	DWORD dwCreationFlag = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
	
	if(LpfnCreateEnvironmentBlock(&pEnv, hTokenDup, FALSE))
	{
		dwCreationFlag |= CREATE_UNICODE_ENVIRONMENT;
	}
	else
	{
		CloseHandle(hTokenThis);
		CloseHandle(hTokenDup);
		OutputDebugStringA("CVista:CreateEnvironmentBlock Fail!\r\n");
		return m_Ret;
	}
	
	MultiByteToWideChar(CP_ACP,0,m_Command,-1,m_NewCommand,MAX_PATH*2);
	
	if(LpfnCreateProcessAsUserW(
		hTokenDup,
		NULL,
		m_NewCommand, 
		NULL,
		NULL,
		FALSE,
		dwCreationFlag,
		pEnv,
		NULL,
		&si,
		&pi))
	{
		m_Ret = TRUE;
		if(bWaiting)
		{
			WaitForSingleObject(pi.hProcess, INFINITE) ;
			
			if(lpExitCode)
			{
				GetExitCodeProcess(pi.hProcess,lpExitCode);
			}
			
		}
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
		
	}
	else
	{
		//CString m_Tmp;
		char szLog[1024] = {0};
		sprintf(szLog,"CVista:CreateProcessAsUser fail %d\r\n",GetLastError());
		OutputDebugStringA(szLog);
	}
	
	lpfnDestroyEnvironmentBlock(pEnv);
	CloseHandle(hTokenThis);
	CloseHandle(hTokenDup);
	return m_Ret;
}

BOOL CVistaCreateProcess::CreateLowProcess(LPSTR m_Command)
{
	HANDLE	hToken = NULL, hTokenDup = NULL;
	BOOL m_Ret = FALSE;
	
	if(NULL == m_Command) return FALSE;
	
	if(!InitFunctionVar())
	{
		OutputDebugStringA("Vista:初始化函数失败！");
		return m_Ret;
	}
	
	DWORD dwSessionId = LpWTSGetActiveConsoleSessionId();
	if (-1 == dwSessionId)
	{
		OutputDebugStringA("Vista:WTSGetActiveConsoleSessionId Fail！\r\n");
		return m_Ret;
	}
	
	if (!LpfnWTSQueryUserToken (dwSessionId, &hToken))
	{
		OutputDebugStringA("Vista:WTSQueryUserToken Fail！\r\n");
		return m_Ret;
	}
	
	if(LpfnDuplicateTokenEx (hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hTokenDup))
	{
		OutputDebugStringA("CVista:DuplicateTokenEx Fail!\r\n");
		CloseHandle(hToken);
		return m_Ret;
	}
	
	STARTUPINFOA si = {0};
	PROCESS_INFORMATION pi = {0};
	
	si.cb			= sizeof (STARTUPINFO);
	si.lpDesktop	= "winsta0\\default";
	
	if (lpCreateProcessAsUserA(
		hTokenDup,
		NULL,
		m_Command,
		NULL,
		NULL,
		FALSE,
		NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&si,
		&pi
		))
	{
		m_Ret = TRUE;
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
	}
	else
	{		
		char szLog[1024] = {0};
		sprintf(szLog,"CVista:CreateProcessAsUser fail %d\r\n",GetLastError());
		OutputDebugStringA(szLog);
	}
	
	CloseHandle(hToken);
	CloseHandle(hTokenDup);
	return m_Ret;
}

BOOL CVistaCreateProcess::CreateProcessLogonUser(LPSTR m_Command, BOOL bShow,BOOL bWaiting, DWORD *lpExitCode)
{
	STARTUPINFOA si={0}; 
	PROCESS_INFORMATION pi; 
	PROFILEINFOA lpProfileInfo;
	PVOID  lpEnvironment;
	char szUserName[MAX_PATH] = {0};
	BOOL m_Ret = FALSE;
	HANDLE	hToken = NULL, hTokenDup = NULL;
	char CurrentDirectory[MAX_PATH] = {0}, CurrentDisk[MAX_PATH] = {0}, CurFileName[MAX_PATH] = {0}, CurExt[MAX_PATH] = {0};
	
	if(!InitFunctionVar())
	{
		OutputDebugStringA("Vista:初始化函数失败！");		
		return m_Ret;
	}
	
	DWORD dwSessionId = LpWTSGetActiveConsoleSessionId();
	if (-1 == dwSessionId)
	{
		OutputDebugStringA("Vista:WTSGetActiveConsoleSessionId Fail！\r\n");	
		return m_Ret;
	}
	
	if (!LpfnWTSQueryUserToken (dwSessionId, &hToken))
	{
		char szLog[1024] = {0};
		sprintf(szLog,"Vista:WTSQueryUserToken Fail！SessionId = %d, value=%d\r\n", dwSessionId, GetLastError());
		
		OutputDebugStringA(szLog);
		
		hToken = GetCurrentTokenByExplorer();

		if(!hToken)
			return m_Ret;
	}
		
	if(LpfnDuplicateTokenEx (hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hTokenDup))
	{
		OutputDebugStringA("CVista:DuplicateTokenEx Fail!\r\n");
		return m_Ret;
	}
	
	//if(!GetCurrentLogonUserName(hToken, szUserName))
	if(!GetCurrentLogonUserNameNew(hToken, szUserName))
	{
		CloseHandle(hToken);
		OutputDebugStringA("CVista:GetCurrentLogonUserName Fail!\r\n");		
		return FALSE;
	}
	
	ZeroMemory(&lpProfileInfo,sizeof(PROFILEINFO));
	lpProfileInfo.dwSize= sizeof(PROFILEINFO);
	lpProfileInfo.lpUserName = szUserName;
	
	if(!lpLoadUserProfile(hToken,&lpProfileInfo))
	{
		OutputDebugStringA("CVista:LoadUserProfile Fail!\r\n");		
		CloseHandle(hToken);
		return FALSE;
	}
	
	if(!LpfnCreateEnvironmentBlock(&lpEnvironment,hToken,FALSE))
	{
		OutputDebugStringA("CVista:CreateEnvironmentBlock Fail!\r\n");
		lpUnloadUserProfile(hToken,lpProfileInfo.hProfile);
		CloseHandle(hToken);		
		return FALSE;
	}
	
	_splitpath(m_Command, CurrentDisk, CurrentDirectory, CurFileName, CurExt);
	
	
	
	si.cb = sizeof(STARTUPINFO);
	si.lpReserved = NULL;
	si.lpDesktop = NULL;
	si.lpTitle = NULL;
	if (bShow)
	{
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_SHOW;
	}
	else
	{
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
	}
	si.cbReserved2 = NULL;
	si.lpReserved2 = NULL;
		
	
	if (!lpCreateProcessAsUserA(hToken,NULL,m_Command,NULL,NULL,
								FALSE,CREATE_NEW_CONSOLE|CREATE_UNICODE_ENVIRONMENT,
								lpEnvironment,CurrentDisk,&si,&pi))
	{
		char Tmp[MAX_PATH] = {0};
		sprintf(Tmp, "CVista:CreateProcessAsUser Fail Vaule=%d, Command=%s!\r\n", GetLastError(), m_Command);
		OutputDebugStringA(Tmp);	
		m_Ret = FALSE;
	}
	else
	{
		char szLog[1024] = {0};
		sprintf(szLog,"VRVAUD_C:CreateProcessAsUser command = %s ok!",m_Command);
		
		OutputDebugStringA(szLog);
		
		if(bWaiting)
		{
			WaitForSingleObject(pi.hProcess, INFINITE) ;
			
			if(lpExitCode)
			{
				GetExitCodeProcess(pi.hProcess,lpExitCode);
			}
			
		}
		
		m_Ret = TRUE;
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
	}
	
	lpUnloadUserProfile(hToken,lpProfileInfo.hProfile);
	CloseHandle(hToken);
	lpfnDestroyEnvironmentBlock(lpEnvironment);
	return m_Ret;
}

BOOL CVistaCreateProcess::GetCurrentLogonUserName(HANDLE m_hToken, LPSTR lpszUserName)
{
	unsigned long vol,reg;
	HKEY          m_hkey1;
	BOOL	m_Ret = FALSE;
	
	vol=REG_OPTION_NON_VOLATILE;
	reg=REG_OPENED_EXISTING_KEY;
	
	char buf[1024*4];
	DWORD m_Len=1024*4;
	
	if(!lpGetTokenInformation(m_hToken,TokenUser,buf,m_Len,&m_Len))
	{
		OutputDebugStringA("GetTokenInformation Fail!\r\n");
		return m_Ret;
	}
	
	TOKEN_USER *LpOwer=(TOKEN_USER*)buf;
	UNICODE_STRING LpString;
	
	if(lpRtlConvertSidToUnicodeString(&LpString,LpOwer->User.Sid,TRUE))
	{
		OutputDebugStringA("RtlConvertSidToUnicodeString Fail!\r\n");
		return m_Ret;
	}
	
	
	char SubKey[2048] = {0};
	
	WideCharToMultiByte(CP_ACP,0,LpString.Buffer,-1,SubKey,2048,NULL,NULL);
	
	lpRtlFreeUnicodeString(&LpString);
	
	if(strlen(SubKey) == 0) return m_Ret;
	
	strcat(SubKey, "\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer");
	
	long ret1=RegCreateKeyExA(HKEY_USERS,
							 SubKey,
							 0,
							 "",
							 vol,
							 KEY_ALL_ACCESS,
							 NULL,
							 &m_hkey1,
							 &reg);
	if(ret1 != ERROR_SUCCESS)	
	{
		return m_Ret;
	}
	
	unsigned long type,len;
	
	type=REG_SZ;
	len = MAX_PATH;
	
	ret1=RegQueryValueExA(m_hkey1, 
		"Logon User Name",
		NULL, &type,
		(BYTE *)lpszUserName,
		&len);
	
	if(ret1 == ERROR_SUCCESS)
	{
		m_Ret = TRUE;
	}
	
	RegCloseKey(m_hkey1);
	
	
	return m_Ret;
	
}

BOOL CVistaCreateProcess::CreateProcessLogonUser2K(LPSTR m_Command, BOOL bWaiting, DWORD *lpExitCode)
{
	STARTUPINFOA si={0}; 
	PROCESS_INFORMATION pi; 
	PROFILEINFOA lpProfileInfo;
	PVOID  lpEnvironment;
	char szUserName[MAX_PATH] = {0};
	BOOL bRet = FALSE;
	HANDLE	hToken = NULL, hTokenDup = NULL;
	DWORD m_ProcessID=0;
	char CurrentDirectory[MAX_PATH] = {0}, CurrentDisk[MAX_PATH] = {0}, CurFileName[MAX_PATH] = {0}, CurExt[MAX_PATH] = {0};
	
	InitFunctionVar();
	
	if(NULL == lpLoadUserProfile  ||  
		NULL == LpfnCreateEnvironmentBlock  ||  
		NULL == lpUnloadUserProfile  ||  
		NULL == lpCreateProcessAsUserA ||
		NULL == lpGetTokenInformation  ||  
		NULL == lpRtlConvertSidToUnicodeString  ||  
		NULL == lpRtlFreeUnicodeString  ||  
		NULL == LpfnOpenProcessToken)
		return FALSE;
	
	HWND m_hWnd=FindWindowA("Shell_TrayWnd",NULL);
	
	if(NULL==m_hWnd)
	{
		return bRet;
	}
	
	GetWindowThreadProcessId(m_hWnd,&m_ProcessID);
	
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,m_ProcessID);
	
	if( !hProcess ) return FALSE;
	
	bRet = LpfnOpenProcessToken(hProcess,TOKEN_ALL_ACCESS,&hToken);
	
	if(!bRet)
	{
		CloseHandle(hProcess);
		return FALSE;
	}
	
	CloseHandle(hProcess);
	
	
	if(!GetCurrentLogonUserName(hToken, (char*)szUserName))
	{
		CloseHandle(hToken);
		OutputDebugStringA("CVista:2k GetCurrentLogonUserName Fail!\r\n");
		return FALSE;
	}
	
	ZeroMemory(&lpProfileInfo,sizeof(PROFILEINFO));
	lpProfileInfo.dwSize= sizeof(PROFILEINFO);
	lpProfileInfo.lpUserName = szUserName;
	
	if(!lpLoadUserProfile(hToken,&lpProfileInfo))
	{
		CloseHandle(hToken);
		OutputDebugStringA("CVista:LoadUserProfile Fail!\r\n");
		return FALSE;
	}
	
	if(!LpfnCreateEnvironmentBlock(&lpEnvironment,hToken,FALSE))
	{
		OutputDebugStringA("CVista:CreateEnvironmentBlock Fail!\r\n");
		lpUnloadUserProfile(hToken,lpProfileInfo.hProfile);
		CloseHandle(hToken);
		return FALSE;
	}
	
	_splitpath(m_Command, CurrentDisk, CurrentDirectory, CurFileName, CurExt);
	
	if (!(lpCreateProcessAsUserA(hToken,NULL,m_Command,NULL,NULL,FALSE,CREATE_NEW_CONSOLE|CREATE_UNICODE_ENVIRONMENT,lpEnvironment,CurrentDisk,&si,&pi)))
	{
		char Tmp[MAX_PATH] = {0};
		sprintf(Tmp, "CVista:CreateProcessAsUser Fail %d!\r\n", GetLastError());
		
		bRet = FALSE;
	}
	else
	{
		if(bWaiting)
		{
			WaitForSingleObject(pi.hProcess, INFINITE) ;
			
			if(lpExitCode)
			{
				GetExitCodeProcess(pi.hProcess,lpExitCode);
			}
			
		}
		bRet = TRUE;
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
	}
	
	lpUnloadUserProfile(hToken,lpProfileInfo.hProfile);
	lpfnDestroyEnvironmentBlock(lpEnvironment);
	CloseHandle(hToken);
	return bRet;
}

BOOL CVistaCreateProcess::GetCurrentLogonUserNameNew(HANDLE m_hToken, LPSTR lpszUserName)
{
	BOOL m_Ret = FALSE;
	char buf[1024*4] = {0};
	DWORD m_Len=1024*4;
	SID_NAME_USE snu;
	DWORD cchUserName = MAX_PATH, cchDomainName = MAX_PATH;
	char domainName[MAX_PATH] = {0};
	
	
	if( NULL == lpszUserName )  return m_Ret;
	
	if(!lpGetTokenInformation(m_hToken,TokenUser,buf,m_Len,&m_Len))
	{
		OutputDebugStringA("GetTokenInformation Fail!\r\n");
		return m_Ret;
	}
	
	//if(!LookupAccountSid(NULL, ((PTOKEN_USER)buf)->User.Sid, lpszUserName, &cchUserName, domainName, &cchDomainName, &snu ))
	if(!lpLookupAccountSidA(NULL, ((PTOKEN_USER)buf)->User.Sid, lpszUserName, &cchUserName, domainName, &cchDomainName, &snu ))
	{
		OutputDebugStringA("LookupAccountSid Fail!\r\n");
	}
	else
	{
		m_Ret = TRUE;
	}
	
	return m_Ret;
}

HANDLE CVistaCreateProcess::GetCurrentTokenByExplorer()
{
	HANDLE	hToken = NULL;
	DWORD	m_ProcessID = 0;

	if(NULL == LpfnOpenProcessToken)
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
		OutputDebugStringA("CVista: OpenProcess EXPLORER Fail!");
		return hToken;
	}

	if(!LpfnOpenProcessToken(hProcess,TOKEN_ALL_ACCESS,&hToken))
	{
		OutputDebugStringA("CVista: OpenProcessToken EXPLORER Fail!");
		CloseHandle(hProcess);
		return hToken;
	}
	
	CloseHandle(hProcess);

	return hToken;
}
