#include "stdafx.h"
#include "AppWindow.h"
#include "Resource.h"
#include "WindowHelper.h"
#include <sstream>

const int AppWindow::MaxMenuTextLength = 32;
const int AppWindow::MenuItemBreakPoint = 24;
const int AppWindow::CursorArrow = 32512;
const int AppWindow::CursorMove = 32646;
const int AppWindow::CursorPan = 32649;
const int AppWindow::CursorScale = 32642;
const int AppWindow::CursorNoFunction = 32648;

AppWindow::AppWindow() :
	CWindow(),
	adjustableThumbnail(),
	sourceWindow(NULL),
	sourceIndex(0),
	currentViewSetting(),
	chromeWidth(0),
	chromeHeight(0),
	contextMenu(NULL),
	zoomMenu(NULL),
	baseMenuItemCount(0),
	suppressContextMenu(false),
	currentCursor(0),
	cursorSet(false)
{
	lastPos.x = lastPos.y = 0;
}

void AppWindow::PreCreate(CREATESTRUCT & cs, WNDCLASSEX & wcex)
{
	accelerators = LoadAccelerators(cs.hInstance, MAKEINTRESOURCE(IDW_MAIN));
	cs.lpszClass = _T("DwmWindowMonitorApp");
	cs.style = WS_POPUPWINDOW | WS_SIZEBOX;
	wcex.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(CursorArrow));
}

void AppWindow::OnInitialUpdate()
{
	// Set title and icons
	WindowHelper::SetTitle(windowHandle, instance, IDS_TITLE);
	WindowHelper::SetIcon(windowHandle, instance, IDW_MAIN);
	WindowHelper::SetIcon(windowHandle, instance, IDW_MAIN, true);

	// Create menu
	HMENU menu = LoadMenu(instance, MAKEINTRESOURCE(IDR_CTXMENU));
	contextMenu = GetSubMenu(menu, 0);
	zoomMenu = GetSubMenu(contextMenu, 0);
	baseMenuItemCount = GetMenuItemCount(contextMenu);

	// Set menu to send WM_MENUCOMMAND instead of WM_COMMAND
	MENUINFO menuInfo;
	menuInfo.cbSize = sizeof MENUINFO;
	menuInfo.fMask = MIM_STYLE;
	menuInfo.dwStyle = MNS_NOTIFYBYPOS;
	SetMenuInfo(contextMenu, &menuInfo);

	// Update menu, filtered windows
	UpdateMenu();

	// Select source window
	SelectSource(0);

	// Show window
	ShowWindow(windowHandle, SW_SHOWNORMAL);
}

void AppWindow::OnDestroy()
{
	adjustableThumbnail.UnsetThumbnail();

	// Quit
	PostQuitMessage(0);
}

void AppWindow::UpdateWindow()
{
	long width, height;
	currentViewSetting.GetScaledDimensions(width, height);

	// Calc window dimensions
	RECT windowRect{ 0, 0, width, height };
	DWORD dwStyle = static_cast<DWORD>(GetWindowLong(windowHandle, GWL_STYLE));
	AdjustWindowRect(&windowRect, dwStyle, FALSE);

	// Set window pos
	SetWindowPos(windowHandle, HWND_TOPMOST, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_NOMOVE);

	// Get size of window chrome
	chromeWidth = (windowRect.right - windowRect.left) - static_cast<long>(currentViewSetting.GetWidth());
	chromeHeight = (windowRect.bottom - windowRect.top) - static_cast<long>(currentViewSetting.GetHeight());
}

void AppWindow::UpdateThumbnail()
{
	RECT sourceRect;
	GetClientRect(sourceWindow, &sourceRect);
	currentViewSetting.GetScaledRect(static_cast<double>(sourceRect.right), static_cast<double>(sourceRect.bottom), sourceRect);
	adjustableThumbnail.SetSize(sourceRect);
}

void AppWindow::SelectSource(int const & index)
{
	// Get filtered windows
	windowFilter.Execute();

	// Get size
	std::size_t size = windowFilter.items.size();
	if (size < 1) return;

	// Set source index
	if (index < 0) sourceIndex = size - 1;
	else if (static_cast<size_t>(index) >= size) sourceIndex = 0;
	else sourceIndex = index;

	// Get source window handle
	sourceWindow = windowFilter.items.at(sourceIndex).hwnd;

	// Set thumbnail to source
	adjustableThumbnail.SetThumbnail(windowHandle, sourceWindow);

	// Reset
	Reset();
}

