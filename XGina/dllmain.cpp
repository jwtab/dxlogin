// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include "dllmain.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			DisableThreadLibraryCalls(hModule);
			break;
		}

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

/*	CGinaWndEx imp
*/
CGinaWndEx::CGinaWndEx()
{
	m_hWnd = NULL;
}

CGinaWndEx::~CGinaWndEx()
{

}

void CGinaWndEx::SetWnd(HWND hWnd,GINA_WND_TYPE type)
{
	m_hWnd = hWnd;
	m_wndType = type;

	LOGFONTW font;  
	memset(&font, 0, sizeof(font));  
	font.lfHeight  = 12;  
	font.lfWeight  = FW_NORMAL;  
	//font.lfCharSet = DEFAULT_CHARSET;//GB2312_CHARSET;  
	//font.lfQuality = DEFAULT_QUALITY;  
	wcscpy_s(font.lfFaceName,LF_FACESIZE,L"宋体");

	m_hFont = CreateFontIndirectW(&font);
}

void CGinaWndEx::DrawLogo(BOOL bBigPic)
{
	wchar_t wszBmpFilePath[MAX_PATH] = {0};

	SystemX::CXSysOS::GetWorkingDir(wszBmpFilePath);
	PathAppendW(wszBmpFilePath,BMP_FILE_NAME);
}

/*
	WM_COMMAND 
		LOWORD(wParam): 控件ID,　　HIWORD(wParam): 通知码 ,lParam: 子窗口句柄。
*/
BOOL CGinaWndEx::WndProcEx(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	if (WM_INITDIALOG == uMsg)
	{
		////m_hWatchThread = CreateThread(NULL,0,WorkThread,this,0,NULL);

		MakeWnd_SMS(wParam,lParam);
	}
	else if (WM_COMMAND == uMsg)
	{
		DWORD dwCtrlID = LOWORD(wParam);
		if (IDC_IDOK == dwCtrlID)
		{
			MessageBoxW(m_hWnd,L"ID ok.",L"Notice",MB_OK);
		}
		else if (IDC_GET_CODE_BTN == dwCtrlID)
		{
			MessageBoxW(m_hWnd,L"IDC_GET_CODE_BTN.",L"Notice",MB_OK);
		}
		else if (IDC_COMBOBOX_TYPE == dwCtrlID &&
			HIWORD(wParam) == BN_CLICKED)
		{
			LRESULT r = SendMessage(GetDlgItem(m_hWnd,IDC_COMBOBOX_TYPE),BM_GETCHECK,0,0); 
			if (BST_CHECKED == r)
			{
				MessageBoxW(m_hWnd,L"Checked",L"Notice",MB_OK);
			}
			else if (BST_UNCHECKED == r)
			{
				MessageBoxW(m_hWnd,L"UnChecked",L"Notice",MB_OK);
			}			
		}
	}

	return TRUE;
}

