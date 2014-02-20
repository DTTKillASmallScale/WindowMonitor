#pragma once
class WindowMonitor;
class AppWindow;

class AppWindowCursorHandler
{
public:
	AppWindowCursorHandler(AppWindow * const appWindow, WindowMonitor * const windowMonitor);
	bool WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT & result);

private:
	AppWindow * appWindow;
	WindowMonitor * windowMonitor;

	unsigned char prevKeyState;

	static const int CursorArrow;
	static const int CursorMove;
	static const int CursorPan;
	static const int CursorScale;
	static const int CursorNoFunction;

	enum CursorKeyState
	{
		keyStateLmb = 1 << 0,
		keyStateShift = 1 << 1,
		keyStateControl = 1 << 2,
		keyStateNothing = 1 << 3
	};
};

