#pragma once
#include "CWindow.h"
#include "WindowMonitorObserver.h"
#include "AdjustableThumbnail.h"
#include "PresetWindow.h"

class WindowMonitor;

class AppWindow : public CWindow, public WindowMonitorObserver
{
public:
	AppWindow(WindowMonitor * const windowMonitor, PresetWindow * const presetWindow);
	void OnWindowMonitorEvent(WindowMonitorEvent const & event);

protected:
	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;

private:
	// Events
	bool OnSetCursor(WPARAM const & wParam, LPARAM const & lParam);
	bool OnMouseMove(WPARAM const & wParam, LPARAM const & lParam);
	bool OnSizing(WPARAM const & wParam, LPARAM const & lParam);
	bool OnAccelCommand(WPARAM const & wParam, LPARAM const & lParam);
	void OnContextMenu(WPARAM const & wParam, LPARAM const & lParam);
	void OnOptionsMenuCmd(WPARAM const & wParam);
	void OnPresetsMenuCmd(WPARAM const & wParam);
	void OnZoomMenuCmd(WPARAM const & wParam);
	void OnCreate();
	void OnDestroy();

	// Methods
	void ToggleBorder();
	void ToggleFullscreen();
	void ToggleClickThrough();
	void SetContextualCursor();
	void UpdateSourceMenu();
	void UpdatePresetMenu();
	void UpdateWindow();
	void CenterWindow();

	// Vars
	AdjustableThumbnail adjustableThumbnail;
	HMENU menu, contextMenu, presetsMenu, zoomMenu;
	int baseMenuItemCount;
	bool suppressContextMenu;
	int currentCursor;
	bool cursorSet;
	POINTS lastPos;
	int chromeWidth, chromeHeight;
	bool wasSizing;

	WindowMonitor * windowMonitor;
	PresetWindow * presetWindow;

public:
	// Constants
	static const int MaxMenuTextLength;
	static const int MenuItemBreakPoint;
	static const int CursorArrow;
	static const int CursorMove;
	static const int CursorPan;
	static const int CursorScale;
	static const int CursorNoFunction;
	static const COLORREF BackgroundColour;
};