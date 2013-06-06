#include "stdafx.h"
#include "AppWindow.h"
#include "Resource.h"
#include "WindowHelper.h"

const int AppWindow::MaxMenuTextLength = 32;
const int AppWindow::MenuItemBreakPoint = 12;

AppWindow::AppWindow() : 
	CWindow(),
	adjustableThumbnail(),
	sourceWindow(NULL),
	contextMenu(NULL),
	sourceIndex(0),
	baseMenuItemCount(0)
{
	lastPos.x = lastPos.y = 0;
}

LRESULT AppWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_LBUTTONDOWN:
		if (wParam == MK_LBUTTON)
		{
			SendMessage(windowHandle, WM_SYSCOMMAND, SC_MOVE|0x0002, NULL);
			SetWindowPos(windowHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE);
			return 0;
		}
	case WM_MOUSEWHEEL:
		if (OnMouseWheel(wParam, lParam)) return 0;
		break;
	case WM_COMMAND:
		if (OnAccelCommand(wParam, lParam)) return 0;
		break;
	case WM_MOUSEMOVE:
		if (OnMouseMove(wParam, lParam)) return 0;
		break;
	case WM_CONTEXTMENU:
		if (OnContextMenu(wParam, lParam)) return 0;
		break;
	case WM_MENUCOMMAND:
		if (OnMenuCommand(wParam, lParam)) return 0;
		break;
	case WM_DESTROY:
		OnDestroy();
		break;
	}

	// Use the default message handling for remaining messages
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void AppWindow::PreCreate(CREATESTRUCT& cs)
{
	accelerators = LoadAccelerators(cs.hInstance, MAKEINTRESOURCE(IDW_MAIN));
	cs.lpszClass = _T("DwmWindowMonitorApp");
	cs.style = WS_POPUP | WS_SIZEBOX | WS_VISIBLE;
}

void AppWindow::OnInitialUpdate()
{
	// Set title and icons
	WindowHelper::SetTitle(windowHandle, instance, IDS_TITLE);
	WindowHelper::SetIcon(windowHandle, instance, IDW_MAIN);
	WindowHelper::SetIcon(windowHandle, instance, IDW_MAIN, true);

	// Set always on top
	SetWindowPos(windowHandle, HWND_TOPMOST, 50, 50, 0, 0, SWP_NOSIZE);

	// Create menu
	HMENU menu = LoadMenu(instance, MAKEINTRESOURCE(IDR_CTXMENU));
	contextMenu = GetSubMenu(menu, 0);
	zoomMenu = GetSubMenu(contextMenu, 2);
	baseMenuItemCount = GetMenuItemCount(contextMenu);

	// Set menu to send WM_MENUCOMMAND instead of WM_COMMAND
	MENUINFO menuInfo;
	menuInfo.cbSize = sizeof MENUINFO;
	menuInfo.fMask = MIM_STYLE;
	menuInfo.dwStyle = MNS_NOTIFYBYPOS;
	SetMenuInfo(contextMenu, &menuInfo);

	// Update menu, filtered windows
	UpdateMenu();
	
	// Set thumbnail
	SelectSource(0);
}

void AppWindow::OnDestroy()
{
	// Kill menu
	if (contextMenu != NULL) 
	{
		DestroyMenu(contextMenu);
		contextMenu = NULL;
	}

	// Unregister thumbnail
	adjustableThumbnail.UnsetThumbnail();

	// Quit
	PostQuitMessage(0);
}

// Scale window via mouse wheel
bool AppWindow::OnMouseWheel(WPARAM const & wParam, LPARAM const & lParam)
{
	short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	if (zDelta != 0)
	{
		adjustableThumbnail.StepScaleThumbnail(windowHandle, sourceWindow, zDelta);
		return true;
	}

	return false;
}

