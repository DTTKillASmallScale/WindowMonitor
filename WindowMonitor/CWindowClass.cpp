#include "stdafx.h"
#include "CWindowClass.h"
#include "WindowHelper.h"

void CWindowClass::Register(WNDPROC wndProc)
{
	if (registered) return;
	WNDCLASSEX wcex;
	SecureZeroMemory(&wcex, sizeof(WNDCLASSEX));
	Configure(wcex);
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hInstance = WindowHelper::GetCurrentModuleHandle();
	wcex.lpfnWndProc = wndProc;
	wcex.lpszClassName = className.c_str();
	if (RegisterClassExW(&wcex) != 0) registered = true;
}

void CWindowClass::Unregister()
{
	if (registered)
	{
		UnregisterClassW(className.c_str(), WindowHelper::GetCurrentModuleHandle());
	}
}

void CWindowStruct::Create(CWindow & window)
{
	CREATESTRUCT cs;
	SecureZeroMemory(&cs, sizeof(CREATESTRUCT));
	Configure(cs);
	cs.hInstance = WindowHelper::GetCurrentModuleHandle();
	cs.lpCreateParams = &window;
	::CreateWindowEx(cs.dwExStyle, cs.lpszClass, cs.lpszName, cs.style, cs.x, cs.y, cs.cx, cs.cy, cs.hwndParent, cs.hMenu, cs.hInstance, cs.lpCreateParams);
}