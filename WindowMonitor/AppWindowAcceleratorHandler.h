#pragma once
class WindowMonitor;
class AppWindow;

class AppWindowAcceleratorHandler
{
public:
	AppWindowAcceleratorHandler(AppWindow * const appWindow, WindowMonitor * const windowMonitor);
	bool WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT & result);

private:
	AppWindow * appWindow;
	WindowMonitor * windowMonitor;
};
