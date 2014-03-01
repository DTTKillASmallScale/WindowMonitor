#include "stdafx.h"
#include "Window.h"
#include "Application.h"
#include "WindowHelper.h"

Window::Window() :
	customClassRegistered(false)
{
}

Window::~Window()
{
	if (customClassRegistered)
		UnregisterClassW(customClassname.c_str(), WindowHelper::GetCurrentModuleHandle());
}

void Window::Create(
	std::function<void(WNDCLASSEX &)> const & configureWindowClass, 
	std::function<void(CREATESTRUCT &)> const & configureWindowStruct)
{
	if (customClassRegistered == false && configureWindowClass != nullptr)
	{
		WNDCLASSEX wcex;
		SecureZeroMemory(&wcex, sizeof(WNDCLASSEX));
		configureWindowClass(wcex);
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.hInstance = WindowHelper::GetCurrentModuleHandle();
		wcex.lpfnWndProc = Application::StaticWndProc;
		customClassname.assign(wcex.lpszClassName);
		customClassRegistered = (RegisterClassExW(&wcex) != 0);
	}

	if (configureWindowStruct != nullptr)
	{
		CREATESTRUCT cs;
		SecureZeroMemory(&cs, sizeof(CREATESTRUCT));
		configureWindowStruct(cs);
		cs.hInstance = WindowHelper::GetCurrentModuleHandle();
		cs.lpCreateParams = this;
		::CreateWindowEx(cs.dwExStyle, cs.lpszClass, cs.lpszName, cs.style, cs.x, cs.y, cs.cx, cs.cy, cs.hwndParent, cs.hMenu, cs.hInstance, cs.lpCreateParams);
	}
}
