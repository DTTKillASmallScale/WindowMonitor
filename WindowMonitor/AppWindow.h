#pragma once
#include "CWindow.h"
#include "AppWindowAcceleratorHandler.h"
#include "AppWindowCursorHandler.h"
#include "AppWindowMenuHandler.h"
#include "WindowMonitorObserver.h"
#include "AdjustableThumbnail.h"
#include "PresetWindow.h"

class WindowMonitor;

class AppWindow : public CWindow, public WindowMonitorObserver
{
	friend class AppWindowMenuHandler;
public:
	AppWindow(WindowMonitor * const windowMonitor, PresetWindow * const presetWindow);
	void ToggleBorder();
	void ToggleFullscreen();
	void ToggleClickThrough();
	void ShowPresetWindow();
	void OnWindowMonitorEvent(WindowMonitorEvent const & event);
	static const COLORREF BackgroundColour;

protected:
	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;

private:
	// Events
	AppWindowCursorHandler cursorHandler;
	AppWindowAcceleratorHandler acceleratorhandler;
	AppWindowMenuHandler menuHandler;
	bool OnMouseMove(WPARAM const & wParam, LPARAM const & lParam);
	bool OnSizing(WPARAM const & wParam, LPARAM const & lParam);
	void OnCreate();
	void OnDestroy();

	// Methods
	void UpdateWindow();
	void CenterWindow();

	// Vars
	AdjustableThumbnail adjustableThumbnail;
	POINTS lastPos;
	int chromeWidth, chromeHeight;
	bool wasSizing, borderVisible, fullScreen;
	WindowMonitor * windowMonitor;
	PresetWindow * presetWindow;
};