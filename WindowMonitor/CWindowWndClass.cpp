#include "stdafx.h"
#include "CWindowWndClass.h"
#include "WindowHelper.h"

void CWindowWndClass::Register(WNDPROC wndProc)
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

void CWindowWndClass::Unregister()
{
	if (registered)
	{
		UnregisterClassW(className.c_str(), WindowHelper::GetCurrentModuleHandle());
	}
}
