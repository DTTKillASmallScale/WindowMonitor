#pragma once
#include "CWindow.h"
#include "AdjustableThumbnail.h"
#include "ViewSetting.h"

class WindowFilter;
class PresetManager;

class AppWindow : public CWindow
{
public:
	AppWindow(WindowFilter * const windowFilter, PresetManager * const presetManager);
	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

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
	void OnMenuCommand(WPARAM const & wParam, LPARAM const & lParam);
	void OnCreate();
	void OnDestroy();

	// Methods
	void UpdateWindow();
	void UpdateThumbnail();
	void SelectSource(int const & index);
	void CycleForward();
	void CycleBack();
	void Reset();
	void ToggleBorder();
	void ToggleClickThrough();
	void SetContextualCursor();
	void UpdateMenu();
	void GetMonitorRect(RECT & rect);

	// Vars
	AdjustableThumbnail adjustableThumbnail;
	HWND sourceWindow;
	std::size_t sourceIndex;

	ViewSetting currentViewSetting;
	int chromeWidth, chromeHeight;

	HMENU contextMenu;
	HMENU zoomMenu;
	int baseMenuItemCount;
	bool suppressContextMenu;
	int currentCursor;
	bool cursorSet;
	POINTS lastPos;

	WindowFilter * windowFilter;
	PresetManager * presetManager;

	// Constants
	static const int MaxMenuTextLength;
	static const int MenuItemBreakPoint;
	static const int CursorArrow;
	static const int CursorMove;
	static const int CursorPan;
	static const int CursorScale;
	static const int CursorNoFunction;
};