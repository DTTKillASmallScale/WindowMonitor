#include "stdafx.h"
#include "AppWindow.h"
#include "Resource.h"
#include "WindowHelper.h"
#include <sstream>

LRESULT AppWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_LBUTTONDOWN:
	case WM_KEYDOWN:
		if (OnKeyDown(wParam, lParam)) return 0;
		break;
	case WM_KEYUP:
		if (OnKeyUp(wParam, lParam)) return 0;
		break;
	case WM_LBUTTONUP:
		if (OnLeftButtonUp(wParam, lParam)) return 0;
		break;
	case WM_RBUTTONUP:
		if (OnRightButtonUp(wParam, lParam)) return 0;
		break;
	case WM_SETCURSOR:
		if (OnSetCursor(wParam, lParam)) return TRUE;
		break;
	case WM_MOUSEMOVE:
		if (OnMouseMove(wParam, lParam)) return 0;
		break;
	case WM_SIZING:
		if (OnSizing(wParam, lParam)) return TRUE;
		break;
	case WM_SIZE:
		if (OnSize(wParam, lParam)) return TRUE;
		break;
	case WM_LBUTTONDBLCLK:
		if (OnLeftDoubleClick(wParam, lParam)) return 0;
		break;
	case WM_COMMAND:
		if (OnAccelCommand(wParam, lParam)) return 0;
		break;
	case WM_CONTEXTMENU:
		OnContextMenu(wParam, lParam);
		break;
	case WM_MENUCOMMAND:
		OnMenuCommand(wParam, lParam);
		break;
	case WM_DESTROY:
		OnDestroy();
		break;
	}

	// Use the default message handling for remaining messages
	return DefWindowProc(hWnd, message, wParam, lParam);
}

bool AppWindow::OnKeyDown(WPARAM const & wParam, LPARAM const & lParam)
{
	// Bit 30: The value is 1 if the key is down before the message is sent, or it is zero if the key is up.
	bool notRepeated = (lParam & 0x40000000) == 0;

	if (notRepeated)
	{
		SetContextualCursor();
		return true;
	}

	return false;
}

bool AppWindow::OnKeyUp(WPARAM const & wParam, LPARAM const & lParam)
{
	SetContextualCursor();
	return true;
}

bool AppWindow::OnLeftButtonUp(WPARAM const & wParam, LPARAM const & lParam)
{
	SetContextualCursor();
	if (wParam == MK_RBUTTON) { suppressContextMenu = true; return true; }
	return false;
}

bool AppWindow::OnRightButtonUp(WPARAM const & wParam, LPARAM const & lParam)
{
	if (wParam == MK_LBUTTON) return true;
	return false;
}

bool AppWindow::OnSetCursor(WPARAM const & wParam, LPARAM const & lParam)
{
	if (LOWORD(lParam) == HTCLIENT && currentCursor != 0)
	{
		if (cursorSet == false)
		{
			cursorSet = true;
			SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(currentCursor)));
		}

		return true;
	}

	if (LOWORD(lParam) != HTCLIENT && cursorSet)
		cursorSet = false;

	return false;
}

bool AppWindow::OnMouseMove(WPARAM const & wParam, LPARAM const & lParam)
{
	POINTS pos = MAKEPOINTS(lParam);
	bool lmb = (wParam == MK_LBUTTON);
	bool shiftLmb = (wParam == (MK_LBUTTON | MK_SHIFT));
	bool ctrlLmb = (wParam == (MK_LBUTTON | MK_CONTROL));

	// Drag window
	if (lmb)
	{
		SendMessage(windowHandle, WM_SYSCOMMAND, SC_MOVE | 0x0002, NULL);
		SetContextualCursor();
		return true;
	}

	if (!(lastPos.x == 0 && lastPos.y == 0))
	{
		long x = pos.x - lastPos.x;
		long y = pos.y - lastPos.y;

		// Shift view
		if (shiftLmb)
		{
			selectionRect.left -= x / scale;
			selectionRect.top -= y / scale;
			selectionRect.right -= x / scale;
			selectionRect.bottom -= y / scale;
			UpdateThumbnail();
		}
		// Crop view
		else if (ctrlLmb)
		{
			selectionRect.right += x / scale;
			selectionRect.bottom += y / scale;
			UpdateWindow();
		}
	}

	// Store last mouse position
	if (shiftLmb || ctrlLmb)
	{
		lastPos.x = pos.x;
		lastPos.y = pos.y;
	}
	else
	{
		lastPos.x = lastPos.y = 0;
	}

	return true;
}

