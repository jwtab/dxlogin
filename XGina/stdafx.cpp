// stdafx.cpp : source file that includes just the standard includes
// XGina.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

#include "dllmain.h"

GINA_WND_EX g_wndEx[WND_TYPE_MAX] = 
		{{NULL,FALSE}, //TYPE_OUT_SAS
		 {NULL,FALSE}, //TYPE_OUT_LOG
		 {NULL,FALSE}, //TYPE_ON_SAS
		 {NULL,FALSE}, //TYPE_ON_LOG
		};