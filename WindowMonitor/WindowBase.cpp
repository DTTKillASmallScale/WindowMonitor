#include "stdafx.h"
#include "WindowBase.h"
#include "WindowHelper.h"

WindowBase::WindowBase() :
	windowHandle(NULL),
	accelerators(NULL)
{
}

void WindowBase::Create()
{
}

void WindowBase::Destroy()
{
	if (windowHandle != NULL) DestroyWindow(windowHandle);
}

HWND const & WindowBase::GetWindowHandle()
{
	return windowHandle;
}

void WindowBase::SetWindowHandle(HWND const & newWindowHandle)
{
	windowHandle = newWindowHandle;
}

HACCEL const & WindowBase::GetAccelerators()
{
	return accelerators;
}

void WindowBase::SetAccelerators(int const & resourceId)
{
	accelerators = LoadAccelerators(WindowHelper::GetCurrentModuleHandle(), MAKEINTRESOURCE(resourceId));
}

LRESULT WindowBase::ApplicationWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Store handle
	if (message == WM_CREATE) windowHandle = hWnd;

	// Run window process
	LRESULT result = WndProc(hWnd, message, wParam, lParam);

	// Remove handle if destroyed
	if (message == WM_DESTROY) windowHandle = NULL;

	// Return result
	return result;
}