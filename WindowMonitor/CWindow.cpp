#include "stdafx.h"
#include "CWindow.h"

CWindow::CWindow() :
	instance(NULL),
	windowHandle(NULL),
	accelerators(NULL),
	keepRunning(true)
{
}

void CWindow::Create()
{
	instance = GetModuleHandle(NULL);
	
	CREATESTRUCT cs;
	SecureZeroMemory(&cs, sizeof(CREATESTRUCT));

	cs.hInstance = instance;
	cs.hwndParent = NULL;
	cs.hMenu = NULL;
	cs.lpszName = _T("New Window");
	cs.lpszClass = _T("CustomWindowClass");
	cs.cx = 640;
	cs.cy = 480;
	cs.style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	cs.dwExStyle = NULL;
	cs.lpCreateParams = this;

	PreCreate(cs);

	// Center window
	RECT rc;
	GetClientRect(GetDesktopWindow(), &rc);
	cs.x = (rc.right - rc.left - cs.cx) / 2;
	cs.y = (rc.bottom - rc.top - cs.cy) / 2;

	RegisterWindowClass(cs);
	MakeWindow(cs);
	OnInitialUpdate();
}

void CWindow::Run()
{
	MSG msg;
	BOOL ret;
	
	while (keepRunning && (ret = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (ret == -1)
		{
			keepRunning = false;
			break;
		}

		if (!PreTranslateMessage(msg))
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
		if (TranslateAccelerator(windowHandle, accelerators, &msg)) 
			processed = true;
	}
	
	return processed;
}

LRESULT CWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_QUIT:
	case WM_CLOSE:
		keepRunning = false;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	
	return 0;
}

ATOM CWindow::RegisterWindowClass(CREATESTRUCT const & cs)
{
	WNDCLASSEX wcex;
	SecureZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = CWindow::StaticWndProc;
	wcex.hInstance = instance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszClassName = cs.lpszClass;
	return RegisterClassEx(&wcex);
}

bool CWindow::MakeWindow(CREATESTRUCT const & cs)
{
	// Create window
	windowHandle = CreateWindowEx(
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

	if (!windowHandle) return false;

	// Show window
	ShowWindow(windowHandle, SW_SHOWNORMAL);
	UpdateWindow(windowHandle);

	return true;
}

LRESULT CALLBACK CWindow::StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CWindow * window = reinterpret_cast<CWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	if (window == NULL && message == WM_NCCREATE) 
	{
		// Get pointer to object from lpCreateParams and set userdata
		// This is so we don't lose any messages that are sent before CreateWindowEx exits
		CREATESTRUCT * cs = reinterpret_cast<CREATESTRUCT*>(lParam);
		window = reinterpret_cast<CWindow*>(cs->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)window);
	}

	if (window == NULL) return DefWindowProc(hWnd, message, wParam, lParam);
	else return window->WndProc(hWnd, message, wParam, lParam);
}