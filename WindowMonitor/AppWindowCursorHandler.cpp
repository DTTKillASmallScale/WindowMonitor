#include "stdafx.h"
#include "AppWindowCursorHandler.h"
#include "Resource.h"
#include "AppWindow.h"
#include "WindowMonitor.h"
#include "WindowHelper.h"

const int AppWindowCursorHandler::CursorArrow = 32512;
const int AppWindowCursorHandler::CursorMove = 32646;
const int AppWindowCursorHandler::CursorPan = 32649;
const int AppWindowCursorHandler::CursorScale = 32642;
const int AppWindowCursorHandler::CursorNoFunction = 32648;

AppWindowCursorHandler::AppWindowCursorHandler(AppWindow * const appWindow, WindowMonitor * const windowMonitor):
appWindow(appWindow),
windowMonitor(windowMonitor),
prevKeyState(keyStateNothing)
{
}

bool AppWindowCursorHandler::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT & result)
{
	switch (message)
	{
	case WM_KEYDOWN:
		// Bit 30: The value is 1 if the key is down before the message is sent, or it is zero if the key is up.
		if ((lParam & 0x40000000) == 1) break;
	case WM_KEYUP:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_SETCURSOR:
	{
		// Let DefWndProc handle non-client area cursor
		if (message == WM_SETCURSOR && LOWORD(lParam) != HTCLIENT)
		{
			prevKeyState = keyStateNothing;
			break;
		}

		// Get input state
		unsigned char newKeyState = (WindowHelper::IsKeyDown(VK_LBUTTON) ? keyStateLmb : 0)
			| (WindowHelper::IsKeyDown(VK_SHIFT) ? keyStateShift : 0)
			| (WindowHelper::IsKeyDown(VK_CONTROL) ? keyStateControl : 0);

		// Check state has changed
		if (newKeyState == prevKeyState) break;
		prevKeyState = newKeyState;

		// Set new cursor
		int newCursor;
		switch (newKeyState)
		{
		case keyStateLmb: newCursor = CursorMove; break;
		case keyStateShift: case keyStateShift | keyStateLmb: newCursor = CursorPan; break;
		case keyStateControl: case keyStateControl | keyStateLmb: newCursor = CursorScale; break;
		case keyStateShift | keyStateControl: case keyStateShift | keyStateControl | keyStateLmb: newCursor = CursorNoFunction; break;
		default: newCursor = CursorArrow; break;
		}
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(newCursor)));
	}
	}

	// If we handled client-area WM_SETCURSOR message prevent DefWndProc from also handling
	if (message == WM_SETCURSOR && LOWORD(lParam) == HTCLIENT)
	{
		result = TRUE;
		return true;
	}

	// Otherwise continue
	result = 0;
	return false;
}
