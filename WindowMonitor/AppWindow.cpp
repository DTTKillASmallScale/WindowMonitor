#include "stdafx.h"
#include "Resource.h"
#include "AppWindow.h"
#include "WindowMonitor.h"
#include "WindowHelper.h"

const int AppWindow::MaxMenuTextLength = 32;
const int AppWindow::MenuItemBreakPoint = 24;
const int AppWindow::CursorArrow = 32512;
const int AppWindow::CursorMove = 32646;
const int AppWindow::CursorPan = 32649;
const int AppWindow::CursorScale = 32642;
const int AppWindow::CursorNoFunction = 32648;
const COLORREF AppWindow::BackgroundColour = RGB(255, 255, 255);

AppWindow::AppWindow(WindowMonitor * const windowMonitor, PresetWindow * const presetWindow) :
	CWindow(),
	windowMonitor(windowMonitor),
	presetWindow(presetWindow),
	adjustableThumbnail(),
	menu(NULL),
	contextMenu(NULL),
	presetsMenu(NULL),
	zoomMenu(NULL),
	baseMenuItemCount(0),
	suppressContextMenu(false),
	currentCursor(0),
	cursorSet(false),
	chromeWidth(0),
	chromeHeight(0)
{
	lastPos.x = lastPos.y = 0;
}

void AppWindow::PreCreate(CREATESTRUCT & cs, WNDCLASSEX & wcex)
{
	accelerators = LoadAccelerators(cs.hInstance, MAKEINTRESOURCE(IDW_MAIN));
	cs.lpszClass = _T("DwmWindowMonitorApp");
	cs.style = WS_VISIBLE | WS_POPUP | WS_SYSMENU | WS_THICKFRAME | WS_BORDER | WS_MINIMIZEBOX;
	wcex.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(CursorArrow));
	wcex.hbrBackground = CreateSolidBrush(AppWindow::BackgroundColour);
}

