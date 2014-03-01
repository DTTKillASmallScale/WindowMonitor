#include "stdafx.h"
#include "Control.h"
#include "Application.h"
#include "WindowHelper.h"

Control::Control(WindowBase * parent) :
	parent(parent)
{
}

void Control::Create(std::function<void(CREATESTRUCT &)> const & configureWindowStruct)
{
	// Check
	if (configureWindowStruct == nullptr) return; // TODO: throw error

	// Create window
	CREATESTRUCT cs;
	SecureZeroMemory(&cs, sizeof(CREATESTRUCT));
	configureWindowStruct(cs);
	cs.hwndParent = parent->GetWindowHandle();
	HWND hwnd = CreateWindowEx(cs.dwExStyle, cs.lpszClass, cs.lpszName, cs.style, cs.x, cs.y, cs.cx, cs.cy, cs.hwndParent, cs.hMenu, WindowHelper::GetCurrentModuleHandle(), cs.lpCreateParams);
	SetWindowHandle(hwnd);

	// Set user data for StaticWndProc
	SetWindowLongPtr(GetWindowHandle(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	// Re-wire to use StaticWndProc
	controlWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(GetWindowHandle(), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Application::StaticWndProc)));
}
