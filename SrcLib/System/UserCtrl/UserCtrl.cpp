
#include "stdafx.h"

#include "UserCtrl.h"

CUserCtrl::CUserCtrl(const wchar_t * pDomainName)
{
	if (NULL != pDomainName)
	{
		m_wstrDomainName = pDomainName;
	}
	else
	{
		m_wstrDomainName = L"";
	}
}

CUserCtrl::~CUserCtrl()
{

}

BOOL CUserCtrl::GetAllUsersInfo(USER_INFO_VECTOR &vc)
{
	BOOL bRet = TRUE;

	LPUSER_INFO_0 pBuf = NULL;
	LPUSER_INFO_0 pTmpBuf;
	DWORD dwLevel = 0;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;		
	NET_API_STATUS nStatus;

	do
	{
		nStatus = NetUserEnum(m_wstrDomainName.length() > 1 ?m_wstrDomainName.c_str():NULL,
			dwLevel,
			FILTER_NORMAL_ACCOUNT,
			(LPBYTE*)&pBuf,
			dwPrefMaxLen,
			&dwEntriesRead,
			&dwTotalEntries,
			&dwResumeHandle);		
		if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
		{
			if ((pTmpBuf = pBuf) != NULL)
			{
				for (int i = 0; i < dwEntriesRead; i++)
				{
					if (pTmpBuf == NULL)
					{
						break;
					}
					
					USER_INFO uInfo = {0};
					wcscpy(uInfo.wszName,pTmpBuf->usri0_name);
					if (GetUserInfo(uInfo.dwUserID,uInfo.dwGroupID,uInfo.wszName))
					{
						vc.push_back(uInfo);
					}

					pTmpBuf++;					
				}
			}
		}
				
		if (pBuf != NULL)
		{
			NetApiBufferFree(pBuf);
			pBuf = NULL;
		}
	}while (nStatus == ERROR_MORE_DATA);

	return bRet;
}

BOOL CUserCtrl::GetUserInfo(DWORD &dwUid,DWORD &dwGid,const wchar_t * pName)
{
	BOOL bRet = FALSE;
    
	wchar_t wszUserName[200] = {0};
	DWORD dwUserNameLen = 200;

	LPUSER_INFO_3 lpUserInfo3 = NULL;

	if (NULL == pName)
	{
		GetUserNameW(wszUserName,&dwUserNameLen);
	}
	else
	{
		wcscpy(wszUserName,pName);
	}	

	NET_API_STATUS ntStatus = NetUserGetInfo(m_wstrDomainName.length() > 1 ?m_wstrDomainName.c_str():NULL,
		                       wszUserName,3,(LPBYTE*)&lpUserInfo3);
	if (NERR_Success == ntStatus)
	{
		dwUid = lpUserInfo3->usri3_user_id;
		dwGid = lpUserInfo3->usri3_primary_group_id;

		NetApiBufferFree(lpUserInfo3);
		lpUserInfo3 = NULL;

		bRet = TRUE;
	}

	return bRet;
}

BOOL CUserCtrl::ChangeUserPwd(const wchar_t * pName,const wchar_t * pWord,const wchar_t * pOldWrod)
{
	BOOL bRet = TRUE;

	NET_API_STATUS ntStatus = 0;

	if (NULL != pOldWrod)
	{
		if (!CanLogin(pName,pOldWrod))
		{
			return FALSE;
		}
	}

	DWORD dwLen;
	USER_INFO_1003 userInfo = {0};

	userInfo.usri1003_password = (wchar_t*)pWord;
	ntStatus = NetUserSetInfo(m_wstrDomainName.length() > 1 ?m_wstrDomainName.c_str():NULL,
		pName,1003,(LPBYTE)&userInfo,&dwLen);
	if (NERR_Success != ntStatus)
	{
		bRet = FALSE;
	}

	return bRet;
}

BOOL CUserCtrl::CanLogin(const wchar_t * pName,const wchar_t * pWord)
{
	BOOL bRet = TRUE;

	HANDLE hToken;
	if (!LogonUserW(pName,m_wstrDomainName.length() > 1 ?m_wstrDomainName.c_str():L".",
		pWord,LOGON32_LOGON_INTERACTIVE,LOGON32_PROVIDER_DEFAULT,&hToken))
	{
		bRet = FALSE;
	}
	else
	{
		CloseHandle(hToken);
		hToken = NULL;
	}

	return bRet;
}