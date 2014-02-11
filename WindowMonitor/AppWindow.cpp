#include "stdafx.h"
#include "AppWindow.h"
#include "Resource.h"
#include "WindowFilter.h"
#include "PresetManager.h"
#include "ViewSetting.h"
#include "WindowHelper.h"

const int AppWindow::MaxMenuTextLength = 32;
const int AppWindow::MenuItemBreakPoint = 24;
const int AppWindow::CursorArrow = 32512;
const int AppWindow::CursorMove = 32646;
const int AppWindow::CursorPan = 32649;
const int AppWindow::CursorScale = 32642;
const int AppWindow::CursorNoFunction = 32648;

AppWindow::AppWindow(WindowFilter * const windowFilter, PresetManager * const presetManager, ViewSetting * const currentViewSetting) :
	CWindow(),
	windowFilter(windowFilter),
	presetManager(presetManager),
	currentViewSetting(currentViewSetting),
	presetWindow(presetManager, currentViewSetting),
	adjustableThumbnail(),
	sourceWindow(NULL),
	sourceIndex(0),
	chromeWidth(0),
	chromeHeight(0),
	menu(NULL),
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
	cs.style = WS_VISIBLE | WS_POPUP | WS_SYSMENU | WS_THICKFRAME | WS_BORDER;
	wcex.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(CursorArrow));
	wcex.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
}

void AppWindow::OnCreate()
{
	// Set window options
	WindowHelper::SetTitle(windowHandle, instance, IDS_TITLE);
	WindowHelper::SetIcon(windowHandle, instance, IDW_MAIN);
	WindowHelper::SetIcon(windowHandle, instance, IDW_MAIN, true);
	SetLayeredWindowAttributes(windowHandle, RGB(255, 255, 255), 0, LWA_COLORKEY);
	SetWindowPos(windowHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	// Create menu
	menu = LoadMenu(instance, MAKEINTRESOURCE(IDR_CTXMENU));
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

	// Add view setting observer
	currentViewSetting->RegisterObserver(this);
}

void AppWindow::OnDestroy()
{
	presetWindow.Destroy();
	currentViewSetting->UnregisterObserver(this);
	adjustableThumbnail.UnsetThumbnail();
	DestroyMenu(menu);
	PostQuitMessage(0);
}

void AppWindow::UpdateWindow()
{
	long width, height;
	currentViewSetting->GetScaledDimensions(width, height);

	// Calc window dimensions
	RECT windowRect{ 0, 0, width, height };
	DWORD dwStyle = static_cast<DWORD>(GetWindowLong(windowHandle, GWL_STYLE));
	AdjustWindowRect(&windowRect, dwStyle, FALSE);

	// Set window size
	SetWindowPos(windowHandle, NULL, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	// Get size of window chrome
	chromeWidth = (windowRect.right - windowRect.left) - static_cast<long>(currentViewSetting->GetWidth());
	chromeHeight = (windowRect.bottom - windowRect.top) - static_cast<long>(currentViewSetting->GetHeight());
}

void AppWindow::UpdateThumbnail()
{
	RECT sourceRect;
	GetClientRect(sourceWindow, &sourceRect);
	currentViewSetting->GetScaledRect(static_cast<double>(sourceRect.right), static_cast<double>(sourceRect.bottom), sourceRect);
	adjustableThumbnail.SetSize(sourceRect);
}

void AppWindow::SelectSource(int const & index)
{
	// Get filtered windows
	windowFilter->Refresh();

	// Get size
	std::size_t size = windowFilter->ItemCount();
	if (size < 1) return;

	// Set source index
	if (index < 0) sourceIndex = size - 1;
	else if (static_cast<size_t>(index) >= size) sourceIndex = 0;
	else sourceIndex = index;

	// Get source window handle
	sourceWindow = windowFilter->GetWindowHandle(sourceIndex);

	// Set thumbnail to source
	adjustableThumbnail.SetThumbnail(windowHandle, sourceWindow);

	// Reset
	Reset();
}

void AppWindow::CycleForward()
{
	// Refresh list
	windowFilter->Refresh();

	// Select next source
	SelectSource(static_cast<int>(sourceIndex)+1);
}

void AppWindow::CycleBack()
{
	// Refresh list
	windowFilter->Refresh();

	// Select next source
	SelectSource(static_cast<int>(sourceIndex)-1);
}

void AppWindow::Reset()
{
	// Set selection
	currentViewSetting->SetFromClientRect(sourceWindow);

	// Set scale
	RECT monitorRect;
	WindowHelper::GetMonitorRect(windowHandle, monitorRect);
	currentViewSetting->SetScaleToMonitorSize(monitorRect);

	// Update window
	UpdateWindow();

	// Center window
	RECT windowRect;
	GetWindowRect(windowHandle, &windowRect);
	int x = (monitorRect.right + monitorRect.left - windowRect.right + windowRect.left) / 2;
	int y = (monitorRect.bottom + monitorRect.top - windowRect.bottom + windowRect.top) / 2;
	SetWindowPos(windowHandle, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	// Update thumbnail
	UpdateThumbnail();
}

void AppWindow::ToggleBorder()
{
	// Get old style
	DWORD oldStyle = static_cast<DWORD>(GetWindowLong(windowHandle, GWL_STYLE));

	// Calc new style
	LONG_PTR newStyle = WS_VISIBLE | WS_POPUP | WS_SYSMENU;
	if ((oldStyle & WS_THICKFRAME) == 0) newStyle = newStyle | WS_THICKFRAME | WS_BORDER;

	// Set new style
	SetWindowLongPtr(windowHandle, GWL_STYLE, newStyle);
	UpdateWindow();
}

void AppWindow::ToggleClickThrough()
{
	// Get old style
	DWORD oldStyle = static_cast<DWORD>(GetWindowLong(windowHandle, GWL_EXSTYLE));

	// Calc new style
	LONG_PTR newStyle = NULL;
	if ((oldStyle & WS_EX_TRANSPARENT) == false) newStyle = newStyle | WS_EX_TRANSPARENT | WS_EX_LAYERED;

	// Set new style
	SetWindowLongPtr(windowHandle, GWL_EXSTYLE, newStyle);
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
	windowFilter->Refresh();

	// Add items
	std::wstring text;
	int identifier = baseMenuItemCount;
	windowFilter->IterateItems([&](WindowFilterItem const & item)
	{
		// Get title text
		text.assign(item.title.substr(0, AppWindow::MaxMenuTextLength));

		// Add ellipsis if truncated
		if (item.title.length() > AppWindow::MaxMenuTextLength) text.append(L"...");

		// Create menu item
		bool breakMenu = identifier % AppWindow::MenuItemBreakPoint == 0;
		AppendMenu(contextMenu, MF_STRING | (breakMenu ? MF_MENUBARBREAK : 0), identifier, text.c_str());

		// Next item
		++identifier;
	});

	// Add blank item if no windows were added
	if (identifier == baseMenuItemCount) AppendMenu(contextMenu, MF_STRING | MF_GRAYED, 0,
		WindowHelper::LoadString(instance, IDS_NOWINDOWSFOUND).c_str());
}

void AppWindow::ViewSettingUpdated(ViewSettingObserverState const & state)
{
	if (state == ViewSettingObserverState::CopyFrom)
	{
		UpdateWindow();
		UpdateThumbnail();
	}
}