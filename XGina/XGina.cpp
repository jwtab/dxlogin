// XGina.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "XGina.h"

//Pointers to redirected functions.
static PWLX_DIALOG_BOX_PARAM pfWlxDialogBoxParam = NULL;

//窗口处理函数.
static DLGPROC pfWlxLoggedOutSASDlgProc   = NULL;
static DLGPROC pfWlxWkstaLockedSASDlgProc = NULL;
static DLGPROC pfChangePasswordDlgProc    = NULL;
static DLGPROC pfWlxDisplaySASDlgProc = NULL;
static DLGPROC pfWlxLoggedOnSASDlgProc = NULL;

//Winlogon function dispatch table.
static PVOID g_pWinlogon = NULL;
static DWORD g_dwVersion = WLX_VERSION_1_3;

static FARPROC gMsGinaFunAddr[28];  //28个裸函数.

static PFWLXNEGOTIATE						   pfWlxNegotiate;
static PFWLXINITIALIZE							    pfWlxInitialize;
static PFWLXDISPLAYSASNOTICE			  pfWlxDisplaySASNotice;
static PFWLXLOGGEDOUTSAS					pfWlxLoggedOutSAS;
static PFWLXACTIVATEUSERSHELL		    pfWlxActivateUserShell;
static PFWLXLOGGEDONSAS					  pfWlxLoggedOnSAS;
static PFWLXDISPLAYLOCKEDNOTICE     pfWlxDisplayLockedNotice;
static PFWLXWKSTALOCKEDSAS			  pfWlxWkstaLockedSAS;
static PFWLXISLOCKOK							  pfWlxIsLockOk;
static PFWLXISLOGOFFOK						  pfWlxIsLogoffOk;
static PFWLXLOGOFF								   pfWlxLogoff;
static PFWLXSHUTDOWN						pfWlxShutdown;

static PWLXCHANGEPASSWORDNOTIFY    pfWlxChangePasswordNotify;

//New for version 1.1
static PFWLXSTARTAPPLICATION     pfWlxStartApplication  = NULL;
static PFWLXSCREENSAVERNOTIFY    pfWlxScreenSaverNotify = NULL;


// New for version 1.2 - No new GINA interface was added, except
//                       a new function in the dispatch table.

// New for version 1.3
static PFWLXNETWORKPROVIDERLOAD   pfWlxNetworkProviderLoad  = NULL;
static PFWLXDISPLAYSTATUSMESSAGE  pfWlxDisplayStatusMessage = NULL;
static PFWLXGETSTATUSMESSAGE      pfWlxGetStatusMessage     = NULL;
static PFWLXREMOVESTATUSMESSAGE   pfWlxRemoveStatusMessage  = NULL;

static FUNShellShutdownDialog pfShellShutdownDialog;

//远程桌面函数
static FUNWLXGetConsoleSwitchCredentials pfWlxGetConsoleSwitchCredentials;
static FUNWLXReconnectNotify pfWlxReconnectNotify;
static FUNWLXDisconnectNotify pfWlxDisconnectNotify;

// Local functions.
int WINAPI HookedWlxChangePasswordNotify(HANDLE hWlx, PWLX_MPR_NOTIFY_INFO pMprInfo,DWORD dwChangeInfo);
int WINAPI HookedWlxDialogBoxParam(HANDLE, HANDLE, LPWSTR, HWND, DLGPROC, LPARAM);

