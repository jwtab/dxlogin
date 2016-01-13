
#include "stdafx.h"

#include "ServiceCtrl.h"

CNTServiceHandler::CNTServiceHandler()
{
	m_hSCManager = NULL;
	m_dwLastError = 0;

	InitSCManager();
}

CNTServiceHandler::~CNTServiceHandler()
{
	UniniSCManager();
}

BOOL CNTServiceHandler::InitSCManager()
{
	BOOL bRetFun = TRUE;

	m_hSCManager = OpenSCManagerW(NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 
	if (NULL == m_hSCManager)
	{
		m_dwLastError = GetLastError();

		bRetFun = FALSE;
	}

	return bRetFun;
}

BOOL CNTServiceHandler::UniniSCManager()
{
	BOOL bRetFun = TRUE;

	if (!CloseServiceHandle(m_hSCManager))
	{
		m_dwLastError = GetLastError();

		bRetFun = FALSE;
	}

	return bRetFun;	
}

BOOL CNTServiceHandler::InstallService(const wchar_t * wszSrvName,const wchar_t * wszExePath)
{
	BOOL bRetFun = TRUE;

	SC_HANDLE hService = NULL;
	wchar_t wszFilePath[MAX_PATH] = {0};	

	if (IsInstalled(wszSrvName))
	{
		UnInstallService(wszSrvName);
	}

	if (NULL == wszExePath)
	{
		GetModuleFileNameW(NULL,wszFilePath,MAX_PATH);
	}
	else
	{
		wcscpy_s(wszFilePath,MAX_PATH,wszExePath);
	}

	hService = CreateServiceW(m_hSCManager,              //SCM database 
		wszSrvName,                   //name of service 
		wszSrvName,                   //service name to display 
		SERVICE_ALL_ACCESS,        //desired access 
		SERVICE_WIN32_OWN_PROCESS, //service type 
		SERVICE_AUTO_START,      // start type 
		SERVICE_ERROR_NORMAL,      // error control type 
		wszFilePath,                // path to service's binary 
		NULL,                      // no load ordering group 
		NULL,                      // no tag identifier 
		NULL,                      // no dependencies 
		NULL,                      // LocalSystem account 
		NULL);                     // no password 
	if (NULL == hService)
	{
		bRetFun = FALSE;
		m_dwLastError = GetLastError();
	}

	if (NULL != hService)
	{
		CloseServiceHandle(hService); 
		hService = NULL;
	}

	return bRetFun;
}

BOOL CNTServiceHandler::UnInstallService(const wchar_t * wszSrvName)
{
	BOOL bRetFun = TRUE;

	SC_HANDLE hService = NULL;

	if (IsInstalled(wszSrvName))
	{
		DoStopSvc(wszSrvName);

		hService = OpenServiceW(m_hSCManager,       // SCM database 
			wszSrvName,          // name of service 
			DELETE);            // need delete access
		if (NULL == hService)
		{
			bRetFun = FALSE;
			m_dwLastError = GetLastError();
		}
		else
		{
			if (!DeleteService(hService)) 
			{
				bRetFun = FALSE;
				m_dwLastError = GetLastError();
			}
		}
	}
	else
	{
		bRetFun = TRUE;
	}

	if (NULL != hService)
	{
		CloseServiceHandle(hService); 
		hService = NULL;
	}

	return bRetFun;
}

BOOL CNTServiceHandler::DoStartSvc(const wchar_t * wszSrvName)
{
	BOOL bRetFun = TRUE;

	SC_HANDLE hService = NULL;

	SERVICE_STATUS_PROCESS ssStatus; 
	DWORD dwBytesNeeded;

	hService = OpenServiceW(m_hSCManager,         // SCM database 
		wszSrvName,            // name of service 
		SERVICE_ALL_ACCESS);  // full access
	if (hService == NULL)
	{ 
		bRetFun = FALSE;
	}    
	else
	{
		if (!QueryServiceStatusEx(hService,  // handle to service 
			SC_STATUS_PROCESS_INFO,         // information level
			(LPBYTE)&ssStatus,             // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&dwBytesNeeded))              // size needed if buffer is too small
		{
			bRetFun = FALSE;
			m_dwLastError = GetLastError();
		}
		else
		{
			if(ssStatus.dwCurrentState != SERVICE_RUNNING)
			{
				if (!StartServiceW(hService,  //handle to service 
					0,           //number of arguments 
					NULL))      //no arguments 
				{
					bRetFun = FALSE;
					m_dwLastError = GetLastError();
				}
			}			
		}
	}

	if (NULL != hService)
	{
		CloseServiceHandle(hService);
		hService = NULL;
	}

	return bRetFun;
}

BOOL CNTServiceHandler::DoStopSvc(const wchar_t * wszSrvName)
{
	BOOL bRetFun = TRUE;

	SC_HANDLE hService = NULL;

	SERVICE_STATUS_PROCESS ssStatus; 
	DWORD dwBytesNeeded;

	hService = OpenServiceW(m_hSCManager,  // SCM database 
		wszSrvName,            // name of service 
		SERVICE_ALL_ACCESS);  // full access
	if (hService == NULL)
	{ 
		bRetFun = FALSE;
		m_dwLastError = GetLastError();
	}    
	else
	{
		if (!QueryServiceStatusEx(hService,  //handle to service 
			SC_STATUS_PROCESS_INFO,         //information level
			(LPBYTE)&ssStatus,             //address of structure
			sizeof(SERVICE_STATUS_PROCESS), //size of structure
			&dwBytesNeeded))              //size needed if buffer is too small
		{
			bRetFun = FALSE;
		}
		else
		{
			if(ssStatus.dwCurrentState != SERVICE_STOPPED)
			{
				if (!ControlService(hService, 
					SERVICE_CONTROL_STOP, 
					(LPSERVICE_STATUS)&ssStatus))
				{
					bRetFun = FALSE;
					m_dwLastError = GetLastError();
				}
			}			
		}
	}

	if (NULL != hService)
	{
		CloseServiceHandle(hService);
		hService = NULL;
	}

	return bRetFun;
}

BOOL CNTServiceHandler::IsInstalled(const wchar_t * wszSrvName)
{
	BOOL isInstalled = TRUE;

	SC_HANDLE hService = NULL;

	hService = OpenServiceW(m_hSCManager,
		wszSrvName,
		SERVICE_QUERY_CONFIG);
	if (NULL != hService)
	{
		CloseServiceHandle(hService); 
		hService = NULL;
	}
	else
	{
		m_dwLastError = GetLastError();
		isInstalled = FALSE;
	}

	return isInstalled;
}

void CNTServiceHandler::SetSvrDescription(const wchar_t * wszSrvName,const wchar_t * pwszDescription)
{
	if (NULL == pwszDescription)
	{
		return;
	}

	wchar_t wszSubKey[MAX_PATH] = {0};	
	HKEY hKey = NULL;

	swprintf_s(wszSubKey,L"SYSTEM\\CurrentControlSet\\services\\%s",wszSrvName);
	LONG lRet = RegOpenKeyExW(HKEY_LOCAL_MACHINE,wszSubKey,0,KEY_ALL_ACCESS,&hKey);
	if (NULL != hKey)
	{
		RegSetValueExW(hKey,L"Description",0,REG_SZ,(BYTE*)(pwszDescription),wcslen(pwszDescription)*2);

		RegCloseKey(hKey);
		hKey = NULL;
	}
}