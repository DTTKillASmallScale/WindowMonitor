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
	bool OnAccelCommand(const WPARAM& wParam, const LPARAM& lParam);
	bool OnMenuCommand(const WPARAM& wParam, const LPARAM& lParam);
	bool OnContextMenu(const WPARAM& wParam, const LPARAM& lParam);
	bool OnMouseWheel(const WPARAM& wParam, const LPARAM& lParam);
	bool OnMouseMove(const WPARAM& wParam, const LPARAM& lParam);
	void OnDestroy();

	void UpdateMenu();
	void CycleForward();
	void CycleBack();
	void SelectSource(const int& index);

	// Vars
	HWND sourceWindow;
	HMENU contextMenu;
	std::size_t sourceIndex;
	WindowFilter windowFilter;
	POINTS lastPos;
	AdjustableThumbnail adjustableThumbnail;

	static const std::size_t SKIP_MENU_ITEMS;
};