void HookWlxDialogBoxParam(PVOID pWinlogonFunctions, DWORD dwWlxVersion)
{
	switch (dwWlxVersion)
	{
	case WLX_VERSION_1_0: 
		{
			pfWlxChangePasswordNotify = ((PWLX_DISPATCH_VERSION_1_0)pWinlogonFunctions)->WlxChangePasswordNotify;
			((PWLX_DISPATCH_VERSION_1_0)pWinlogonFunctions)->WlxChangePasswordNotify = HookedWlxChangePasswordNotify;
			
			pfWlxDialogBoxParam = ((PWLX_DISPATCH_VERSION_1_0)pWinlogonFunctions)->WlxDialogBoxParam;
			((PWLX_DISPATCH_VERSION_1_0)pWinlogonFunctions)->WlxDialogBoxParam = HookedWlxDialogBoxParam;												 
						
			break;
		}
		
	case WLX_VERSION_1_1:
		{
			pfWlxChangePasswordNotify = ((PWLX_DISPATCH_VERSION_1_1)pWinlogonFunctions)->WlxChangePasswordNotify;
			((PWLX_DISPATCH_VERSION_1_1)pWinlogonFunctions)->WlxChangePasswordNotify = HookedWlxChangePasswordNotify;
			
			pfWlxDialogBoxParam = ((PWLX_DISPATCH_VERSION_1_1)pWinlogonFunctions)->WlxDialogBoxParam;
			((PWLX_DISPATCH_VERSION_1_1)pWinlogonFunctions)->WlxDialogBoxParam = HookedWlxDialogBoxParam;
			
			break;
		}
		
	case WLX_VERSION_1_2:
		{
			pfWlxChangePasswordNotify = ((PWLX_DISPATCH_VERSION_1_2)pWinlogonFunctions)->WlxChangePasswordNotify;
			((PWLX_DISPATCH_VERSION_1_2)pWinlogonFunctions)->WlxChangePasswordNotify = HookedWlxChangePasswordNotify;
			
			pfWlxDialogBoxParam = ((PWLX_DISPATCH_VERSION_1_2)pWinlogonFunctions)->WlxDialogBoxParam;
			((PWLX_DISPATCH_VERSION_1_2)pWinlogonFunctions)->WlxDialogBoxParam = HookedWlxDialogBoxParam;
			
			break;
		}
		
	case WLX_VERSION_1_3:
		{
			pfWlxChangePasswordNotify = ((PWLX_DISPATCH_VERSION_1_3)pWinlogonFunctions)->WlxChangePasswordNotify;
			((PWLX_DISPATCH_VERSION_1_3)pWinlogonFunctions)->WlxChangePasswordNotify = HookedWlxChangePasswordNotify;
			
			pfWlxDialogBoxParam = ((PWLX_DISPATCH_VERSION_1_3)pWinlogonFunctions)->WlxDialogBoxParam;
			((PWLX_DISPATCH_VERSION_1_3)pWinlogonFunctions)->WlxDialogBoxParam = HookedWlxDialogBoxParam;
						
			break;
		}
		
	default:
		{
			pfWlxChangePasswordNotify = ((PWLX_DISPATCH_VERSION_1_4)pWinlogonFunctions)->WlxChangePasswordNotify;
			((PWLX_DISPATCH_VERSION_1_4)pWinlogonFunctions)->WlxChangePasswordNotify = HookedWlxChangePasswordNotify;
			
			pfWlxDialogBoxParam = ((PWLX_DISPATCH_VERSION_1_4)pWinlogonFunctions)->WlxDialogBoxParam;
			((PWLX_DISPATCH_VERSION_1_4)pWinlogonFunctions)->WlxDialogBoxParam = HookedWlxDialogBoxParam;
			
			break;
		}
	}
}

/*
	用户第一次登录,提示用户按下 Alt+Ctrl+Delete 界面处理函数.
*/
BOOL CALLBACK
HookedWlxDisplaySASDlgProc(HWND   hwndDlg,  
						    UINT   uMsg,     
						    WPARAM wParam, 
						    LPARAM lParam)  
{	
	BOOL bResult;

	//
	bResult = pfWlxDisplaySASDlgProc(hwndDlg, uMsg, wParam, lParam);	

	return bResult;
}

/*
	用户第一次登录,提示用户输入 用户名+密码 界面处理函数.
*/
BOOL CALLBACK  
HookedWlxLoggedOutSASDlgProc(HWND   hwndDlg,  // handle to dialog box
							  UINT   uMsg,     // message  
							  WPARAM wParam,   // first message parameter
							  LPARAM lParam)   // second message parameter
{
	BOOL bResult;		

	//
	bResult = pfWlxLoggedOutSASDlgProc(hwndDlg, uMsg, wParam, lParam);
	if (WM_INITDIALOG == uMsg && !g_wndEx[WND_TYPE_OUT_LOG].bInit)
	{
		g_wndEx[WND_TYPE_OUT_LOG].pWndEx = new CGinaWndEx();
		if (NULL != g_wndEx[WND_TYPE_OUT_LOG].pWndEx)
		{			
			g_wndEx[WND_TYPE_OUT_LOG].bInit = TRUE;
		}		
	}
	
	if (NULL != g_wndEx[WND_TYPE_OUT_LOG].pWndEx && g_wndEx[WND_TYPE_OUT_LOG].bInit)
	{
		g_wndEx[WND_TYPE_OUT_LOG].pWndEx->SetWnd(hwndDlg,WND_TYPE_OUT_LOG);
		g_wndEx[WND_TYPE_OUT_LOG].pWndEx->WndProcEx(uMsg,wParam,lParam);
	}	

	return bResult;
}

