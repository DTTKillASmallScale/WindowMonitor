#pragma once
#include "WindowMonitorObserver.h"
class WindowMonitor;
class AppWindow;

class AppWindowMenuHandler : public WindowMonitorObserver
{
public:
	AppWindowMenuHandler(AppWindow * const appWindow, WindowMonitor * const windowMonitor);
	~AppWindowMenuHandler();
	bool WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT & result);
	void OnWindowMonitorEvent(WindowMonitorEvent const & event);

private:
	AppWindow * appWindow;
	WindowMonitor * windowMonitor;

	HMENU entireMenu, contextMenu, presetMenu, zoomMenu;
	int baseMenuItemCount;

	void OnContextMenu(WPARAM const & wParam, LPARAM const & lParam);
	void HandleMenuCmd(WPARAM const & wParam, LPARAM const & lParam);
	
	void RefreshSourceMenu(); 
	void ReloadSourceMenu();
	void RefreshPresetMenu();
	void ReloadPresetMenu();

	static const int MaxMenuTextLength;
	static const int MenuItemBreakPoint;
};

