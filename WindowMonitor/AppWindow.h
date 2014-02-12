#pragma once
#include "CWindow.h"
#include "AdjustableThumbnail.h"
#include "ViewSettingObserver.h"
#include "PresetWindow.h"

class WindowFilter;
class PresetManager;
class ViewSetting;

class AppWindow : public CWindow, public ViewSettingObserver
{
public:
	AppWindow(WindowFilter * const windowFilter, PresetManager * const presetManager, ViewSetting * const currentViewSetting, PresetWindow * const presetWindow);
	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void ViewSettingUpdated(ViewSettingObserverState const & state);

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
	void UpdateWindow();
	void UpdateThumbnail();
	void SelectSource(int const & index);
	void CycleForward();
	void CycleBack();
	void Reset();
	void ToggleBorder();
	void ToggleClickThrough();
	void SetContextualCursor();
	void UpdateSourceMenu();
	void UpdatePresetMenu();

	// Vars
	AdjustableThumbnail adjustableThumbnail;
	HWND sourceWindow;
	std::size_t sourceIndex;

	int chromeWidth, chromeHeight;

	HMENU menu, contextMenu, presetsMenu, zoomMenu;
	int baseMenuItemCount;
	bool suppressContextMenu;
	int currentCursor;
	bool cursorSet;
	POINTS lastPos;

	WindowFilter * windowFilter;
	PresetManager * presetManager;
	ViewSetting * currentViewSetting;
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