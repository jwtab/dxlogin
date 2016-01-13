
#include "stdafx.h"

#include "NTService.h"

CNTService* CNTService::g_This = NULL;

CNTService::CNTService(const wchar_t * pwszSvrName,const wchar_t * pwszMutexName)
{
	if (NULL != pwszSvrName)
	{
		swprintf_s(m_wszSvrName,128,L"%s",pwszSvrName);
	}
	else
	{
		swprintf_s(m_wszSvrName,128,L"%s",L"My service");
	}

	if (NULL == pwszMutexName)
	{
		wcscpy_s(m_wszMutexName,128,MUTEX_ONLY_NAME);
	}
	else
	{
		wcscpy_s(m_wszMutexName,128,pwszMutexName);
	}

	g_This = this;

	m_hStopEvent = NULL;
	m_hOnlyMutex = NULL;

	m_lpWorkDisp = NULL;
	m_lpWorkDispHandles = NULL;
	m_nWorkDispCount = 0;

	m_dwCheckPoint = 0;
}

CNTService::~CNTService()
{
	if (NULL != m_hStopEvent)
	{
		CloseHandle(m_hStopEvent);
		m_hStopEvent = NULL;
	}

	if (NULL != m_hOnlyMutex)
	{
		CloseHandle(m_hOnlyMutex);
		m_hOnlyMutex = NULL;
	}

	if (NULL != m_lpWorkDisp)
	{
		delete [] m_lpWorkDisp;
		m_lpWorkDisp = NULL;
	}

	if (NULL != m_lpWorkDispHandles)
	{
		delete [] m_lpWorkDispHandles;
		m_lpWorkDispHandles = NULL;
	}
}

void CNTService::SetWorkDispatch(LPSERVICE_WORK_DISPATCH lpWorkDisp,int nDispNum)
{
	m_lpWorkDisp = new SERVIC_WORK_DISPATCH[nDispNum];
	if (NULL != lpWorkDisp)
	{
		m_lpWorkDispHandles = new HANDLE[nDispNum];
		ZeroMemory(m_lpWorkDispHandles,sizeof(HANDLE)*nDispNum);

		ZeroMemory(m_lpWorkDisp,sizeof(SERVIC_WORK_DISPATCH)*nDispNum);

		m_nWorkDispCount = nDispNum;
		for (int index = 0; index < nDispNum; index++)
		{
			m_lpWorkDisp[index].pDisHandler = lpWorkDisp[index].pDisHandler;
			m_lpWorkDisp[index].lpParam = lpWorkDisp[index].lpParam;
			wcscpy_s(m_lpWorkDisp[index].wszStopEventName,128,lpWorkDisp[index].wszStopEventName);

			m_lpWorkDispHandles[index] = NULL;
		}
	}
}

BOOL CNTService::ServiceRunning()
{	
	SERVICE_TABLE_ENTRY DispatchTable[] = 
	{ 
		{m_wszSvrName,(LPSERVICE_MAIN_FUNCTION)ServiceMain}, 
		{NULL,NULL} 
	};

	if (!StartServiceCtrlDispatcher(DispatchTable)) 
	{ 
		OutputDebugStringW(L"SVR: StartServiceCtrlDispatcher() Error!");
		return FALSE;
	}

	return TRUE;
}

void CNTService::ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{	
	g_This->m_hSvrStatusHandle = RegisterServiceCtrlHandler( 
		g_This->m_wszSvrName, 
		SvcCtrlHandler);
	if(NULL == g_This->m_hSvrStatusHandle)
	{ 
		OutputDebugStringW(L"SVR: RegisterServiceCtrlHandler() Error!");
		return; 
	} 

	g_This->m_SvrStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
	g_This->m_SvrStatus.dwServiceSpecificExitCode = 0;

	g_This->ReportSvcStatus(SERVICE_START_PENDING,NO_ERROR,1000);

	g_This->Running(dwArgc,lpszArgv);
}

