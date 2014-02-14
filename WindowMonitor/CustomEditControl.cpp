#include "stdafx.h"
#include "CustomEditControl.h"
#include "PresetWindow.h"

CustomEditControl::CustomEditControl() :
	originalProc(NULL)
{
}

void CustomEditControl::CreateControl(int const & x, int const & y, int const & cx, int const & cy, HWND const & parent)
{
	// Check current window handle
	if (windowHandle != NULL) return;

	instance = GetModuleHandle(NULL);

	CREATESTRUCT cs;
	SecureZeroMemory(&cs, sizeof(CREATESTRUCT));
	cs.hwndParent = parent;
	cs.hMenu = NULL;
	cs.lpszName = _T("");
	cs.lpszClass = _T("EDIT");
	cs.x = x;
	cs.y = y;
	cs.cx = cx;
	cs.cy = cy;
	cs.style = WS_CHILD | WS_VISIBLE | SS_LEFT;
	cs.dwExStyle = WS_EX_CLIENTEDGE;
	cs.lpCreateParams = this;
	cs.hInstance = instance;

	// Create window
	windowHandle = CreateWindowEx(cs.dwExStyle, cs.lpszClass, cs.lpszName, cs.style, cs.x, cs.y, cs.cx, cs.cy, cs.hwndParent, cs.hMenu, cs.hInstance, cs.lpCreateParams);

	// Set user data for StaticWndProc
	SetWindowLongPtr(windowHandle, GWLP_USERDATA, (LONG_PTR)this);

	// Re-wire to use StaticWndProc
	originalProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(windowHandle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&CWindow::StaticWndProc)));
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
			HWND parent = GetParent(windowHandle);
			SendMessage(parent, WM_COMMAND, MAKEWPARAM(PresetCommand::SavePreset, 0), reinterpret_cast<LPARAM>(windowHandle));
			return 0;
		}
	}

	return CallWindowProc(originalProc, hWnd, message, wParam, lParam);
}