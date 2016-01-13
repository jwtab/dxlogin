
#ifndef USER_CTRL_H_
#define USER_CTRL_H_

#include <Windows.h>

#include <LM.h>
#pragma comment(lib,"Netapi32.lib")

#include <vector>
#include <string>

using namespace std;

typedef struct User_Info
{
	wchar_t wszName[100];
	DWORD   dwUserID;
	DWORD   dwGroupID;
}USER_INFO,*LPUSER_INFO;

typedef vector<USER_INFO> USER_INFO_VECTOR;

class CUserCtrl
{
public:
	CUserCtrl(const wchar_t * pDomainName = NULL);
	~CUserCtrl();

public:
	BOOL GetAllUsersInfo(USER_INFO_VECTOR &vc);
	BOOL GetUserInfo(DWORD &dwUid,DWORD &dwGid,const wchar_t * pName = NULL);

	BOOL ChangeUserPwd(const wchar_t * pName,const wchar_t * pWord,const wchar_t * pOldWrod = NULL);
	BOOL CanLogin(const wchar_t * pName,const wchar_t * pWord);

private:
	wstring m_wstrDomainName;
};

#endif