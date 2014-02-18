#include "stdafx.h"
#include "resource.h"
#include "AppWindow.h"
#include "WindowMonitor.h"
#include "WindowHelper.h"

LRESULT AppWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_LBUTTONDOWN:
	case WM_KEYDOWN:
		// Bit 30: The value is 1 if the key is down before the message is sent, or it is zero if the key is up.
		if ((lParam & 0x40000000) == 0) { SetContextualCursor(); return 0; }
		break;
	case WM_KEYUP:
		SetContextualCursor();
		return 0;
	case WM_LBUTTONUP:
		SetContextualCursor();
		if (wParam == MK_RBUTTON) { suppressContextMenu = true; return 0; }
		break;
	case WM_RBUTTONUP:
		if (wParam == MK_LBUTTON) return 0;
		break;
	case WM_SETCURSOR:
		if (OnSetCursor(wParam, lParam)) return TRUE;
		break;
	case WM_MOUSEMOVE:
		if (OnMouseMove(wParam, lParam)) return 0;
		break;
	case WM_SIZING:
		wasSizing = true;
		if (OnSizing(wParam, lParam)) return TRUE;
		break;
	case WM_SIZE:
		if (wasSizing) { wasSizing = false; windowMonitor->ScaleToFitWindow(GetWindowHandle()); }
		break;
	case WM_EXITSIZEMOVE:
		wasSizing = false;
		break;
	case WM_LBUTTONDBLCLK:
		ToggleBorder();
		return 0;
	case WM_COMMAND:
		if (OnAccelCommand(wParam, lParam)) return 0;
		break;
	case WM_CONTEXTMENU:
		OnContextMenu(wParam, lParam);
		break;
	case WM_MENUCOMMAND: {
		HMENU sourceMenu = reinterpret_cast<HMENU>(lParam);
		if (sourceMenu == contextMenu) OnOptionsMenuCmd(wParam);
		else if (sourceMenu == presetsMenu) OnPresetsMenuCmd(wParam);
		else if (sourceMenu == zoomMenu) OnZoomMenuCmd(wParam);
		break;
	}
	case WM_CREATE:
		OnCreate();
		return 0;
	case WM_DESTROY:
		OnDestroy();
		return 0;
	}

	// Use the default message handling for remaining messages
	return DefWindowProc(hWnd, message, wParam, lParam);
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
		SendMessage(GetWindowHandle(), WM_SYSCOMMAND, SC_MOVE | 0x0002, NULL);
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
			windowMonitor->Shift(x, y);
		}
		// Crop view
		else if (ctrlLmb)
		{
			windowMonitor->Crop(x, y);
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
	double aspect = windowMonitor->GetAspect();
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

	// Set scale
	windowMonitor->ScaleToFitWindow(GetWindowHandle());
	return true;
}

bool AppWindow::OnAccelCommand(WPARAM const & wParam, LPARAM const & lParam)
{
	if (HIWORD(wParam) == 1)
	{
		switch (LOWORD(wParam))
		{
		case ID_ACCEL_SWITCH:
		{
			windowMonitor->SelectNextSource();
			return true;
		}
		case ID_ACCEL_RSWITCH:
		{
			windowMonitor->SelectPreviousSource();
			return true;
		}
		case ID_ACCEL_RESET:
		{
			windowMonitor->ResetAndScaleToFitMonitor(GetWindowHandle());
			return true;
		}
		case ID_ACCEL_FULLSCREEN:
		{
			DWORD style = static_cast<DWORD>(GetWindowLong(GetWindowHandle(), GWL_STYLE));
			if ((style & WS_THICKFRAME) != 0) ToggleBorder();
			windowMonitor->ScaleToFitMonitor(GetWindowHandle(), true);
			return true;
		}
		case ID_ACCEL_CLICKTHROUGH:
			ToggleClickThrough();
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

	// Check if menu opened by keyboard shortcut
	if (x == -1 && y == -1)
	{
		RECT rect;
		GetWindowRect(GetWindowHandle(), &rect);
		x = rect.left + (rect.right - rect.left) / 2;
		y = rect.top + (rect.bottom - rect.top) / 2;
	}

	// Show menu
	UpdateSourceMenu();
	UpdatePresetMenu();
	TrackPopupMenu(contextMenu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, GetWindowHandle(), NULL);
}

void AppWindow::OnOptionsMenuCmd(WPARAM const & wParam)
{
	int selection = static_cast<unsigned int>(wParam);
	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_ID;
	GetMenuItemInfo(contextMenu, selection, TRUE, &mii);

	switch (mii.wID)
	{
	case ID_MENU_TOGGLEBORDER:
		ToggleBorder();
		break;
	case ID_MENU_RESET:
		windowMonitor->ResetAndScaleToFitMonitor(GetWindowHandle());
		break;
	case ID_MENU_EXIT:
		Destroy();
		break;
	default:
		if (selection >= baseMenuItemCount)
			windowMonitor->SelectSource(static_cast<size_t>(selection - baseMenuItemCount));
		break;

	}
}

void AppWindow::OnPresetsMenuCmd(WPARAM const & wParam)
{
	int selection = static_cast<unsigned int>(wParam);
	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_ID;
	GetMenuItemInfo(presetsMenu, selection, TRUE, &mii);

	switch (mii.wID)
	{
	case ID_MENU_MANAGEPRESETS:
	{
		HWND hwnd = presetWindow->GetWindowHandle();
		if (hwnd != NULL) SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		else presetWindow->Create();
		break;
	}
	default: {
		if (selection >= 2)
		{
			// Get string size
			mii.fMask = MIIM_STRING;
			mii.dwTypeData = NULL;
			GetMenuItemInfo(presetsMenu, selection, TRUE, &mii);

			// Get string
			std::vector<wchar_t> buffer(mii.cch + 1, '\0');
			mii.dwTypeData = &buffer[0];
			mii.cch++;
			GetMenuItemInfo(presetsMenu, selection, TRUE, &mii);

			// Select preset
			windowMonitor->SelectPreset(&buffer[0]);
		}
		break;
	}
	}
}

void AppWindow::OnZoomMenuCmd(WPARAM const & wParam)
{
	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_ID;
	GetMenuItemInfo(zoomMenu, static_cast<unsigned int>(wParam), TRUE, &mii);

	switch (mii.wID)
	{
	case ID_ZOOM_25: windowMonitor->Scale(0.25); break;
	case ID_ZOOM_50: windowMonitor->Scale(0.5); break;
	case ID_ZOOM_75: windowMonitor->Scale(0.75); break;
	case ID_ZOOM_100: windowMonitor->Scale(1.0); break;
	case ID_ZOOM_125: windowMonitor->Scale(1.25); break;
	case ID_ZOOM_150: windowMonitor->Scale(1.5); break;
	case ID_ZOOM_175: windowMonitor->Scale(1.75); break;
	case ID_ZOOM_200: windowMonitor->Scale(2.0); break;
	default: return;
	}
}
