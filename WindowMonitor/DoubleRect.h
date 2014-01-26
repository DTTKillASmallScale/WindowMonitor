#pragma once
#include <windows.h>

class DoubleRect
{
public:
	double left;
	double top;
	double right;
	double bottom;
	DoubleRect();
	void SetFromClientRect(HWND const & hWnd);
};