/*
	用户锁屏登录,提示用户按下 Alt+Ctrl+Delete 界面处理函数.
*/
BOOL CALLBACK 
HookedWlxWkstaLockedSASDlgProc(HWND   hwndDlg,  // handle to dialog box
								UINT   uMsg,     // message  
								WPARAM wParam,   // first message parameter
								LPARAM lParam)   // second message parameter
{
	BOOL bResult;	
	
	//
	bResult = pfWlxWkstaLockedSASDlgProc(hwndDlg, uMsg, wParam, lParam);
	
	return bResult;
}

/*
	用户锁屏登录,提示用户输入 用户名+密码 界面处理函数.
*/
BOOL CALLBACK 
HookedWlxLoggedOnSASDlgProc (HWND   hwndDlg,  
						     UINT   uMsg,     
							 WPARAM wParam, 
							 LPARAM lParam)  
{	
	BOOL bResult = FALSE;

	//
	bResult = pfWlxLoggedOnSASDlgProc(hwndDlg, uMsg, wParam, lParam);
	if (WM_INITDIALOG == uMsg && !g_wndEx[WND_TYPE_ON_LOG].bInit)
	{
		g_wndEx[WND_TYPE_ON_LOG].pWndEx = new CGinaWndEx();
		if (NULL != g_wndEx[WND_TYPE_ON_LOG].pWndEx)
		{			
			g_wndEx[WND_TYPE_ON_LOG].bInit = TRUE;
		}		
	}

	if (NULL != g_wndEx[WND_TYPE_ON_LOG].pWndEx && g_wndEx[WND_TYPE_ON_LOG].bInit)
	{
		g_wndEx[WND_TYPE_ON_LOG].pWndEx->SetWnd(hwndDlg,WND_TYPE_ON_LOG);
		g_wndEx[WND_TYPE_ON_LOG].pWndEx->WndProcEx(uMsg,wParam,lParam);
	}	

	return bResult;
}

/*
	用户修改密码 界面处理函数.
*/
BOOL
CALLBACK
HookedChangePasswordDlgProc (HWND   hwndDlg,  // handle to dialog box
							 UINT   uMsg,     // message  
							 WPARAM wParam,   // first message parameter
							 LPARAM lParam)   // second message parameter
{
	BOOL bResult;	
	
	//
	bResult = pfChangePasswordDlgProc(hwndDlg, uMsg, wParam, lParam);		
	
	return bResult;
}

int 
WINAPI
HookedWlxChangePasswordNotify(HANDLE hWlx, PWLX_MPR_NOTIFY_INFO pMprInfo,  DWORD dwChangeInfo)
{	
	return pfWlxChangePasswordNotify(hWlx,pMprInfo,dwChangeInfo);
}

