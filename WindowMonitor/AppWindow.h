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
	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void OnWindowMonitorEvent(WindowMonitorEvent const & event);

protected:
	virtual void PreCreate(CREATESTRUCT & cs, WNDCLASSEX & wcex);

private:
	// Events
	bool OnKeyDown(WPARAM const & wParam, LPARAM const & lParam);
	bool OnKeyUp(WPARAM const & wParam, LPARAM const & lParam);
	bool OnLeftButtonUp(WPARAM const & wParam, LPARAM const & lParam);
	bool OnRightButtonUp(WPARAM const & wParam, LPARAM const & lParam);
	bool OnSetCursor(WPARAM const & wParam, LPARAM const & lParam);
	bool OnMouseMove(WPARAM const & wParam, LPARAM const & lParam);
	bool OnSizing(WPARAM const & wParam, LPARAM const & lParam);
	bool OnSize(WPARAM const & wParam, LPARAM const & lParam);
	bool OnLeftDoubleClick(WPARAM const & wParam, LPARAM const & lParam);
	bool OnAccelCommand(WPARAM const & wParam, LPARAM const & lParam);
	void OnContextMenu(WPARAM const & wParam, LPARAM const & lParam);
	void OnOptionsMenuCmd(WPARAM const & wParam);
	void OnPresetsMenuCmd(WPARAM const & wParam);
	void OnZoomMenuCmd(WPARAM const & wParam);
	void OnCreate();
	void OnDestroy();

	// Methods
	void ToggleBorder();
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

	WindowMonitor * windowMonitor;
	PresetWindow * presetWindow;

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