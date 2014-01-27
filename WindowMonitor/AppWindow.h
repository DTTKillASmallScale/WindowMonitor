#pragma once
#include "CWindow.h"
#include "DoubleRect.h"
#include "AdjustableThumbnail.h"
#include "WindowFilter.h"

class AppWindow : public CWindow
{
public:
	AppWindow();
	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	virtual void PreCreate(CREATESTRUCT & cs, WNDCLASSEX & wcex);
	virtual void OnInitialUpdate();

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
	void OnDestroy();

	// Methods
	void UpdateWindow(bool const & center);
	void ScaleThumbnail();
	void SelectSource(int const & index);
	void CycleForward();
	void CycleBack();
	void Reset();
	void ToggleBorder();
	void SetContextualCursor();
	void UpdateMenu();

	// Vars
	AdjustableThumbnail adjustableThumbnail;
	HWND sourceWindow;
	std::size_t sourceIndex;
	WindowFilter windowFilter;

	DoubleRect selectionRect;
	double scale;
	int chromeWidth, chromeHeight;

	HMENU contextMenu;
	HMENU zoomMenu;
	int baseMenuItemCount;
	bool suppressContextMenu;
	int currentCursor;
	bool cursorSet;
	POINTS lastPos;

	// Constants
	static const int MaxMenuTextLength;
	static const int MenuItemBreakPoint;
	static const int CursorMove;
	static const int CursorPan;
	static const int CursorScale;
	static const int CursorNoFunction;
};