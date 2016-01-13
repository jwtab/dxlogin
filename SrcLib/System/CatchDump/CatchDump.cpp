
#include "stdafx.h"

#include "CatchDump.h"

CCatchDumpFile * CCatchDumpFile::g_This = NULL;

CCatchDumpFile::CCatchDumpFile(wchar_t * pwszDumpFilePath)
{
	m_hDebugHelpDll = NULL;
	m_pFun_MiniDumpWriteDump = NULL;
	m_pFun_SetUnhandledExceptionFilter = NULL;

	g_This = this;

	if (NULL == pwszDumpFilePath)
	{
		GetModuleFileNameW(NULL,m_wszDumFilePath,MAX_PATH);
		wcscat_s(m_wszDumFilePath,MAX_PATH,L".dmp");
	}
	else
	{
		wcscpy_s(m_wszDumFilePath,MAX_PATH,pwszDumpFilePath);
	}
}

CCatchDumpFile::~CCatchDumpFile()
{
	if (NULL != m_hDebugHelpDll)
	{
		FreeLibrary(m_hDebugHelpDll);
		m_hDebugHelpDll = NULL;
	}
}

void CCatchDumpFile::DoCatchDump()
{
	m_hDebugHelpDll = LoadLibraryW(L"dbghelp.dll");
	if (NULL != m_hDebugHelpDll)
	{
		m_pFun_MiniDumpWriteDump = (PFUN_MiniDumpWriteDump)GetProcAddress(m_hDebugHelpDll,"MiniDumpWriteDump");
		m_pFun_SetUnhandledExceptionFilter = (PFUN_SetUnhandledExceptionFilter)GetProcAddress(GetModuleHandleW(L"Kernel32.dll"),
			"SetUnhandledExceptionFilter");
	}

	if (NULL != m_pFun_SetUnhandledExceptionFilter)
	{
		m_pFun_SetUnhandledExceptionFilter(AppCrashHandler);
	}	
}

LONG CCatchDumpFile::AppCrashHandler(EXCEPTION_POINTERS *pException)
{	
	g_This->CreateDumpFile(g_This->m_wszDumFilePath,pException);

	return EXCEPTION_EXECUTE_HANDLER;  
}

void CCatchDumpFile::CreateDumpFile(wchar_t * lpstrDumpFilePathName, EXCEPTION_POINTERS *pException)
{
	//创建Dump文件
	HANDLE hDumpFile = CreateFile(lpstrDumpFilePathName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE != hDumpFile &&
		NULL != m_pFun_MiniDumpWriteDump)
	{
		//Dump信息
		MINIDUMP_EXCEPTION_INFORMATION dumpInfo;  
		dumpInfo.ExceptionPointers = pException;  
		dumpInfo.ThreadId = GetCurrentThreadId();  
		dumpInfo.ClientPointers = TRUE;  

		//写入Dump文件内容
		m_pFun_MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);  

		CloseHandle(hDumpFile);  
		hDumpFile = NULL;
	}	
}