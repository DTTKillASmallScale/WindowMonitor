#include "stdafx.h"
#include "AppWindowAcceleratorHandler.h"
#include "Resource.h"
#include "AppWindow.h"
#include "WindowMonitor.h"

AppWindowAcceleratorHandler::AppWindowAcceleratorHandler(AppWindow * const appWindow, WindowMonitor * const windowMonitor) : 
appWindow(appWindow), 
windowMonitor(windowMonitor)
{
}

bool AppWindowAcceleratorHandler::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT & result)
{
	result = 0;

	switch (message)
	{
	case WM_COMMAND:
	{
		if (HIWORD(wParam) == 1)
		{
			switch (LOWORD(wParam))
			{
			case ID_ACCEL_SWITCH:
				windowMonitor->SelectNextSource();
				return true;
			case ID_ACCEL_RSWITCH:
				windowMonitor->SelectPreviousSource();
				return true;
			case ID_ACCEL_RESET:
				windowMonitor->ResetAndScaleToFitMonitor(appWindow->GetWindowHandle());
				return true;
			case ID_ACCEL_FULLSCREEN:
				appWindow->ToggleFullscreen();
				return true;
			case ID_ACCEL_CLICKTHROUGH:
				appWindow->ToggleClickThrough();
				return true;
			}
		}
	}
	}

	return false;
}