void AppWindow::OnCreate()
{
	// Set window options
	WindowHelper::SetTitle(windowHandle, instance, IDS_TITLE);
	WindowHelper::SetIcon(windowHandle, instance, IDW_MAIN);
	WindowHelper::SetIcon(windowHandle, instance, IDW_MAIN, true);
	SetLayeredWindowAttributes(windowHandle, AppWindow::BackgroundColour, 0, LWA_COLORKEY);
	SetWindowPos(windowHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	// Create menu
	menu = LoadMenu(instance, MAKEINTRESOURCE(IDR_CTXMENU));
	contextMenu = GetSubMenu(menu, 0);
	presetsMenu = GetSubMenu(contextMenu, 0);
	zoomMenu = GetSubMenu(contextMenu, 1);
	baseMenuItemCount = GetMenuItemCount(contextMenu);

	// Set menu to send WM_MENUCOMMAND instead of WM_COMMAND
	MENUINFO menuInfo;
	menuInfo.cbSize = sizeof MENUINFO;
	menuInfo.fMask = MIM_STYLE;
	menuInfo.dwStyle = MNS_NOTIFYBYPOS;
	SetMenuInfo(contextMenu, &menuInfo);

	// Add observer
	windowMonitor->RegisterObserver(this);

	// Select first source
	windowMonitor->SelectSource(0);
}

void AppWindow::OnDestroy()
{
	// Remove view setting observer
	windowMonitor->UnregisterObserver(this);

	// Close preset window
	presetWindow->Destroy();

	// Clean up resources
	adjustableThumbnail.UnsetThumbnail();
	DestroyMenu(menu);

	// Close program
	PostQuitMessage(0);
}

void AppWindow::ToggleBorder()
{
	// Toggle window border
	DWORD style = static_cast<DWORD>(GetWindowLong(windowHandle, GWL_STYLE));
	style ^= WS_THICKFRAME;
	style ^= WS_BORDER;

	// Set new style
	SetWindowLongPtr(windowHandle, GWL_STYLE, style);
	UpdateWindow(); // Recalc chrome size

	// Update content menu
	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;
	GetMenuItemInfo(contextMenu, ID_MENU_TOGGLEBORDER, FALSE, &mii);
	mii.fState ^= MFS_CHECKED;
	SetMenuItemInfo(contextMenu, ID_MENU_TOGGLEBORDER, FALSE, &mii);
}

void AppWindow::ToggleClickThrough()
{
	// Toggle window transparency
	DWORD style = static_cast<DWORD>(GetWindowLong(windowHandle, GWL_EXSTYLE));
	style ^= WS_EX_TRANSPARENT;
	style ^= WS_EX_LAYERED;

	// Set new style
	SetWindowLongPtr(windowHandle, GWL_EXSTYLE, style);
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

void AppWindow::UpdateSourceMenu()
{
	// Clear
	while (GetMenuItemCount(contextMenu) > baseMenuItemCount)
		DeleteMenu(contextMenu, baseMenuItemCount, MF_BYPOSITION);

	// Add items
	std::wstring text;
	int identifier = baseMenuItemCount;
	windowMonitor->IterateSources([&](std::wstring const & title, bool const & selected)
	{
		// Get title text
		text.assign(title.substr(0, AppWindow::MaxMenuTextLength));

		// Add ellipsis if truncated
		if (title.length() > AppWindow::MaxMenuTextLength) text.append(L"...");

		// Create menu item
		bool breakMenu = identifier % AppWindow::MenuItemBreakPoint == 0;
		AppendMenu(contextMenu, MF_STRING | (breakMenu ? MF_MENUBARBREAK : 0) | (selected ? MF_CHECKED : 0), identifier, text.c_str());

		// Next item
		++identifier;
	});

	// Add blank item if no windows were added
	if (identifier == baseMenuItemCount) 
	{
		std::wstring text;
		WindowHelper::GetResourceString(instance, IDS_NOWINDOWSFOUND, text);
		AppendMenuW(contextMenu, MF_STRING | MF_GRAYED, 0, text.c_str());
	}
}

void AppWindow::UpdatePresetMenu()
{
	// Clear
	while (GetMenuItemCount(presetsMenu) > 2)
		DeleteMenu(presetsMenu, 2, MF_BYPOSITION);

	// Add items
	std::wstring text;
	int identifier = 2;
	windowMonitor->IteratePresets([&](std::wstring const & name, bool const & selected)
	{
		// Create menu item
		bool breakMenu = identifier % AppWindow::MenuItemBreakPoint == 0;
		AppendMenu(presetsMenu, MF_STRING | (breakMenu ? MF_MENUBARBREAK : 0) | (selected ? MF_CHECKED : 0), identifier, name.c_str());

		// Next item
		++identifier;
	});

	// Add blank item if no windows were added
	if (identifier == 2) 
	{
		std::wstring text;
		WindowHelper::GetResourceString(instance, IDS_NOPRESETSFOUND, text);
		AppendMenuW(presetsMenu, MF_STRING | MF_GRAYED, 0, text.c_str());
	}
}

void AppWindow::UpdateWindow()
{
	long width = static_cast<long>(windowMonitor->GetScaledWidth());
	long height = static_cast<long>(windowMonitor->GetScaledHeight());

	// Calc window dimensions
	RECT windowRect{ 0, 0, width, height };
	DWORD dwStyle = static_cast<DWORD>(GetWindowLong(windowHandle, GWL_STYLE));
	AdjustWindowRect(&windowRect, dwStyle, FALSE);

	// Set window size
	SetWindowPos(windowHandle, NULL, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

	// Get size of window chrome
	chromeWidth = (windowRect.right - windowRect.left) - width;
	chromeHeight = (windowRect.bottom - windowRect.top) - height;
}

void AppWindow::CenterWindow()
{
	RECT windowRect, monitorRect;
	GetWindowRect(windowHandle, &windowRect);
	WindowHelper::GetMonitorRect(windowHandle, monitorRect);
	int x = (monitorRect.right + monitorRect.left - windowRect.right + windowRect.left) / 2;
	int y = (monitorRect.bottom + monitorRect.top - windowRect.bottom + windowRect.top) / 2;
	SetWindowPos(windowHandle, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
}

void AppWindow::OnWindowMonitorEvent(WindowMonitorEvent const & event)
{
	switch (event)
	{
	case WindowMonitorEvent::SourceSelected:
		adjustableThumbnail.SetThumbnail(windowHandle, windowMonitor->GetSourceWindow());
		windowMonitor->ResetAndScaleToFitMonitor(windowHandle);
		break;
	case WindowMonitorEvent::Moved:
		adjustableThumbnail.SetSize(windowMonitor->GetScaledRect());
		break;
	case WindowMonitorEvent::Cropped:
		UpdateWindow();
		break;
	case WindowMonitorEvent::PresetSelected:
	case WindowMonitorEvent::Scaled:
		adjustableThumbnail.SetSize(windowMonitor->GetScaledRect());
		UpdateWindow();
		break;
	case WindowMonitorEvent::DimensionsReset:
	case WindowMonitorEvent::ScaledToMonitor:
		adjustableThumbnail.SetSize(windowMonitor->GetScaledRect());
		UpdateWindow();
		CenterWindow();
		break;
	case WindowMonitorEvent::ScaledToWindow:
		adjustableThumbnail.SetSize(windowMonitor->GetScaledRect());
		break;
	default:
		break;
	}
}