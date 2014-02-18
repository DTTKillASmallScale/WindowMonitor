#include "stdafx.h"
#include "CWindow.h"
#include "WindowHelper.h"

CWindow::CWindow() :
	windowHandle(NULL),
	accelerators(NULL)
{
}

void CWindow::Create()
{
	// Check current window handle
	if (windowHandle != NULL) return;

	CREATESTRUCT cs;
	SecureZeroMemory(&cs, sizeof(CREATESTRUCT));
	cs.hwndParent = NULL;
	cs.hMenu = NULL;
	cs.lpszName = _T("New Window");
	cs.lpszClass = _T("CustomWindowClass");
	cs.x = 0;
	cs.y = 0;
	cs.cx = 640;
	cs.cy = 480;
	cs.style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	cs.dwExStyle = NULL;

	WNDCLASSEX wcex;
	SecureZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	// Let child classes override settings
	PreCreate(cs, wcex);

	// Make sure critical values are set
	cs.hInstance = WindowHelper::GetCurrentModuleHandle();
	cs.lpCreateParams = this;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hInstance = WindowHelper::GetCurrentModuleHandle();
	wcex.lpfnWndProc = CWindow::StaticWndProc;
	wcex.lpszClassName = cs.lpszClass;

	// Create window
	RegisterClassEx(&wcex);

	CreateWindowEx(
		cs.dwExStyle,
		cs.lpszClass,
		cs.lpszName,
		cs.style,
		cs.x, cs.y,
		cs.cx, cs.cy,
		cs.hwndParent,
		cs.hMenu,
		cs.hInstance,
		cs.lpCreateParams);
}

void CWindow::Destroy()
{
	if (windowHandle != NULL) DestroyWindow(windowHandle);
}

void CWindow::PreCreate(CREATESTRUCT & cs, WNDCLASSEX & wcex)
{
}

LRESULT CWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	
	return 0;
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