int 
WINAPI 
HookedWlxDialogBoxParam (HANDLE  hWlx,
		HANDLE  hInst,
		LPWSTR  lpszTemplate,
		HWND    hwndOwner,
		DLGPROC dlgprc,
		LPARAM  dwInitParam)
{	
	if (!HIWORD(lpszTemplate))
	{	
		switch ((DWORD)lpszTemplate)
		{
		case IDD_WLXDIAPLAYSASNOTICE_DIALOG:
			{				
				pfWlxDisplaySASDlgProc = dlgprc;
				return pfWlxDialogBoxParam(hWlx, hInst, lpszTemplate,(HWND)hwndOwner, 
					(DLGPROC)HookedWlxDisplaySASDlgProc, dwInitParam);
			}
			
		case 1500:
		case IDD_WLXLOGGEDOUTSAS_DIALOG:
			{
				pfWlxLoggedOutSASDlgProc = dlgprc;
				return pfWlxDialogBoxParam(hWlx, hInst, lpszTemplate, hwndOwner, 
					(DLGPROC)HookedWlxLoggedOutSASDlgProc, dwInitParam);
			}
			
		case 1900:
		case IDD_WLXWKSTALOCKEDSAS_DIALOG:
			{
				pfWlxWkstaLockedSASDlgProc = dlgprc;
				return pfWlxDialogBoxParam(hWlx, hInst, lpszTemplate, hwndOwner, 
					(DLGPROC)HookedWlxWkstaLockedSASDlgProc, dwInitParam);
			}
		case 1950:
		case IDD_WLXLOGGEDONSAS_DIALOG:
			{	
				pfWlxLoggedOnSASDlgProc = dlgprc;
				return pfWlxDialogBoxParam(hWlx, hInst, lpszTemplate, hwndOwner, 
					(DLGPROC)HookedWlxLoggedOnSASDlgProc, dwInitParam);
			}
		case 1700:
		case IDD_CHANGE_PASSWORD_DIALOG:
			{	
				pfChangePasswordDlgProc = dlgprc;
				return pfWlxDialogBoxParam(hWlx, hInst, lpszTemplate, hwndOwner, 
					(DLGPROC)HookedChangePasswordDlgProc, dwInitParam);
			}
		}
	}
	
	return pfWlxDialogBoxParam(hWlx, hInst, lpszTemplate, 
		hwndOwner, dlgprc, dwInitParam);
}
		  
