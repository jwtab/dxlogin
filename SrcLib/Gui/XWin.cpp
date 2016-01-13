
#include "stdafx.h"

#include "XWin.h"

CXWin::CXWin()
{
	m_hInstance = NULL;
}

CXWin::CXWin(HINSTANCE hInstance)
{
	m_hInstance = hInstance;
}

void CXWin::SetInstance(HINSTANCE hInstance)
{
	m_hInstance = hInstance;
}