
#ifndef CMD_EVENT_WND_H_
#define CMD_EVENT_WND_H_

#include <Windows.h>
#include "CredentialV2_Imp.h"

class CXCreProviderV2;

class CCmdEventWnd
{
public:
	CCmdEventWnd();
    ~CCmdEventWnd();

    HRESULT Initialize(CXCreProviderV2 *pProvider);
    BOOL GetConnectedStatus();

private:
    HRESULT _MyRegisterClass();
    HRESULT _InitInstance();
    BOOL _ProcessNextMessage();
    
    static DWORD WINAPI _ThreadProc(__in LPVOID lpParameter);
    static LRESULT CALLBACK    _WndProc(__in HWND hWnd, __in UINT message, __in WPARAM wParam, __in LPARAM lParam);
    
    CXCreProviderV2  * m_pProvider;        // Pointer to our owner.
    HWND  m_hWnd;             // Handle to our window.
    HWND  m_hWndButton;       // Handle to our window's button.
    HINSTANCE  m_hInst;            // Current instance
    BOOL m_fConnected;       // Whether or not we're connected.
};

#endif