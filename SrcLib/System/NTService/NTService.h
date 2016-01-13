
#ifndef NT_SERVICE_H_
#define NT_SERVICE_H_

#include <Windows.h>
#include <stdio.h>

/*                               */
#define MUTEX_ONLY_NAME L"Global\\GUID(7C58E4BC-24AD-491E-949D-7789730AB5A9)"

typedef DWORD (WINAPI *LP_Dispatch_Handler)(LPVOID lpParam);
typedef struct Service_Work_Dispatch
{
	LP_Dispatch_Handler pDisHandler;
	LPVOID lpParam;

	wchar_t wszStopEventName[128];
}SERVIC_WORK_DISPATCH,*LPSERVICE_WORK_DISPATCH;

class CNTService
{
public:
	CNTService(const wchar_t * pwszSvrName,const wchar_t * pwszMutexName = NULL);
	~CNTService();

public:
	void SetWorkDispatch(LPSERVICE_WORK_DISPATCH lpWorkDisp,int nDispNum);
	BOOL ServiceRunning();

private:
	static void WINAPI ServiceMain(DWORD dwArgc,LPTSTR *lpszArgv);
	static void WINAPI SvcCtrlHandler(DWORD dwCtrlCode);
	static CNTService *g_This;

private:
	void ReportSvcStatus(DWORD dwCurrentState,DWORD dwWin32ExitCode,DWORD dwWaitHint);
	void Running(DWORD dwArgc,LPTSTR *lpszArgv);

private:
	SERVICE_STATUS m_SvrStatus;
	SERVICE_STATUS_HANDLE m_hSvrStatusHandle;
	DWORD m_dwCheckPoint;

	wchar_t m_wszSvrName[128];
	wchar_t m_wszMutexName[128];

	HANDLE m_hStopEvent;
	HANDLE m_hOnlyMutex;

	LPSERVICE_WORK_DISPATCH m_lpWorkDisp;
	HANDLE * m_lpWorkDispHandles;
	int m_nWorkDispCount;
};

#endif