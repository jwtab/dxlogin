
#ifndef SYS_OS_H_
#define SYS_OS_H_

#include <Windows.h>
#include <objbase.h>
#include <shlwapi.h>

typedef LONG (NTAPI* FUN_RtlGetVersion)(PRTL_OSVERSIONINFOW lpVersionInformation);
typedef void (WINAPI * FUN_GetNativeSystemInfo)(LPSYSTEM_INFO lpSystemInfo);

//Windows boot type
const int WIN_BOOT_TYPE_NORMAL   = 1;
const int WIN_BOOT_TYPE_SAFE     = 2;
const int WIN_BOOT_TYPE_SAFE_NET = 3;

typedef enum OS_TYPE{
	OS_UNK,
	OS_2K,
	OS_XP,
	OS_2K3,
	OS_VISTA,
	OS_WIN7,
	OS_WIN8,
	OS_WIN10
};

namespace SystemX
{
	class CXSysOS
	{
	public:
		//windows version
		static BOOL IsBit64System();
		static BOOL IsVistaOS();
		static int GetOSType();

		//process
		static BOOL RunProcessByParent(wchar_t * command,int nShow = SW_SHOW,BOOL bWait = FALSE);

		//guids
		static BOOL GetRandomGUID(wchar_t * pwszGuid);

		//unicode char.
		static void CharToWChar(const char * pszSrc,wchar_t * pwszDest,int iSrcLen = 0,UINT CodePage = CP_UTF8);
		static void WCharToChar(const wchar_t * pwszSrc,char * pszDest,int iSrcLen = 0,UINT CodePage = CP_UTF8);

		//window boot type
		static int GeWindowstBootType();

		static void GetWorkingDir(wchar_t * pwszWorkDir);
	};
}

#endif