BOOL OrgInitialize(HINSTANCE hDll, DWORD dwWlxVersion)
{
	int nVer = dwWlxVersion;
			
	//WlxNegotiate
	pfWlxNegotiate = (PFWLXNEGOTIATE)GetProcAddress(hDll,"WlxNegotiate");
	if (!pfWlxNegotiate) 
	{
		return FALSE;
	}
	
	pfWlxInitialize = (PFWLXINITIALIZE) GetProcAddress(hDll, "WlxInitialize");
	if (NULL == pfWlxInitialize) 
	{
		return FALSE;
	}
	
	pfWlxDisplaySASNotice = (PFWLXDISPLAYSASNOTICE) GetProcAddress(hDll, "WlxDisplaySASNotice");
	if (NULL == pfWlxDisplaySASNotice) 
	{
		return FALSE;
	}
	
	pfWlxLoggedOutSAS = (PFWLXLOGGEDOUTSAS) GetProcAddress(hDll, "WlxLoggedOutSAS");
	if (NULL == pfWlxLoggedOutSAS) 
	{
		return FALSE;
	}
	
	pfWlxActivateUserShell = (PFWLXACTIVATEUSERSHELL) GetProcAddress(hDll, "WlxActivateUserShell");
	if (NULL == pfWlxActivateUserShell) 
	{
		return FALSE;
	}
	
	pfWlxLoggedOnSAS = (PFWLXLOGGEDONSAS) GetProcAddress(hDll, "WlxLoggedOnSAS");
	if (NULL == pfWlxLoggedOnSAS) 
	{
		return FALSE;
	}
	
	pfWlxDisplayLockedNotice = (PFWLXDISPLAYLOCKEDNOTICE) GetProcAddress(hDll, "WlxDisplayLockedNotice");
	if (NULL == pfWlxDisplayLockedNotice) 
	{
		return FALSE;
	}
	
	pfWlxIsLockOk = (PFWLXISLOCKOK) GetProcAddress(hDll, "WlxIsLockOk");
	if (NULL == pfWlxIsLockOk) 
	{
		return FALSE;
	}
	
	pfWlxWkstaLockedSAS = (PFWLXWKSTALOCKEDSAS) GetProcAddress(hDll, "WlxWkstaLockedSAS");
	if (NULL == pfWlxWkstaLockedSAS) 
	{
		return FALSE;
	}
	
	pfWlxIsLogoffOk = (PFWLXISLOGOFFOK) GetProcAddress(hDll, "WlxIsLogoffOk");
	if (NULL == pfWlxIsLogoffOk) 
	{
		return FALSE;
	}
	
	pfWlxLogoff = (PFWLXLOGOFF) GetProcAddress(hDll, "WlxLogoff");
	if (NULL == pfWlxLogoff) 
	{
		return FALSE;
	}
	
	pfWlxShutdown = (PFWLXSHUTDOWN) GetProcAddress(hDll, "WlxShutdown");
	if (NULL == pfWlxShutdown) 
	{
		return FALSE;
	}
	
	pfShellShutdownDialog = (FUNShellShutdownDialog)GetProcAddress(hDll,"ShellShutdownDialog");
	if (NULL == pfShellShutdownDialog)
	{
		return FALSE;
	}	
	
	pfWlxStartApplication = (PFWLXSTARTAPPLICATION) GetProcAddress(hDll, "WlxStartApplication");
	if (NULL == pfWlxStartApplication)
	{
		return FALSE;
	}
	
	pfWlxScreenSaverNotify = (PFWLXSCREENSAVERNOTIFY) GetProcAddress(hDll, "WlxScreenSaverNotify");
	if (NULL == pfWlxScreenSaverNotify)
	{
		return FALSE;
	}
	
	pfWlxNetworkProviderLoad = (PFWLXNETWORKPROVIDERLOAD)GetProcAddress(hDll, "WlxNetworkProviderLoad");
	if (NULL == pfWlxNetworkProviderLoad)
	{
		return FALSE;
	}
	
	pfWlxDisplayStatusMessage = (PFWLXDISPLAYSTATUSMESSAGE)GetProcAddress(hDll, "WlxDisplayStatusMessage");
	if (NULL == pfWlxDisplayStatusMessage)
	{
		return FALSE;
	}
	
	pfWlxGetStatusMessage = (PFWLXGETSTATUSMESSAGE)GetProcAddress(hDll, "WlxGetStatusMessage");
	if (NULL == pfWlxGetStatusMessage)
	{
		return FALSE;
	}
	
	pfWlxRemoveStatusMessage = (PFWLXREMOVESTATUSMESSAGE)GetProcAddress(hDll, "WlxRemoveStatusMessage");
	if (NULL == pfWlxRemoveStatusMessage)
	{
		return FALSE;
	}
	
	if (nVer > 1)
	{
		pfWlxGetConsoleSwitchCredentials = (FUNWLXGetConsoleSwitchCredentials)GetProcAddress(hDll,"WlxGetConsoleSwitchCredentials");
		if (NULL == pfWlxGetConsoleSwitchCredentials)
		{
			return FALSE;
		}
		
		pfWlxReconnectNotify = (FUNWLXReconnectNotify)GetProcAddress(hDll,"WlxReconnectNotify");
		if (NULL == pfWlxReconnectNotify)
		{
			return FALSE;
		}
		
		pfWlxDisconnectNotify = (FUNWLXDisconnectNotify)GetProcAddress(hDll,"WlxDisconnectNotify");
		if (NULL == pfWlxDisconnectNotify)
		{
			return FALSE;
		}	
		
		//使用裸函数.
		for (int i = 0;i < 28; i++)
		{
			gMsGinaFunAddr[i] = (FARPROC)GetProcAddress(hDll,(LPCSTR)MAKEINTRESOURCE(i+1));
		}
	}  
	
	return TRUE;
}

/*
	msgina.dll 导出函数实现.
*/
BOOL 
WINAPI
WlxNegotiate (DWORD   dwWinlogonVersion,
              DWORD * pdwDllVersion)
{	
	HINSTANCE hDll;
	DWORD dwWlxVersion = GINASTUB_VERSION;

#ifdef _DEBUG
	DebugBreak();
#endif
		
	//Load MSGINA.DLL.	
	if (!(hDll = LoadLibrary(TEXT("msgina.dll")))) 
	{
		return FALSE;
	}

	//Load the rest of the WLX functions from the real MSGINA.
	if (!OrgInitialize(hDll,dwWlxVersion))
	{
		return FALSE;
	}
	
	// Handle older version of Winlogon.	
	if (dwWinlogonVersion < dwWlxVersion)
	{
		dwWlxVersion = dwWinlogonVersion;
	}
	
	//Negotiate with MSGINA for version that we can support.	
	if (!pfWlxNegotiate(dwWlxVersion,&dwWlxVersion))
	{
		return FALSE;
	}	
	
	//Inform Winlogon which version to use.	
	*pdwDllVersion = g_dwVersion = dwWlxVersion;

	return TRUE;
}


