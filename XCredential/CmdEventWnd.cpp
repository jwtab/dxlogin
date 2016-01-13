
#include "stdafx.h"
#include <strsafe.h>

#include "CmdEventWnd.h"

// Custom messages for managing the behavior of the window thread.
#define WM_EXIT_THREAD              WM_USER + 1
#define WM_TOGGLE_CONNECTED_STATUS  WM_USER + 2

const WCHAR c_szClassName[] = L"EventWindow";
const WCHAR c_szConnected[] = L"Connected";
const WCHAR c_szDisconnected[] = L"Disconnected";

CCmdEventWnd::CCmdEventWnd() : m_hWnd(NULL), m_hInst(NULL), m_fConnected(FALSE), m_pProvider(NULL)
{
}

CCmdEventWnd::~CCmdEventWnd()
{
    // If we have an active window, we want to post it an exit message.
    if (m_hWnd != NULL)
    {
        PostMessage(m_hWnd, WM_EXIT_THREAD, 0, 0);
        m_hWnd = NULL;
    }

    // We'll also make sure to release any reference we have to the provider.
    if (m_pProvider != NULL)
    {
        m_pProvider->Release();
        m_pProvider = NULL;
    }
}

// Performs the work required to spin off our message so we can listen for events.
HRESULT CCmdEventWnd::Initialize(CXCreProvider *pProvider)
{
    HRESULT hr = S_OK;

    // Be sure to add a release any existing provider we might have, then add a reference
    // to the provider we're working with now.
    if (m_pProvider != NULL)
    {
        m_pProvider->Release();
    }

    m_pProvider = pProvider;
    m_pProvider->AddRef();
    
    // Create and launch the window thread.
    HANDLE hThread = CreateThread(NULL, 0, _ThreadProc, this, 0, NULL);
    if (hThread == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

// Wraps our internal connected status so callers can easily access it.
BOOL CCmdEventWnd::GetConnectedStatus()
{
    return m_fConnected;
}

//
//  FUNCTION: _MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
HRESULT CCmdEventWnd::_MyRegisterClass()
{
    WNDCLASSEX wcex = { sizeof(wcex) };
    wcex.style            = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc      = _WndProc;
    wcex.hInstance        = m_hInst;
    wcex.hIcon            = NULL;
    wcex.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName    = c_szClassName;

    return RegisterClassEx(&wcex) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

//
//   FUNCTION: _InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HRESULT CCmdEventWnd::_InitInstance()
{
    HRESULT hr = S_OK;

    // Create our window to receive events.
    // 
    // This dialog is for demonstration purposes only.  It is not recommended to create 
    // dialogs that are visible even before a credential enumerated by this credential 
    // provider is selected.  Additionally, any dialogs that are created by a credential
    // provider should not have a NULL hwndParent, but should be parented to the HWND
    // returned by ICredentialProviderCredentialEvents::OnCreatingWindow.
    m_hWnd = CreateWindowEx(
        WS_EX_TOPMOST, 
        c_szClassName, 
        c_szDisconnected, 
        WS_DLGFRAME,
        200, 200, 200, 80, 
        NULL,
        NULL, m_hInst, NULL);
    if (m_hWnd == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    if (SUCCEEDED(hr))
    {
        // Add a button to the window.
        m_hWndButton = CreateWindow(L"Button", L"Press to connect", 
                             WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
                             10, 10, 180, 30, 
                             m_hWnd, 
                             NULL,
                             m_hInst,
                             NULL);
        if (m_hWndButton == NULL)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

        if (SUCCEEDED(hr))
        {
            // Show and update the window.
            if (!ShowWindow(m_hWnd, SW_NORMAL))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }

            if (SUCCEEDED(hr))
            {
                if (!UpdateWindow(m_hWnd))
                {
                   hr = HRESULT_FROM_WIN32(GetLastError());
                }
            }
        }
    }

    return hr;
}

// Called from the separate thread to process the next message in the message queue. If
// there are no messages, it'll wait for one.
BOOL CCmdEventWnd::_ProcessNextMessage()
{
    // Grab, translate, and process the message.
    MSG msg;
    GetMessage(&(msg), m_hWnd, 0, 0);
    TranslateMessage(&(msg));
    DispatchMessage(&(msg));

    // This section performs some "post-processing" of the message. It's easier to do these
    // things here because we have the handles to the window, its button, and the provider
    // handy.
    switch (msg.message)
    {
    // Return to the thread loop and let it know to exit.
    case WM_EXIT_THREAD: return FALSE;

    // Toggle the connection status, which also involves updating the UI.
    case WM_TOGGLE_CONNECTED_STATUS:
        m_fConnected = !m_fConnected;
        if (m_fConnected)
        {
            SetWindowText(m_hWnd, c_szConnected);
            SetWindowText(m_hWndButton, L"Press to disconnect");
        }
        else
        {
            SetWindowText(m_hWnd, c_szDisconnected);
            SetWindowText(m_hWndButton, L"Press to connect");
        }

        m_pProvider->OnConnectStatusChanged();
        break;
    }

    return TRUE;
}

// Manages window messages on the window thread.
LRESULT CALLBACK CCmdEventWnd::_WndProc(__in HWND hWnd, __in UINT message, __in WPARAM wParam, __in LPARAM lParam)
{
    switch (message)
    {
    // Originally we were going to work with USB keys being inserted and removed, but it
    // seems as though these events don't get to us on the secure desktop. However, you
    // might see some messageboxi in CredUI.
    // TODO: Remove if we can't use from LogonUI.
    case WM_DEVICECHANGE:
        MessageBox(NULL, L"Device change", L"Device change", 0);
        break;

    // We assume this was the button being clicked.
    case WM_COMMAND:
        PostMessage(hWnd, WM_TOGGLE_CONNECTED_STATUS, 0, 0);
        break;

    // To play it safe, we hide the window when "closed" and post a message telling the 
    // thread to exit.
    case WM_CLOSE:
        ShowWindow(hWnd, SW_HIDE);
        PostMessage(hWnd, WM_EXIT_THREAD, 0, 0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Our thread procedure. We actually do a lot of work here that could be put back on the 
// main thread, such as setting up the window, etc.
DWORD WINAPI CCmdEventWnd::_ThreadProc(__in LPVOID lpParameter)
{
    CCmdEventWnd *pCommandWindow = static_cast<CCmdEventWnd*>(lpParameter);
    if (pCommandWindow == NULL)
    {
        // TODO: What's the best way to raise this error?
        return 0;
    }

    HRESULT hr = S_OK;

    // Create the window.
    pCommandWindow->m_hInst = GetModuleHandle(NULL);
    if (pCommandWindow->m_hInst != NULL)
    {            
        hr = pCommandWindow->_MyRegisterClass();
        if (SUCCEEDED(hr))
        {
            hr = pCommandWindow->_InitInstance();
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    // ProcessNextMessage will pump our message pump and return false if it comes across
    // a message telling us to exit the thread.
    if (SUCCEEDED(hr))
    {        
        while (pCommandWindow->_ProcessNextMessage()) 
        {
        }
    }
    else
    {
        if (pCommandWindow->m_hWnd != NULL)
        {
            pCommandWindow->m_hWnd = NULL;
        }
    }

    return 0;
}
