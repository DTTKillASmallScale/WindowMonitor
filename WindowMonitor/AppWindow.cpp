#include "stdafx.h"
#include "Resource.h"
#include "AppWindow.h"
#include "CWindowClass.h"
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

class AppWindowClass : public CWindowClass
{
public:
	AppWindowClass() : CWindowClass(L"DwmWindowMonitorApp") { }

	virtual void Configure(WNDCLASSEX & wcex) override
	{
		wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wcex.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(AppWindow::CursorArrow));
		wcex.hbrBackground = CreateSolidBrush(AppWindow::BackgroundColour);
	}
};

class AppWindowStruct : public CWindowStruct
{
public:
	virtual void Configure(CREATESTRUCT & cs) override
	{
		cs.lpszClass = _T("DwmWindowMonitorApp");
		cs.lpszName = _T("New Window");
		cs.style = WS_VISIBLE | WS_POPUP | WS_SYSMENU | WS_THICKFRAME | WS_BORDER | WS_MINIMIZEBOX;
		cs.dwExStyle = NULL;
		cs.hwndParent = NULL;
		cs.x = 0;
		cs.y = 0;
		cs.cx = 640;
		cs.cy = 480;
		cs.hMenu = NULL;
	}
};

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
	SetWindowClass<AppWindowClass>();
	SetWindowStruct<AppWindowStruct>();
}

void AppWindow::OnCreate()
{
	// Set window options
	WindowHelper::SetTitle(GetWindowHandle(), WindowHelper::GetCurrentModuleHandle(), IDS_TITLE);
	WindowHelper::SetIcon(GetWindowHandle(), WindowHelper::GetCurrentModuleHandle(), IDW_MAIN);
	WindowHelper::SetIcon(GetWindowHandle(), WindowHelper::GetCurrentModuleHandle(), IDW_MAIN, true);
	SetLayeredWindowAttributes(GetWindowHandle(), AppWindow::BackgroundColour, 0, LWA_COLORKEY);
	CenterWindow();

	SetAccelerators(IDW_MAIN);

	// Create menu
	menu = LoadMenu(WindowHelper::GetCurrentModuleHandle(), MAKEINTRESOURCE(IDR_CTXMENU));
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
	DWORD style = static_cast<DWORD>(GetWindowLong(GetWindowHandle(), GWL_STYLE));
	style ^= WS_THICKFRAME;
	style ^= WS_BORDER;

	// Set new style
	SetWindowLongPtr(GetWindowHandle(), GWL_STYLE, style);
	UpdateWindow(); // Recalc chrome size

	// Update context menu
	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;
	GetMenuItemInfo(contextMenu, ID_MENU_TOGGLEBORDER, FALSE, &mii);
	mii.fState ^= MFS_CHECKED;
	SetMenuItemInfo(contextMenu, ID_MENU_TOGGLEBORDER, FALSE, &mii);
}

void AppWindow::ToggleFullscreen()
{
	DWORD style = static_cast<DWORD>(GetWindowLong(GetWindowHandle(), GWL_STYLE));

	// Get context menu
	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;
	GetMenuItemInfo(contextMenu, ID_MENU_FULLSCREEN, FALSE, &mii);

	if ((mii.fState & MFS_CHECKED) == 0)
	{
		// Set fullscreen
		if ((style & WS_THICKFRAME) != 0) ToggleBorder();
		windowMonitor->ScaleToFitMonitor(GetWindowHandle(), true);
	}
	else
	{
		// Clear fullscreen
		if ((style & WS_THICKFRAME) == 0) ToggleBorder();
		windowMonitor->ScaleToFitMonitor(GetWindowHandle());
	}

	// Update context menu
	mii.fState ^= MFS_CHECKED;
	SetMenuItemInfo(contextMenu, ID_MENU_FULLSCREEN, FALSE, &mii);
}

void AppWindow::ToggleClickThrough()
{
	// Toggle window transparency
	DWORD style = static_cast<DWORD>(GetWindowLong(GetWindowHandle(), GWL_EXSTYLE));
	style ^= WS_EX_TRANSPARENT;
	style ^= WS_EX_LAYERED;

	// Set new style
	SetWindowLongPtr(GetWindowHandle(), GWL_EXSTYLE, style);
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
		SendMessage(GetWindowHandle(), WM_SETCURSOR, WPARAM(GetWindowHandle()), (LPARAM)MAKELONG(HTCLIENT, WM_MOUSEMOVE));
	}
}

void AppWindow::UpdateSourceMenu()
{
	// Update sources
	size_t checksum = windowMonitor->UpdateSources();
	if (lastChecksum == checksum) return;
	lastChecksum = checksum;

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
		WindowHelper::GetResourceString(WindowHelper::GetCurrentModuleHandle(), IDS_NOWINDOWSFOUND, text);
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
		WindowHelper::GetResourceString(WindowHelper::GetCurrentModuleHandle(), IDS_NOPRESETSFOUND, text);
		AppendMenuW(presetsMenu, MF_STRING | MF_GRAYED, 0, text.c_str());
	}
}

void AppWindow::UpdateWindow()
{
	long width = static_cast<long>(windowMonitor->GetScaledWidth());
	long height = static_cast<long>(windowMonitor->GetScaledHeight());

	// Calc window dimensions
	RECT windowRect{ 0, 0, width, height };
	DWORD dwStyle = static_cast<DWORD>(GetWindowLong(GetWindowHandle(), GWL_STYLE));
	AdjustWindowRect(&windowRect, dwStyle, FALSE);

	// Set window size
	SetWindowPos(GetWindowHandle(), NULL, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

	// Get size of window chrome
	chromeWidth = (windowRect.right - windowRect.left) - width;
	chromeHeight = (windowRect.bottom - windowRect.top) - height;
}

void AppWindow::CenterWindow()
{
	RECT windowRect, monitorRect;
	GetWindowRect(GetWindowHandle(), &windowRect);
	WindowHelper::GetMonitorRect(GetWindowHandle(), monitorRect);
	int x = (monitorRect.right + monitorRect.left - windowRect.right + windowRect.left) / 2;
	int y = (monitorRect.bottom + monitorRect.top - windowRect.bottom + windowRect.top) / 2;
	SetWindowPos(GetWindowHandle(), HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
}

void AppWindow::OnWindowMonitorEvent(WindowMonitorEvent const & event)
{
	switch (event)
	{
	case WindowMonitorEvent::SourceSelected:
		adjustableThumbnail.SetThumbnail(GetWindowHandle(), windowMonitor->GetSourceWindow());
		windowMonitor->ResetAndScaleToFitMonitor(GetWindowHandle());
		lastChecksum = 0;
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
	{
		// Toggle border if set
		DWORD style = static_cast<DWORD>(GetWindowLong(GetWindowHandle(), GWL_STYLE));
		if ((style & WS_THICKFRAME) == 0) ToggleBorder();

		// Clear fullscreen menu
		MENUITEMINFO mii;
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_STATE;
		mii.fState = MFS_ENABLED;
		SetMenuItemInfo(contextMenu, ID_MENU_FULLSCREEN, FALSE, &mii);

		// Continue onto next case
	}
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