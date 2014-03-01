#pragma once
#include "Control.h"

class CustomEditControl : public Control
{
public:
	CustomEditControl(WindowBase * parent);
	void SetCoords(int const & x, int const & y, int const & cx, int const & cy);
	virtual void Create() override;

protected:
	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;

private:
	int cx, cy, x, y;
};

