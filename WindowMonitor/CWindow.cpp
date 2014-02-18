#include "stdafx.h"
#include "CWindow.h"
#include "CWindowClass.h"
#include "WindowHelper.h"

CWindow::CWindow() :
	windowHandle(NULL),
	accelerators(NULL)
{
}

CWindow::~CWindow()
{
	if (windowClass.get() != nullptr) windowClass->Unregister();
}

void CWindow::Create()
{
	if (windowHandle != NULL) return;
	if (windowClass.get() != nullptr) windowClass->Register(StaticWndProc);
	if (windowStruct.get() != nullptr) windowStruct->Create(*this);
}

void CWindow::Destroy()
{
	if (windowHandle != NULL) DestroyWindow(windowHandle);
}

void CWindow::SetAccelerators(int const & resourceId)
{
	accelerators = LoadAccelerators(WindowHelper::GetCurrentModuleHandle(), MAKEINTRESOURCE(resourceId));
}

void CWindow::Run()
{
	MSG msg;
	BOOL ret;

	while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (ret == -1)
		{
			// Error
		}
		else if (!PreTranslateMessage(msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

bool CWindow::PreTranslateMessage(MSG msg)
{
	bool processed = false;

	if ((msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) ||
		(msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST))
	{
		CWindow * window = reinterpret_cast<CWindow*>(GetWindowLongPtr(msg.hwnd, GWLP_USERDATA));
		if (window != NULL && TranslateAccelerator(msg.hwnd, window->accelerators, &msg))
			processed = true;
	}

	return processed;
}

LRESULT CALLBACK CWindow::StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Get pointer to object from lpCreateParams and set userdata
	// This is so we don't lose any messages that are sent before CreateWindowEx exits
	if (message == WM_NCCREATE)
	{
		CREATESTRUCT * cs = reinterpret_cast<CREATESTRUCT*>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
	}

	CWindow * window = reinterpret_cast<CWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	if (window == NULL)
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	else 
	{
		// Store handle
		if (message == WM_CREATE) window->windowHandle = hWnd;
		
		// Run window process
		LRESULT result = window->WndProc(hWnd, message, wParam, lParam);

		// Remove handle if destroyed
		if (message == WM_DESTROY) window->windowHandle = NULL;

		// Return
		return result;
	}
}
