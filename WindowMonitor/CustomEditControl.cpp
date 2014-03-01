#include "stdafx.h"
#include "CustomEditControl.h"
#include "Application.h"
#include "PresetWindow.h"
#include "WindowHelper.h"

CustomEditControl::CustomEditControl(WindowBase * parent) :
	Control(parent),
	x(0), y(0), cx(100), cy(20)
{
}

void CustomEditControl::SetCoords(int const & _x, int const & _y, int const & _cx, int const & _cy)
{
	x = _x;
	y = _y;
	cx = _cx;
	cy = _cy;
}

void CustomEditControl::Create()
{
	// Check current window handle
	if (GetWindowHandle() != NULL) return;

	static const auto configureWindowStruct = [&](CREATESTRUCT & cs)
	{
		cs.lpszName = L"";
		cs.lpszClass = L"EDIT";
		cs.style = WS_CHILD | WS_VISIBLE | SS_LEFT;
		cs.dwExStyle = WS_EX_CLIENTEDGE;
		cs.x = x;
		cs.y = y;
		cs.cx = cx;
		cs.cy = cy;
	};

	Control::Create(configureWindowStruct);
}

LRESULT CustomEditControl::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CHAR:
		if (wParam == VK_RETURN) return 0;
	case WM_KEYUP:
		if (wParam == VK_RETURN)
		{
			HWND parent = GetParent(GetWindowHandle());
			SendMessage(parent, WM_COMMAND, MAKEWPARAM(PresetCommand::SavePreset, 0), reinterpret_cast<LPARAM>(GetWindowHandle()));
			return 0;
		}
	}

	return CallWindowProc(controlWndProc, hWnd, message, wParam, lParam);
}