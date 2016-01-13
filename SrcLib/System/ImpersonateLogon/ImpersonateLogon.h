// ImpersonateLogon.h: interface for the CImpersonateLogon class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMPERSONATELOGON_H__F6F3C46D_0D53_4EC3_99A1_1F6C4A0D4F9F__INCLUDED_)
#define AFX_IMPERSONATELOGON_H__F6F3C46D_0D53_4EC3_99A1_1F6C4A0D4F9F__INCLUDED_

	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Windows.h>

enum OSTYPE
{
	OsUnknown,
	WINDOWS9X,
	WINDOWS2K,
	XpLater
};

typedef DWORD (WINAPI *PFNWTSGetActiveConsoleSessionId)();

typedef BOOL (WINAPI *PFNOpenProcessToken)(
	HANDLE ProcessHandle,
	DWORD DesiredAccess,
	PHANDLE TokenHandle
	);

typedef BOOL (WINAPI *PFNWTSQueryUserToken)(
	ULONG SessionId, 
	PHANDLE phToken 
	);


typedef BOOL (WINAPI *PFNDuplicateTokenEx)(
	HANDLE hExistingToken,
	DWORD dwDesiredAccess,
	LPSECURITY_ATTRIBUTES lpTokenAttributes,
	SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
	TOKEN_TYPE TokenType,
	PHANDLE phNewToken
	);

typedef BOOL (WINAPI *PFNImpersonateLoggedOnUser)(
	HANDLE hToken
	);

typedef BOOL (WINAPI *PFNRevertToSelf)(void);

typedef BOOL (WINAPI *PFNConvertSidToStringSid)(
	PSID Sid,
	LPTSTR *StringSid
	);

typedef BOOL (WINAPI *PFNGetTokenInformation)(
	HANDLE TokenHandle,
	TOKEN_INFORMATION_CLASS TokenInformationClass,
	LPVOID TokenInformation,
	DWORD TokenInformationLength,
	PDWORD ReturnLength
	);


typedef struct _LSA_UNICODE_STRING
{
	USHORT  Length;
	USHORT  MaximumLength;
	PWSTR   Buffer;
}LSA_UNICODE_STRING,*PLSA_UNICODE_STRING;
typedef LSA_UNICODE_STRING UNICODE_STRING, *PUNICODE_STRING;


typedef DWORD (WINAPI *PFNRtlConvertSidToUnicodeString)(
	PUNICODE_STRING UnicodeString,
	PSID Sid,
	BOOLEAN AllocateDestinationString);


typedef VOID (WINAPI *PFNRtlFreeUnicodeString)(IN PUNICODE_STRING  UnicodeString);

//在服务模式下使用的类
class CImpersonateLogon  
{
public:
	HANDLE GetCurrentTokenByExplorer();
	OSTYPE GetOsVersion();
	void InitLibrary( );
	BOOL m_Logon;
	BOOL QueryUserSID( );
	char m_szSID[1024*4];
	//在服务模式下可以取得用户的SID
	BOOL GetUserSID(char *lpszSID, DWORD m_Size);
	void LogOut();
	HANDLE TokenDup( );
	HMODULE hNtdll;
	HMODULE hKernel32;
	HMODULE hWtsapi32;
	HMODULE hAdvapi32;
	//在服务模式下可以登录成功
	BOOL SessionLogon ( );
	PFNRtlFreeUnicodeString lpRtlFreeUnicodeString;
	PFNRtlConvertSidToUnicodeString lpRtlConvertSidToUnicodeString;
	PFNGetTokenInformation lpGetTokenInformation;
	PFNRevertToSelf lpRevertToSelf;
	PFNDuplicateTokenEx lpDuplicateTokenEx;
	PFNImpersonateLoggedOnUser lpImpersonateLoggedOnUser;
	PFNWTSQueryUserToken lpWTSQueryUserToken;
	PFNWTSGetActiveConsoleSessionId lpWTSGetActiveConsoleSessionId;
	PFNOpenProcessToken	lpOpenProcessToken;
	CImpersonateLogon(BOOL m_bNeedLogon);
	CImpersonateLogon();
	virtual ~CImpersonateLogon();

private:
	HANDLE _hToken;
	HANDLE _hTokenDup;
	OSTYPE _OsVersion;
};

#endif // !defined(AFX_IMPERSONATELOGON_H__F6F3C46D_0D53_4EC3_99A1_1F6C4A0D4F9F__INCLUDED_)