void CNTService::Running(DWORD dwArgc,LPTSTR *lpszArgv)
{
	int index = 0;

	m_hStopEvent = CreateEventW(NULL,    // default security attributes
		TRUE,    // manual reset event
		FALSE,   // not signaled
		NULL);   // no name
	if (NULL == m_hStopEvent)
	{
		ReportSvcStatus(SERVICE_STOPPED,NO_ERROR,0);
		return;
	}

	m_hOnlyMutex = CreateMutex(NULL,FALSE,m_wszMutexName);
	if (NULL != m_hOnlyMutex)
	{
		if (ERROR_ALREADY_EXISTS == GetLastError())
		{
			ReportSvcStatus(SERVICE_STOPPED,NO_ERROR,0);
			return;
		}
	}
	else
	{
		ReportSvcStatus(SERVICE_STOPPED,NO_ERROR,0);
		return;
	}

	//Report running status when initialization is complete.
	ReportSvcStatus(SERVICE_RUNNING,NO_ERROR,0);

	//Perform work until service stops.
	for (index = 0; index < m_nWorkDispCount; index++)
	{
		m_lpWorkDispHandles[index] = CreateThread(NULL,0,m_lpWorkDisp[index].pDisHandler,
			m_lpWorkDisp[index].lpParam,0,NULL);
		if (NULL != m_lpWorkDispHandles[index])
		{
			OutputDebugStringW(L"SVR: CreateThread() OK!");
		}
	}

	OutputDebugStringW(L"SVR: call WaitForSingleObject()!");

	//Check whether to stop the service.
	WaitForSingleObject(m_hStopEvent,INFINITE);

	OutputDebugStringW(L"SVR: WaitForSingleObject() OK!");
	for (index = 0; index < m_nWorkDispCount; index++)
	{
		//stop thread by event_name.
		if (NULL != m_lpWorkDispHandles[index])
		{
			HANDLE hThreadStopEvent = OpenEventW(EVENT_MODIFY_STATE,FALSE,m_lpWorkDisp[index].wszStopEventName);
			if (NULL != hThreadStopEvent)
			{
				SetEvent(hThreadStopEvent);

				WaitForSingleObject(m_lpWorkDispHandles[index],INFINITE);

				CloseHandle(hThreadStopEvent);
				hThreadStopEvent = NULL;
			}

			CloseHandle(m_lpWorkDispHandles[index]);
			m_lpWorkDispHandles[index] = NULL;
		}			
	}

	OutputDebugStringW(L"SVR: Service over!");

	ReportSvcStatus(SERVICE_STOPPED,NO_ERROR,0);
}

void CNTService::ReportSvcStatus(DWORD dwCurrentState,DWORD dwWin32ExitCode,DWORD dwWaitHint)
{
	//Fill in the SERVICE_STATUS structure.
	m_SvrStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
	m_SvrStatus.dwServiceSpecificExitCode = 0;
	m_SvrStatus.dwCurrentState = dwCurrentState;
	m_SvrStatus.dwWin32ExitCode = dwWin32ExitCode;
	m_SvrStatus.dwWaitHint = dwWaitHint;

	if (dwCurrentState == SERVICE_START_PENDING)
	{
		m_SvrStatus.dwControlsAccepted = 0;
	}
	else 
	{
		m_SvrStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	}

	if ( (dwCurrentState == SERVICE_RUNNING) ||
		(dwCurrentState == SERVICE_STOPPED) )
	{
		m_SvrStatus.dwCheckPoint = 0;
	}
	else
	{
		m_SvrStatus.dwCheckPoint = m_dwCheckPoint++;
	}

	//Report the status of the service to the SCM.
	SetServiceStatus(m_hSvrStatusHandle,&m_SvrStatus);
}

void CNTService::SvcCtrlHandler(DWORD dwCtrlCode)
{
	DWORD	status=0;

	OutputDebugStringW(L"SVR: call SvcCtrlHandler()!");

	switch(dwCtrlCode) 
	{
	case SERVICE_CONTROL_STOP:
		{
			g_This->ReportSvcStatus(SERVICE_STOP_PENDING,NO_ERROR,0);
			SetEvent(g_This->m_hStopEvent);
			g_This->ReportSvcStatus(g_This->m_SvrStatus.dwCurrentState,NO_ERROR,0);

			break;
		}

	case SERVICE_CONTROL_PAUSE:
		{
			break;
		}		

	case SERVICE_CONTROL_CONTINUE:
		{
			break;
		}		

	default: 
		{
			break;
		}
	}	
}