
#ifndef SERVICE_CTRL_H_
#define SERVICE_CTRL_H_

#include <Windows.h>
#include <stdio.h>

class CNTServiceHandler
{
public:
	CNTServiceHandler();
	~CNTServiceHandler();

public:
	BOOL InstallService(const wchar_t * wszSrvName,const wchar_t * wszExePath);
	BOOL UnInstallService(const wchar_t * wszSrvName);

	BOOL DoStartSvc(const wchar_t * wszSrvName);
	BOOL DoStopSvc(const wchar_t * wszSrvName);

	BOOL IsInstalled(const wchar_t * wszSrvName);

	void SetSvrDescription(const wchar_t * wszSrvName,const wchar_t * pwszDescription);

private:
	BOOL InitSCManager();
	BOOL UniniSCManager();	

private:
	SC_HANDLE m_hSCManager;

	DWORD m_dwLastError;
};

#endif