bool AppWindow::OnSizing(WPARAM const & wParam, LPARAM const & lParam)
{
	LPRECT newRect = (LPRECT)lParam;
	double width = static_cast<double>(newRect->right - newRect->left - chromeWidth);
	double height = static_cast<double>(newRect->bottom - newRect->top - chromeHeight);
	double aspect = static_cast<double>(selectionRect.right - selectionRect.left) / static_cast<double>(selectionRect.bottom - selectionRect.top);
	long newWidth = static_cast<long>(height * aspect);
	long newHeight = static_cast<long>(width * (1.0 / aspect));
	long newValue;

	switch (wParam)
	{
	case WMSZ_TOP:
	case WMSZ_BOTTOM:
		newRect->right = newRect->left + newWidth + chromeWidth;
		break;

	case WMSZ_LEFT:
	case WMSZ_RIGHT:
		newRect->bottom = newRect->top + newHeight + chromeHeight;
		break;

	case WMSZ_TOPRIGHT:
		newValue = newRect->left + newWidth + chromeWidth;
		if (newValue < newRect->right) newRect->top = newRect->bottom - newHeight - chromeHeight;
		else newRect->right = newValue;
		break;

	case WMSZ_BOTTOMRIGHT:
		newValue = newRect->left + newWidth + chromeWidth;
		if (newValue < newRect->right) newRect->bottom = newRect->top + newHeight + chromeHeight;
		else newRect->right = newValue;
		break;

	case WMSZ_TOPLEFT:
		newValue = newRect->right - newWidth - chromeWidth;
		if (newValue > newRect->left) newRect->top = newRect->bottom - newHeight - chromeHeight;
		else newRect->left = newValue;
		break;

	case WMSZ_BOTTOMLEFT:
		newValue = newRect->right - newWidth - chromeWidth;
		if (newValue > newRect->left) newRect->bottom = newRect->top + newHeight + chromeHeight;
		else newRect->left = newValue;
		break;
	}

	// Calculate scale
	RECT clientRect;
	GetClientRect(windowHandle, &clientRect);
	scale = static_cast<double>(clientRect.right - clientRect.left) / ceil(selectionRect.right - selectionRect.left);

	return true;
}

bool AppWindow::OnSize(WPARAM const & wParam, LPARAM const & lParam)
{
	UpdateThumbnail();
	return true;
}

bool AppWindow::OnLeftDoubleClick(WPARAM const & wParam, LPARAM const & lParam)
{
	ToggleBorder();
	return true;
}

bool AppWindow::OnAccelCommand(WPARAM const & wParam, LPARAM const & lParam)
{
	if (HIWORD(wParam) == 1)
	{
		// Tab and reset
		switch (LOWORD(wParam))
		{
		case ID_ACCEL_SWITCH:
			CycleForward();
			return true;
		case ID_ACCEL_RSWITCH:
			CycleBack();
			return true;
		case ID_ACCEL_REFRESH:
			Reset();
			return true;
		}
	}

	return false;
}

void AppWindow::OnContextMenu(WPARAM const & wParam, LPARAM const & lParam)
{
	if (suppressContextMenu)
	{
		suppressContextMenu = false;
		return;
	}

	// Get screen coords
	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);

	// TODO Check x and y for -1

	// Show menu
	UpdateMenu();
	TrackPopupMenu(contextMenu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, windowHandle, NULL);
}

void AppWindow::OnMenuCommand(WPARAM const & wParam, LPARAM const & lParam)
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
			Reset();
			break;
		case ID_MENU_TOGGLEBORDER:
			ToggleBorder();
			break;
		default:
			if (static_cast<int>(wParam) >= baseMenuItemCount)
				SelectSource(wParam - baseMenuItemCount);
			break;
		}
	}
	else if (sourceMenu == zoomMenu)
	{
		GetMenuItemInfo(zoomMenu, wParam, TRUE, &mii);

		switch (mii.wID)
		{
		case ID_ZOOM_25: scale = 0.25; break;
		case ID_ZOOM_50: scale = 0.5; break;
		case ID_ZOOM_75: scale = 0.75; break;
		case ID_ZOOM_100: scale = 1.0; break;
		case ID_ZOOM_125: scale = 1.25; break;
		case ID_ZOOM_150: scale = 1.5; break;
		case ID_ZOOM_175: scale = 1.75; break;
		case ID_ZOOM_200: scale = 2.0; break;
		default: return;
		}

		UpdateWindow();
	}
}
