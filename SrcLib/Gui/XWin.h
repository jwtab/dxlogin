
#ifndef X_WIN_H_
#define X_WIN_H_

#include <windows.h>

class CXWin
{
public:
	CXWin();
	CXWin(HINSTANCE hInstance);

public:
	void SetInstance(HINSTANCE hInstance);
		
public:
	HINSTANCE m_hInstance;
};

#endif