void CGinaWndEx::MakeWnd_SMS(WPARAM wParam,LPARAM lParam)
{
	DWORD dwUserPwdCtrlIDSta = 0;
	DWORD dwUserPwdCtrlIDEdit = 0;

	DWORD dwDomainCtrlIDSta = 0;
	DWORD dwDomainCtrlIDEdit = 0;

	DWORD dwBtnIDOkCtrlID = IDC_IDOK;
	DWORD dwBtnIDCancelCtrlID = IDC_IDCANCEL;
	DWORD dwBtnOptCtrlID = 0;
	DWORD dwBtnShutDownCtrID = 0;

	if (WND_TYPE_OUT_LOG == m_wndType)
	{
		dwUserPwdCtrlIDSta = IDC_WLXLOGGEDOUT_STATIC_PASSWORD;
		dwUserPwdCtrlIDEdit = IDC_WLXLOGGEDOUTSAS_PASSWORD;

		dwDomainCtrlIDSta = IDC_WLXLOGGEDOUTSAS_STATIC_DOMAIN;
		dwDomainCtrlIDEdit = IDC_WLXLOGGEDOUTSAS_DOMAIN;

		dwBtnOptCtrlID = IDC_WLXLOGGEDOUTSAS_OPT;
		dwBtnShutDownCtrID = IDC_WLXLOGGEDOUTSAS_SHUTDOWN;
	}
	else if (WND_TYPE_ON_LOG == m_wndType)
	{
		dwUserPwdCtrlIDSta = IDC_WLXLOGGEDONSAS_STATIC_PASSWORD;
		dwUserPwdCtrlIDEdit = IDC_WLXLOGGEDONSAS_PASSWORD;

		dwDomainCtrlIDSta = IDC_WLXLOGGEDONSAS_STATIC_DOMAIN;
		dwDomainCtrlIDEdit = IDC_WLXLOGGEDONSAS_DOMAIN;

		dwBtnOptCtrlID = IDC_WLXLOGGEDONSAS_OPT;
		dwBtnShutDownCtrID = IDC_WLXLOGGEDONSAS_SHUTDOWN;
	}

	/////////SetWindowText(dwUserPwdCtrlIDSta,L"验证码:");

	RECT rect;
	POINT point;

	//整个窗口变大.
	::GetWindowRect(m_hWnd,&rect);
	rect.bottom = rect.bottom + 60;	

	::SetWindowPos(m_hWnd,HWND_TOPMOST,rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,SWP_SHOWWINDOW);

	//move ID_OK
	HWND hWndSub = ::GetDlgItem(m_hWnd,dwBtnIDOkCtrlID);
	if (NULL != hWndSub)
	{
		::GetWindowRect(hWndSub,&rect);

		point.x = rect.left;
		point.y = rect.top;

		::ScreenToClient(m_hWnd,&point);
		point.y = point.y + 40;
		::MoveWindow(hWndSub,point.x,point.y,rect.right - rect.left,rect.bottom - rect.top,TRUE);
	}

	//move ID_CANCEL
	hWndSub = ::GetDlgItem(m_hWnd,dwBtnIDCancelCtrlID);
	if (NULL != hWndSub)
	{
		::GetWindowRect(hWndSub,&rect);

		point.x = rect.left;
		point.y = rect.top;

		::ScreenToClient(m_hWnd,&point);
		point.y = point.y + 40;
		::MoveWindow(hWndSub,point.x,point.y,rect.right - rect.left,rect.bottom - rect.top,TRUE);
	}

	//move opt
	hWndSub = ::GetDlgItem(m_hWnd,dwBtnOptCtrlID);
	if (NULL != hWndSub)
	{
		::GetWindowRect(hWndSub,&rect);

		point.x = rect.left;
		point.y = rect.top;

		::ScreenToClient(m_hWnd,&point);
		point.y = point.y + 40;
		::MoveWindow(hWndSub,point.x,point.y,rect.right - rect.left,rect.bottom - rect.top,TRUE);
	}

	//momve shutdown
	hWndSub = ::GetDlgItem(m_hWnd,dwBtnShutDownCtrID);
	if (NULL != hWndSub)
	{
		::GetWindowRect(hWndSub,&rect);

		point.x = rect.left;
		point.y = rect.top;

		::ScreenToClient(m_hWnd,&point);
		point.y = point.y + 40;
		::MoveWindow(hWndSub,point.x,point.y,rect.right - rect.left,rect.bottom - rect.top,TRUE);
	}
	
	//add btn
	/*
	if(GetWindowRect(GetDlgItem(m_hWnd,dwUserPwdCtrlIDEdit),&rect))
	{
		point.x = rect.right;
		point.y = rect.top;

		::ScreenToClient(m_hWnd,&point);

		HWND hBtn = CreateWindowW(L"BUTTON",L"获取",WS_CHILD|WS_VISIBLE,
			point.x + 5,point.y,60,20,m_hWnd,(HMENU)IDC_GET_CODE_BTN,((LPCREATESTRUCT)lParam)->hInstance,NULL);
		if(NULL != hBtn)
		{
			SendMessage(hBtn,WM_SETFONT,(WPARAM)m_hFont,TRUE); 
			ShowWindow(hBtn,SW_SHOW);
		}
	}
	*/

	//combobox static
	if(GetWindowRect(GetDlgItem(m_hWnd,dwUserPwdCtrlIDSta),&rect))
	{
		point.x = rect.left;
		point.y = rect.top;

		::ScreenToClient(m_hWnd,&point);

		HWND hStaType = CreateWindowW(L"STATIC",L"登录方式:",WS_CHILD|WS_VISIBLE,
			point.x,point.y + 30,rect.right - rect.left,20,m_hWnd,(HMENU)IDC_STATIC_TYPE,((LPCREATESTRUCT)lParam)->hInstance,NULL);
		if(NULL != hStaType)
		{
			SendMessage(hStaType,WM_SETFONT,(WPARAM)m_hFont,TRUE); 
			ShowWindow(hStaType,SW_SHOW);
		}
	}

	//combobox 
	if(GetWindowRect(GetDlgItem(m_hWnd,dwUserPwdCtrlIDEdit),&rect))
	{
		point.x = rect.left;
		point.y = rect.top;

		::ScreenToClient(m_hWnd,&point);

		/*
		HWND hComboBoxType = CreateWindowW(L"COMBOBOX",L"",WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST,
			point.x,point.y + 30,rect.right - rect.left,20,m_hWnd,(HMENU)IDC_COMBOBOX_TYPE,((LPCREATESTRUCT)lParam)->hInstance,NULL);
		if(NULL != hComboBoxType)
		{
			SendMessage(hComboBoxType,WM_SETFONT,(WPARAM)m_hFont,TRUE); 

			::SendMessage(hComboBoxType,CB_ADDSTRING,NULL,(LPARAM)L"Phone"); 
			::SendMessage(hComboBoxType,CB_ADDSTRING,NULL,(LPARAM)L"EMail");
			::SendMessage(hComboBoxType,CB_ADDSTRING,NULL,(LPARAM)L"XLogin");
			::SendMessage(hComboBoxType,CB_SETCURSEL,0,NULL);

			ShowWindow(hComboBoxType,SW_SHOW);
		}
		*/
		HWND hCheckBoxType = CreateWindowW(L"BUTTON",L"使用计算机终端",WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|BS_PUSHBUTTON,
			point.x,point.y + 30,rect.right - rect.left,20,m_hWnd,(HMENU)IDC_COMBOBOX_TYPE,((LPCREATESTRUCT)lParam)->hInstance,NULL);
		if(NULL != hCheckBoxType)
		{
			SendMessage(hCheckBoxType,WM_SETFONT,(WPARAM)m_hFont,TRUE);
			ShowWindow(hCheckBoxType,SW_SHOW);
		}
	}
}