// Offset thumbnail
bool AppWindow::OnMouseMove(WPARAM const & wParam, LPARAM const & lParam)
{
	// Offset thumbnail
	if (wParam == MK_MBUTTON || wParam == (MK_LBUTTON|MK_SHIFT))
	{
		POINTS pos = MAKEPOINTS(lParam);
		if (!(lastPos.x == 0 && lastPos.y == 0))
		{
			adjustableThumbnail.OffsetThumbnail(windowHandle, sourceWindow, 
				pos.x - lastPos.x,
				pos.y - lastPos.y
				);
		}

		lastPos.x = pos.x;
		lastPos.y = pos.y;
		return true;
	}
	else
	{
		lastPos.x = lastPos.y = 0;
	}

	return false;
}

// Handle shortcut keys
bool AppWindow::OnAccelCommand(WPARAM const & wParam, LPARAM const & lParam)
{
	if (HIWORD(wParam) == 1)
	{
		switch (LOWORD(wParam))
		{
		case ID_ACCEL_SWITCH:
			CycleForward();
			return true;
		case ID_ACCEL_RSWITCH:
			CycleBack();
			return true;
		case ID_ACCEL_REFRESH:
			adjustableThumbnail.SetThumbnail(windowHandle, sourceWindow);
			return true;
		}
	}

	return false;
}

// Show context menu
bool AppWindow::OnContextMenu(WPARAM const & wParam, LPARAM const & lParam)
{
	// Get screen coords
	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);

	// TODO Check x and y for -1

	// Show menu
	UpdateMenu();
	TrackPopupMenu(contextMenu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, windowHandle, NULL);

	// Done
	return true;
}

// Process menu item selection
bool AppWindow::OnMenuCommand(WPARAM const & wParam, LPARAM const & lParam)
{
	HMENU sourceMenu = reinterpret_cast<HMENU>(lParam);
	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_ID;

	if (sourceMenu == contextMenu)
	{
		GetMenuItemInfo(contextMenu, wParam, TRUE, &mii);

		switch (mii.wID)
		{
		case ID_EXIT:
			SendMessage(windowHandle, WM_DESTROY, NULL, NULL);
			break;
		case ID_REFRESH:
			adjustableThumbnail.SetThumbnail(windowHandle, sourceWindow);
			break;
		default:
			SelectSource(wParam - baseMenuItemCount);
			break;
		}
	}
	else if (sourceMenu == zoomMenu)
	{
		GetMenuItemInfo(zoomMenu, wParam, TRUE, &mii);
		double newScale;

		switch (mii.wID)
		{
		case ID_ZOOM_25: newScale = 0.25; break;
		case ID_ZOOM_50: newScale = 0.5; break;
		case ID_ZOOM_75: newScale = 0.75; break;
		case ID_ZOOM_100: newScale = 1.0; break;
		case ID_ZOOM_125: newScale = 1.25; break;
		case ID_ZOOM_150: newScale = 1.5; break;
		case ID_ZOOM_175: newScale = 1.75; break;
		case ID_ZOOM_200: newScale = 2.0; break;
		default: return true;
		}
		
		adjustableThumbnail.SetScaleThumbnail(windowHandle, sourceWindow, newScale);
	}
	
	return true;
}

void AppWindow::UpdateMenu()
{
	// Clear
	while(GetMenuItemCount(contextMenu) > baseMenuItemCount)
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
	if (i == baseMenuItemCount) AppendMenu(contextMenu, MF_STRING|MF_GRAYED, 0, 
		WindowHelper::LoadString(instance, IDS_NOWINDOWSFOUND).c_str());
}

void AppWindow::SelectSource(const int& index)
{
	std::size_t size = windowFilter.items.size();

	// Check size
	if (size < 1) return;
	
	// Set source index
	if (index < 0) sourceIndex = size - 1;
	else if ((size_t)index >= size) sourceIndex = 0;
	else sourceIndex = index;

	// Set thumbnail
	sourceWindow = windowFilter.items.at(sourceIndex).hwnd;
	adjustableThumbnail.SetThumbnail(windowHandle, sourceWindow);
}

void AppWindow::CycleForward()
{
	// Refresh list
	windowFilter.Execute();

	// Select next source
	SelectSource((int)sourceIndex + 1);
}

void AppWindow::CycleBack()
{
	// Refresh list
	windowFilter.Execute();

	// Select next source
	SelectSource((int)sourceIndex - 1);
}