#pragma once
#include "CWindow.h"
class CustomEditControl :
	public CWindow
{
public:
	CustomEditControl();
	void CreateControl(int const & x, int const & y, int const & cx, int const & cy, HWND const & parent);
	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	WNDPROC originalProc;
};

