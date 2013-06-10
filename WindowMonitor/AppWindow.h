#pragma once
#include "CWindow.h"
#include "AdjustableThumbnail.h"
#include "WindowFilter.h"

class AppWindow : public CWindow
{
public:
	AppWindow();
	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	virtual void PreCreate(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();

private:
	// Events
	bool OnDoubleClick(WPARAM const & wParam, LPARAM const & lParam);
	bool OnSetCursor(WPARAM const & wParam, LPARAM const & lParam);
	bool OnKeyDown(WPARAM const & wParam, LPARAM const & lParam);
	bool OnKeyUp(WPARAM const & wParam, LPARAM const & lParam);
	bool OnAccelCommand(WPARAM const & wParam, LPARAM const & lParam);
	bool OnMenuCommand(WPARAM const & wParam, LPARAM const & lParam);
	bool OnContextMenu(WPARAM const & wParam, LPARAM const & lParam);
	bool OnMouseWheel(WPARAM const & wParam, LPARAM const & lParam);
	bool OnMouseMove(WPARAM const & wParam, LPARAM const & lParam);
	void OnDestroy();

	void UpdateMenu();
	void CycleForward();
	void CycleBack();
	void SelectSource(int const & index);
	void SetCurrentCursor(int const & id);

	// Vars
	HWND sourceWindow;
	HMENU contextMenu;
	HMENU zoomMenu;
	int baseMenuItemCount;
	int currentCursor;
	bool suppressContextMenu;
	bool thickFrame;

	std::size_t sourceIndex;
	WindowFilter windowFilter;
	POINTS lastPos;

	AdjustableThumbnail adjustableThumbnail;

	static const int MaxMenuTextLength;
	static const int MenuItemBreakPoint;
	static const int CursorArrow;
	static const int CursorMove;
	static const int CursorScale;
};