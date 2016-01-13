
#ifndef CATCH_DUMP_H_
#define CATCH_DUMP_H_

#include <Windows.h>
#include <Dbghelp.h>


typedef BOOL (WINAPI * PFUN_MiniDumpWriteDump)(
	__in HANDLE hProcess,
	__in DWORD ProcessId,
	__in HANDLE hFile,
	__in MINIDUMP_TYPE DumpType,
	__in_opt PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	__in_opt PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	__in_opt PMINIDUMP_CALLBACK_INFORMATION CallbackParam
	);

typedef LPTOP_LEVEL_EXCEPTION_FILTER (WINAPI *PFUN_SetUnhandledExceptionFilter)(
	__in          LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter
	);

class CCatchDumpFile
{
public:
	CCatchDumpFile(wchar_t * pwszDumpFilePath = NULL);
	~CCatchDumpFile();

public:
	void DoCatchDump();

	static LONG WINAPI  AppCrashHandler(EXCEPTION_POINTERS *pException);
	static CCatchDumpFile * g_This;

private:
	void CreateDumpFile(wchar_t *lpstrDumpFilePathName, EXCEPTION_POINTERS *pException);

private:
	HMODULE m_hDebugHelpDll;
	PFUN_MiniDumpWriteDump m_pFun_MiniDumpWriteDump;
	PFUN_SetUnhandledExceptionFilter m_pFun_SetUnhandledExceptionFilter;

	wchar_t m_wszDumFilePath[MAX_PATH];
};

#endif