void AppWindow::CycleForward()
{
	// Refresh list
	windowFilter.Execute();

	// Select next source
	SelectSource(static_cast<int>(sourceIndex)+1);
}

void AppWindow::CycleBack()
{
	// Refresh list
	windowFilter.Execute();

	// Select next source
	SelectSource(static_cast<int>(sourceIndex)-1);
}

void AppWindow::Reset()
{
	// Set selection
	currentViewSetting.SetFromClientRect(sourceWindow);

	// Set scale
	RECT monitorRect;
	GetMonitorRect(monitorRect);
	currentViewSetting.SetScaleToMonitorSize(monitorRect);

	// Update window
	UpdateWindow();

	// Center window
	RECT windowRect;
	GetWindowRect(windowHandle, &windowRect);
	int x = (monitorRect.right + monitorRect.left - windowRect.right + windowRect.left) / 2;
	int y = (monitorRect.bottom + monitorRect.top - windowRect.bottom + windowRect.top) / 2;
	SetWindowPos(windowHandle, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);

	// Update thumbnail
	UpdateThumbnail();
}

void AppWindow::ToggleBorder()
{
	// Get old style
	DWORD oldStyle = static_cast<DWORD>(GetWindowLong(windowHandle, GWL_STYLE));

	// Calc new style
	LONG_PTR newStyle = WS_VISIBLE | WS_POPUPWINDOW;
	if ((oldStyle & WS_SIZEBOX) == false) newStyle = newStyle | WS_SIZEBOX;

	// Set new style
	SetWindowLongPtr(windowHandle, GWL_STYLE, newStyle);
	UpdateWindow();
}

void AppWindow::SetContextualCursor()
{
	int prevCursor = currentCursor;
	bool lmb = static_cast<unsigned short>(GetKeyState(VK_LBUTTON)) >> 15 == 1;
	bool shift = static_cast<unsigned short>(GetKeyState(VK_SHIFT)) >> 15 == 1;
	bool control = static_cast<unsigned short>(GetKeyState(VK_CONTROL)) >> 15 == 1;

	if (lmb && !shift && !control) currentCursor = CursorMove;
	else if (shift && !control) currentCursor = CursorPan;
	else if (!shift && control) currentCursor = CursorScale;
	else if (shift && control) currentCursor = CursorNoFunction;
	else currentCursor = 0;

	if (prevCursor != currentCursor) 
	{
		cursorSet = false;
		SendMessage(windowHandle, WM_SETCURSOR, WPARAM(windowHandle), (LPARAM)MAKELONG(HTCLIENT, WM_MOUSEMOVE));
	}
}

void AppWindow::UpdateMenu()
{
	// Clear
	while (GetMenuItemCount(contextMenu) > baseMenuItemCount)
		DeleteMenu(contextMenu, baseMenuItemCount, MF_BYPOSITION);

	// Get filtered windows
	windowFilter.Execute();

	// Add items
	int i = baseMenuItemCount;
	for (auto it = windowFilter.items.begin(); it != windowFilter.items.end(); ++it)
	{
		// Get title text
		std::wstring text((*it).title.substr(0, AppWindow::MaxMenuTextLength));

		// Truncate
		if ((*it).title.length() > AppWindow::MaxMenuTextLength) text.append(L"...");

		// Create menu item
		bool breakMenu = i % AppWindow::MenuItemBreakPoint == 0;
		AppendMenu(contextMenu, MF_STRING | (breakMenu ? MF_MENUBARBREAK : 0), i, text.c_str());

		// Next item
		++i;
	}

	// Add blank item if no windows were added
	if (i == baseMenuItemCount) AppendMenu(contextMenu, MF_STRING | MF_GRAYED, 0,
		WindowHelper::LoadString(instance, IDS_NOWINDOWSFOUND).c_str());
}

void AppWindow::GetMonitorRect(RECT & rect)
{
	HMONITOR monitor = MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST);
	MONITORINFO monitorInfo;
	monitorInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &monitorInfo);
	rect.bottom = monitorInfo.rcMonitor.bottom;
	rect.left = monitorInfo.rcMonitor.left;
	rect.right = monitorInfo.rcMonitor.right;
	rect.top = monitorInfo.rcMonitor.top;
}
