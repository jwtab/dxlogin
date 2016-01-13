// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include "dllmain.h"

#include "../SrcLib/DataHandler/Log/Log.h"
#include "../SrcLib/System/SysOS/SysOS.h"

extern GINA_WND_EX g_wndEx[WND_TYPE_MAX];

#ifdef _DEBUG
#pragma comment(lib,"SrcLib_D.lib")
#else
#pragma comment(lib,"SrcLib.lib")
#endif

#pragma comment(lib,"shlwapi.lib")

// TODO: reference additional headers your program requires here