void CGinaWndEx::SetWindowText(DWORD id,wchar_t * pwszText)
{
	HWND hSubWnd = GetDlgItem(m_hWnd,id);
	if (NULL != hSubWnd)
	{
		::SetWindowTextW(hSubWnd,pwszText);
	}
}

void CGinaWndEx::MakeOK(wchar_t * pwszPassword)
{
	DWORD dwUserPwdCtrlIDEdit = -1;	

	if (WND_TYPE_OUT_LOG == m_wndType)
	{
		dwUserPwdCtrlIDEdit = IDC_WLXLOGGEDOUTSAS_PASSWORD;
	}
	else if (WND_TYPE_ON_LOG == m_wndType)
	{		
		dwUserPwdCtrlIDEdit = IDC_WLXLOGGEDONSAS_PASSWORD;
	}

	SetWindowTextW(dwUserPwdCtrlIDEdit,pwszPassword);

	HWND hSubWnd = GetDlgItem(m_hWnd,IDC_IDOK);
	if (NULL != hSubWnd)
	{
		::PostMessage(hSubWnd,BM_CLICK,0,0);
	}
}

DWORD CGinaWndEx::WorkThread(LPVOID lpParam)
{
	DWORD dwRet = 0;

	CGinaWndEx * pGinaWnd = (CGinaWndEx*)lpParam;
	if (NULL != pGinaWnd)
	{
		//获取密码....
		wchar_t wszPassword[256] = {0};

		wcscpy(wszPassword,L"xxxxxx");

		pGinaWnd->MakeOK(wszPassword);
	}

	return dwRet;
}