BOOL
WINAPI
WlxInitialize (LPWSTR  lpWinsta,
               HANDLE  hWlx,
               PVOID   pvReserved,
               PVOID   pWinlogonFunctions,
               PVOID * pWlxContext)
{
	g_pWinlogon = pWinlogonFunctions;	
	
	//挂接函数处理.
	HookWlxDialogBoxParam(g_pWinlogon,g_dwVersion);
	
	return pfWlxInitialize(lpWinsta,
		hWlx,
		pvReserved,
		pWinlogonFunctions,
		pWlxContext);
}

int
WINAPI
WlxLoggedOutSAS (PVOID                pWlxContext,
                 DWORD                dwSasType,
                 PLUID                pAuthenticationId,
                 PSID                 pLogonSid,
                 PDWORD               pdwOptions,
                 PHANDLE              phToken,
                 PWLX_MPR_NOTIFY_INFO pMprNotifyInfo,
                 PVOID *              pProfile)
{
	int iRet;	

	iRet = pfWlxLoggedOutSAS(pWlxContext,
		dwSasType,
		pAuthenticationId,
		pLogonSid,
		pdwOptions,
		phToken,
		pMprNotifyInfo,
		pProfile);	

	return iRet;
}

VOID
WINAPI
WlxDisplaySASNotice (PVOID pWlxContext)
{
	pfWlxDisplaySASNotice(pWlxContext);
}

BOOL
WINAPI
WlxActivateUserShell (PVOID pWlxContext,
                      PWSTR pszDesktopName,
                      PWSTR pszMprLogonScript,
                      PVOID pEnvironment)
{
	return pfWlxActivateUserShell(pWlxContext,
		pszDesktopName,
		pszMprLogonScript,
		pEnvironment);
}


int
WINAPI
WlxLoggedOnSAS (PVOID pWlxContext,
                DWORD dwSasType,
                PVOID pReserved)
{	
	int iRet = pfWlxLoggedOnSAS(pWlxContext,dwSasType,pReserved);
	
	return iRet;
}


VOID
WINAPI
WlxDisplayLockedNotice (PVOID pWlxContext)
{
	pfWlxDisplayLockedNotice(pWlxContext);
}


BOOL
WINAPI
WlxIsLockOk (PVOID pWlxContext)
{
	return pfWlxIsLockOk(pWlxContext);
}


int
WINAPI
WlxWkstaLockedSAS (PVOID pWlxContext,
                   DWORD dwSasType)
{
	
	int iRet = pfWlxWkstaLockedSAS(pWlxContext, dwSasType);
	
	return iRet;
}


BOOL
WINAPI
WlxIsLogoffOk (PVOID pWlxContext)
{
	return pfWlxIsLogoffOk(pWlxContext);	
}


VOID
WINAPI
WlxLogoff (PVOID pWlxContext)
{
	pfWlxLogoff(pWlxContext);
}


VOID
WINAPI
WlxShutdown(PVOID pWlxContext,
            DWORD ShutdownType)
{
	pfWlxShutdown(pWlxContext,ShutdownType);
}

//version 1.2
BOOL
WINAPI
WlxScreenSaverNotify (PVOID  pWlxContext,
                      BOOL * pSecure)
{
	return pfWlxScreenSaverNotify(pWlxContext, pSecure);
}

BOOL
WINAPI
WlxStartApplication (PVOID pWlxContext,
                     PWSTR pszDesktopName,
                     PVOID pEnvironment,
                     PWSTR pszCmdLine)
{
	return pfWlxStartApplication(pWlxContext,
		pszDesktopName,
		pEnvironment,
		pszCmdLine);
}

//version 1.3
BOOL
WINAPI
WlxNetworkProviderLoad (PVOID                pWlxContext,
                        PWLX_MPR_NOTIFY_INFO pNprNotifyInfo)
{
	return pfWlxNetworkProviderLoad(pWlxContext, pNprNotifyInfo);
}


