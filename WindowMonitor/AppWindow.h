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
	void OnSizing(WPARAM const & wParam, LPARAM const & lParam);
	void OnSize(WPARAM const & wParam, LPARAM const & lParam);
	bool OnMouseMove(WPARAM const & wParam, LPARAM const & lParam);
	void OnDestroy();

	// Methods
	void SetWindowSize(double const & scale);
	void SelectSource(int const & index);

	// Vars
	RECT selectionRect;
	int chromeWidth, chromeHeight;
	WindowFilter windowFilter;
	std::size_t sourceIndex;
	HWND sourceWindow;
	AdjustableThumbnail adjustableThumbnail;

	// Constants
	static const int MaxMenuTextLength;
	static const int MenuItemBreakPoint;
	static const int CursorArrow;
	static const int CursorMove;
	static const int CursorScale;
};