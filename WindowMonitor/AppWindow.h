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
	void OnSizing(WPARAM const & wParam, LPARAM const & lParam);
	bool OnMouseMove(WPARAM const & wParam, LPARAM const & lParam);
	bool OnAccelCommand(WPARAM const & wParam, LPARAM const & lParam);
	bool OnContextMenu(WPARAM const & wParam, LPARAM const & lParam);
	bool OnMenuCommand(WPARAM const & wParam, LPARAM const & lParam);
	bool OnKeyDown(WPARAM const & wParam, LPARAM const & lParam);
	bool OnSetCursor(WPARAM const & wParam, LPARAM const & lParam);
	void OnDestroy();

	// Methods
	void SetWindowSize();
	void SetWindowSize(double const & scale);
	void ScaleThumbnail();
	void SelectSource(int const & index);
	void ToggleBorder();
	void CalcScale();
	void UpdateMenu();
	void CycleForward();
	void CycleBack();
	void Reset();
	void SetContextualCursor();

	// Vars
	AdjustableThumbnail adjustableThumbnail;
	HWND sourceWindow;
	WindowFilter windowFilter;
	std::size_t sourceIndex;

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