#include "stdafx.h"
#include "AppWindow.h"
#include "Resource.h"
#include "WindowHelper.h"

const std::size_t AppWindow::SKIP_MENU_ITEMS = 3;

AppWindow::AppWindow() : 
	CWindow(),
	adjustableThumbnail(),
	sourceWindow(NULL),
	contextMenu(NULL),
	sourceIndex(0)
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
}

void AppWindow::OnInitialUpdate()
{
	// Set title and icons
	SetWindowText(windowHandle, WindowHelper::LoadString(instance, IDS_TITLE).c_str());
	WindowHelper::SetIcon(instance, windowHandle, IDW_MAIN);
	WindowHelper::SetIcon(instance, windowHandle, IDW_MAIN, true);

	// Remove title bar
	SetWindowLong(windowHandle, GWL_STYLE, WS_SIZEBOX);
	SetWindowPos(windowHandle, HWND_TOPMOST, 50, 50, 300, 200, SWP_SHOWWINDOW);

	// Create menu
	contextMenu = CreatePopupMenu();

	// Set menu to send WM_MENUCOMMAND instead of WM_COMMAND
	MENUINFO menuInfo;
	menuInfo.cbSize = sizeof MENUINFO;
	menuInfo.fMask = MIM_STYLE;
	menuInfo.dwStyle = MNS_NOTIFYBYPOS;
	SetMenuInfo(contextMenu, &menuInfo);

	// Add default items
	AppendMenu(contextMenu, MF_STRING, 0, WindowHelper::LoadString(instance, IDS_REFRESH).c_str());
	AppendMenu(contextMenu, MF_STRING, 0, WindowHelper::LoadString(instance, IDS_EXIT).c_str());
	AppendMenu(contextMenu, MF_SEPARATOR, 0, NULL);

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
	::PostQuitMessage(0);
}

// Scale window via mouse wheel
bool AppWindow::OnMouseWheel(const WPARAM& wParam, const LPARAM& lParam)
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
bool AppWindow::OnMouseMove(const WPARAM& wParam, const LPARAM& lParam)
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
bool AppWindow::OnAccelCommand(const WPARAM& wParam, const LPARAM& lParam)
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
bool AppWindow::OnContextMenu(const WPARAM& wParam, const LPARAM& lParam)
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
bool AppWindow::OnMenuCommand(const WPARAM& wParam, const LPARAM& lParam)
{
	if (wParam == 0) 
	{
		// Refresh
		adjustableThumbnail.SetThumbnail(windowHandle, sourceWindow);
	}
	else if (wParam == 1)
	{
		// Exit
		SendMessage(windowHandle, WM_DESTROY, NULL, NULL);
	}
	else
	{
		// Set thumbnail
		SelectSource(wParam - SKIP_MENU_ITEMS);
	}
	
	return true;
}

void AppWindow::UpdateMenu()
{
	// Clear
	while(GetMenuItemCount(contextMenu) > SKIP_MENU_ITEMS)
		DeleteMenu(contextMenu, SKIP_MENU_ITEMS, MF_BYPOSITION);

	// Get filtered windows
	windowFilter.Execute();

	// Add items
	int i = SKIP_MENU_ITEMS;
	for (auto it = windowFilter.items.begin(); it != windowFilter.items.end(); ++it)
	{
		//AppendMenu(contextMenu, MF_STRING, i++, ((*it).className + (*it).title).c_str());
		AppendMenu(contextMenu, MF_STRING, i++, (*it).title.c_str());
	}

	// Add blank item if no windows were added
	if (i == SKIP_MENU_ITEMS) AppendMenu(contextMenu, MF_STRING|MF_GRAYED, 0, 
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