BOOL
WINAPI
WlxDisplayStatusMessage (PVOID pWlxContext,
                         HDESK hDesktop,
                         DWORD dwOptions,
                         PWSTR pTitle,
                         PWSTR pMessage)
{
	return pfWlxDisplayStatusMessage(pWlxContext,
		hDesktop,
		dwOptions,
		pTitle,
		pMessage);
}


BOOL
WINAPI
WlxGetStatusMessage (PVOID   pWlxContext,
                     DWORD * pdwOptions,
                     PWSTR   pMessage,
                     DWORD   dwBufferSize)
{
	return pfWlxGetStatusMessage(pWlxContext,
		pdwOptions,
		pMessage,
		dwBufferSize);
}


BOOL
WINAPI
WlxRemoveStatusMessage (PVOID pWlxContext)
{	
	return pfWlxRemoveStatusMessage(pWlxContext);
}

BOOL 
WINAPI 
WlxGetConsoleSwitchCredentials( 
								  PVOID                                       pWlxContext, 
								  PVOID                                       pCredInfo 
								  )
{
	return pfWlxGetConsoleSwitchCredentials(pWlxContext,pCredInfo);	
}

VOID 
WINAPI 
WlxReconnectNotify( 
					  PVOID                                       pWlxContext 
					  )
{
	pfWlxReconnectNotify(pWlxContext);
}

VOID 
WINAPI 
WlxDisconnectNotify(PVOID  pWlxContext)
{
	pfWlxDisconnectNotify(pWlxContext);		
}

DWORD WINAPI ShellShutdownDialog(HWND hParent, WCHAR *Username, BOOL bHideLogoff)
{
	return pfShellShutdownDialog(hParent,Username,bHideLogoff);	
}

int MyNoNameFun1()
{
	__asm jmp gMsGinaFunAddr[0];
}

int MyNoNameFun2()
{
	__asm jmp gMsGinaFunAddr[1];
}

int MyNoNameFun3()
{
	__asm jmp gMsGinaFunAddr[2];
}

int MyNoNameFun4()
{
	__asm jmp gMsGinaFunAddr[3];
}

int MyNoNameFun5()
{
	__asm jmp gMsGinaFunAddr[4];
}

int MyNoNameFun6()
{
	__asm jmp gMsGinaFunAddr[5];
}

int MyNoNameFun7()
{
	__asm jmp gMsGinaFunAddr[6];
}

int MyNoNameFun8()
{
	__asm jmp gMsGinaFunAddr[7];
}

int MyNoNameFun9()
{
	__asm jmp gMsGinaFunAddr[8];
}

int MyNoNameFun10()
{
	__asm jmp gMsGinaFunAddr[9];
}

int MyNoNameFun11()
{
	__asm jmp gMsGinaFunAddr[10];
}

int MyNoNameFun12()
{
	__asm jmp gMsGinaFunAddr[11];
}

int MyNoNameFun13()
{
	__asm jmp gMsGinaFunAddr[12];
}

int MyNoNameFun14()
{
	__asm jmp gMsGinaFunAddr[13];
}

int MyNoNameFun15()
{
	__asm jmp gMsGinaFunAddr[14];
}

int MyNoNameFun16()
{
	__asm jmp gMsGinaFunAddr[15]
}

int MyNoNameFun17()
{
	__asm jmp gMsGinaFunAddr[16];
}

int MyNoNameFun18()
{
	__asm jmp gMsGinaFunAddr[17];
}

int MyNoNameFun19()
{
	__asm jmp gMsGinaFunAddr[18];
}

int MyNoNameFun20()
{
	__asm jmp gMsGinaFunAddr[19];
}

int MyNoNameFun21()
{
	__asm jmp gMsGinaFunAddr[20];
}

int MyNoNameFun22()
{
	__asm jmp gMsGinaFunAddr[21];
}

int MyNoNameFun23()
{
	__asm jmp gMsGinaFunAddr[22];
}

int MyNoNameFun24()
{
	__asm jmp gMsGinaFunAddr[23];
}

int MyNoNameFun25()
{
	__asm jmp gMsGinaFunAddr[24];
}

int MyNoNameFun26()
{
	__asm jmp gMsGinaFunAddr[25];
}

int MyNoNameFun27()
{
	__asm jmp gMsGinaFunAddr[26];
}

int MyNoNameFun28()
{
	__asm jmp gMsGinaFunAddr[27];
}