#include "stdafx.h"
#include "Application.h"
#include "WindowBase.h"

namespace Application
{
	bool PreTranslateMessage(MSG msg)
	{
		bool processed = false;
	
		if ((msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) ||
			(msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST))
		{
			WindowBase * window = reinterpret_cast<WindowBase*>(GetWindowLongPtr(msg.hwnd, GWLP_USERDATA));
			if (window != NULL && TranslateAccelerator(msg.hwnd, window->GetAccelerators(), &msg))
				processed = true;
		}
	
		return processed;
	}

	int Run()
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
	
		return static_cast<int>(msg.wParam);
	}

	LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		// Get pointer to object from lpCreateParams and set userdata
		// This is so we don't lose any messages that are sent before CreateWindowEx exits
		if (message == WM_NCCREATE)
		{
			CREATESTRUCT * cs = reinterpret_cast<CREATESTRUCT*>(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
		}
	
		// Get window pointer from userdata
		WindowBase * window = reinterpret_cast<WindowBase*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	
		// Call wndproc
		if (window != NULL) return window->ApplicationWndProc(hWnd, message, wParam, lParam); 
		else return DefWindowProc(hWnd, message, wParam, lParam);
	}
}
