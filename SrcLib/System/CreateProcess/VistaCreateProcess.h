// VistaCreateProcess.h: interface for the CVistaCreateProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VISTACREATEPROCESS_H__D39B2127_1987_4FFB_9851_DA8C82AB23DC__INCLUDED_)
#define AFX_VISTACREATEPROCESS_H__D39B2127_1987_4FFB_9851_DA8C82AB23DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <Userenv.h>

#include <shellapi.h>
typedef struct _UNICODE_STRING {
  USHORT  Length;
  USHORT  MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING,*PUNICODE_STRING;

//////////////////////////////////////////////////////////////////////////////////

typedef LONG (WINAPI *PFNRtlConvertSidToUnicodeString)(
  __out         PUNICODE_STRING UnicodeString,
  __in          PSID Sid,
  __in          BOOLEAN AllocateDestinationString
);

typedef DWORD (WINAPI *fnWTSGetActiveConsoleSessionId)(VOID);

typedef BOOL (WINAPI *fnOpenProcessToken)(
  HANDLE ProcessHandle,
  DWORD DesiredAccess,
  PHANDLE TokenHandle
);

typedef BOOL (WINAPI *fnWTSQueryUserToken)(
  ULONG   SessionId, 
  PHANDLE phToken
);

typedef BOOL (WINAPI *fnDuplicateTokenEx)(
  HANDLE hExistingToken,                           // handle to token to duplicate
  DWORD dwDesiredAccess,                           // access rights of new token
  LPSECURITY_ATTRIBUTES lpTokenAttributes,         // attributes
  SECURITY_IMPERSONATION_LEVEL ImpersonationLevel, // level
  TOKEN_TYPE TokenType,                            // primary or impersonation token
  PHANDLE phNewToken                               // handle to duplicated token
);

typedef BOOL (WINAPI *fnSetTokenInformation)(
  HANDLE TokenHandle,                            // handle to access token
  TOKEN_INFORMATION_CLASS TokenInformationClass, // type
  LPVOID TokenInformation,                       // buffer
  DWORD TokenInformationLength                   // size of buffer
);

typedef BOOL (WINAPI *fnCreateEnvironmentBlock)(
  LPVOID *lpEnvironment,  // environment block
  HANDLE hToken,          // user token
  BOOL bInherit           // inheritance
);
typedef BOOL (WINAPI *fnDestroyEnvironmentBlock)(
									 LPVOID lpEnvironment  // environment block
									 );

typedef BOOL (WINAPI *fnCreateProcessAsUserW)(
  HANDLE hToken,                             // handle to user token
  LPCWSTR lpApplicationName,                 // name of executable module
  LPWSTR lpCommandLine,                      // command-line string
  LPSECURITY_ATTRIBUTES lpProcessAttributes, // SD
  LPSECURITY_ATTRIBUTES lpThreadAttributes,  // SD
  BOOL bInheritHandles,                      // inheritance option
  DWORD dwCreationFlags,                     // creation flags
  LPVOID lpEnvironment,                      // new environment block
  LPCWSTR lpCurrentDirectory,                // current directory name
  LPSTARTUPINFOW lpStartupInfo,               // startup information
  LPPROCESS_INFORMATION lpProcessInformation // process information
);

typedef BOOL (WINAPI *PFNConvertSidToStringSid)(
											   PSID Sid,
											   LPTSTR *StringSid
											   );
typedef 
BOOL (WINAPI *PFNGetTokenInformation)(
										HANDLE TokenHandle,
										TOKEN_INFORMATION_CLASS TokenInformationClass,
										LPVOID TokenInformation,
										DWORD TokenInformationLength,
										PDWORD ReturnLength
										);

typedef 
VOID (WINAPI *PFNRtlFreeUnicodeString)(IN PUNICODE_STRING  UnicodeString);

typedef 
BOOL (WINAPI *PFNLoadUserProfile)(
									HANDLE hToken,               // user token
									LPPROFILEINFOA lpProfileInfo  // profile
								 );

typedef
BOOL (WINAPI *PFNUnloadUserProfile)(
									HANDLE hToken,   // user token
									HANDLE hProfile  // handle to registry key
									);
typedef
BOOL (WINAPI *PFNCreateProcessAsUserA)(
						 HANDLE hToken,                             // handle to user token
						 LPCSTR lpApplicationName,                 // name of executable module
						 LPSTR lpCommandLine,                      // command-line string
						 LPSECURITY_ATTRIBUTES lpProcessAttributes, // SD
						 LPSECURITY_ATTRIBUTES lpThreadAttributes,  // SD
						 BOOL bInheritHandles,                      // inheritance option
						 DWORD dwCreationFlags,                     // creation flags
						 LPVOID lpEnvironment,                      // new environment block
						 LPCSTR lpCurrentDirectory,                // current directory name
						 LPSTARTUPINFOA lpStartupInfo,               // startup information
						 LPPROCESS_INFORMATION lpProcessInformation // process information
						 );
typedef 
BOOL (WINAPI *PFNLookupAccountSidA)(
					  LPCSTR lpSystemName,  // name of local or remote computer
					  PSID Sid,              // security identifier
					  LPSTR Name,           // account name buffer
					  LPDWORD cbName,        // size of account name buffer
					  LPSTR DomainName,     // domain name
					  LPDWORD cbDomainName,  // size of domain name buffer
					  PSID_NAME_USE peUse    // SID type
					  );

///////////////////////////////////////////////////////////////////////////////////////////////
class CVistaCreateProcess  
{
public:
	HANDLE GetCurrentTokenByExplorer();
	BOOL GetCurrentLogonUserNameNew(HANDLE m_hToken, LPSTR lpszUserName);
	BOOL CreateProcessLogonUser2K(LPSTR m_Command,BOOL bWaiting = FALSE, DWORD *lpExitCode = NULL);
	BOOL GetCurrentLogonUserName(HANDLE m_hToken, LPSTR lpszUserName);
	BOOL CreateProcessLogonUser(LPSTR m_Command,BOOL bShow,BOOL bWaiting = FALSE, DWORD *lpExitCode = NULL);
	HMODULE m_hDll4;
	HMODULE m_hDll3;
	BOOL CreateLowProcess(LPSTR m_Command);
	BOOL CreateProcessFromService(LPCSTR m_Command, BOOL bShow,BOOL bWaiting = FALSE, DWORD *lpExitCode = NULL);
	HMODULE m_hDll2;
	HMODULE m_hDll;
	BOOL InitFunctionVar( );
	PFNCreateProcessAsUserA				lpCreateProcessAsUserA;
	PFNUnloadUserProfile				lpUnloadUserProfile;
	PFNLoadUserProfile					lpLoadUserProfile;
	PFNGetTokenInformation				lpGetTokenInformation;
	PFNRtlConvertSidToUnicodeString		lpRtlConvertSidToUnicodeString;
	PFNRtlFreeUnicodeString				lpRtlFreeUnicodeString;
	fnWTSGetActiveConsoleSessionId		LpWTSGetActiveConsoleSessionId;
	fnOpenProcessToken					LpfnOpenProcessToken;
	fnDuplicateTokenEx					LpfnDuplicateTokenEx;
	fnSetTokenInformation				LpfnSetTokenInformation;
	fnCreateEnvironmentBlock			LpfnCreateEnvironmentBlock;
	fnDestroyEnvironmentBlock			lpfnDestroyEnvironmentBlock;
	fnCreateProcessAsUserW				LpfnCreateProcessAsUserW;
	fnWTSQueryUserToken					LpfnWTSQueryUserToken;
	PFNLookupAccountSidA				lpLookupAccountSidA;
	BOOL RunElevated( HWND	hwnd, LPCSTR pszPath, LPCSTR pszParameters, LPCSTR pszDirectory );
	BOOL MyShellExec(	HWND hwnd, LPCSTR pszVerb, LPCSTR pszPath, LPCSTR pszParameters, LPCSTR pszDirectory );
	BOOL IsVista();
	CVistaCreateProcess();
	virtual ~CVistaCreateProcess();

};

#endif // !defined(AFX_VISTACREATEPROCESS_H__D39B2127_1987_4FFB_9851_DA8C82AB23DC__INCLUDED_)
