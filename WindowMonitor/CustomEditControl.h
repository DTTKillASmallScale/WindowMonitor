#pragma once
#include "CWindow.h"
class CustomEditControl :
	public CWindow
{
public:
	CustomEditControl(int const & x, int const & y, int const & cx, int const & cy);
	void SetParent(HWND const & parent);
	virtual void Create() override;

protected:
	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;

private:
	CREATESTRUCT cs;
	WNDPROC originalProc;
};

