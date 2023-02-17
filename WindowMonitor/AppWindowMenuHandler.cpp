#include "stdafx.h"
#include "AppWindowMenuHandler.h"
#include "Resource.h"
#include "AppWindow.h"
#include "WindowMonitor.h"
#include "WindowHelper.h"

const int AppWindowMenuHandler::MaxMenuTextLength = 32;
const int AppWindowMenuHandler::MenuItemBreakPoint = 24;

AppWindowMenuHandler::AppWindowMenuHandler(AppWindow * const appWindow, WindowMonitor * const windowMonitor):
appWindow(appWindow),
windowMonitor(windowMonitor),
entireMenu(NULL),
contextMenu(NULL),
presetMenu(NULL),
zoomMenu(NULL),
baseMenuItemCount(0)
{
	// Create menu
	entireMenu = LoadMenu(WindowHelper::GetCurrentModuleHandle(), MAKEINTRESOURCE(IDR_CTXMENU));
	contextMenu = GetSubMenu(entireMenu, 0);
	presetMenu = GetSubMenu(contextMenu, 0);
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
}

AppWindowMenuHandler::~AppWindowMenuHandler()
{
	windowMonitor->UnregisterObserver(this);
	DestroyMenu(entireMenu);
}

bool AppWindowMenuHandler::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT & result)
{
	switch (message)
	{
		case WM_CONTEXTMENU:
			OnContextMenu(wParam, lParam);
			return true;
		case WM_MENUCOMMAND:
			HandleMenuCmd(wParam, lParam);
			return true;
	}

	return false;
}

void AppWindowMenuHandler::OnContextMenu(WPARAM const & wParam, LPARAM const & lParam)
{
	// Get screen coords
	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);

	// Check if menu opened by keyboard shortcut
	if (x == -1 && y == -1)
	{
		RECT rect;
		GetWindowRect(appWindow->GetWindowHandle(), &rect);
		x = rect.left + (rect.right - rect.left) / 2;
		y = rect.top + (rect.bottom - rect.top) / 2;
	}

	// Update menu
	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;

	mii.fState = MFS_ENABLED | (appWindow->IsBorderVisible() ? MFS_CHECKED : 0);
	SetMenuItemInfo(contextMenu, ID_MENU_TOGGLEBORDER, FALSE, &mii);

	mii.fState = MFS_ENABLED | (appWindow->IsFullscreen() ? MFS_CHECKED : 0);
	SetMenuItemInfo(contextMenu, ID_MENU_FULLSCREEN, FALSE, &mii);

	// Update sources
	windowMonitor->UpdateSources();

	// Update presets
	windowMonitor->UpdatePresets();

	// Show menu
	TrackPopupMenu(contextMenu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, appWindow->GetWindowHandle(), NULL);
}

void AppWindowMenuHandler::RefreshSourceMenu()
{
	int count = GetMenuItemCount(contextMenu);
	size_t selectedHash = windowMonitor->GetSourceHash();

	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE | MIIM_ID;

	for (int i = baseMenuItemCount; i < count; i++)
	{
		GetMenuItemInfo(contextMenu, i, true, &mii);
		mii.fState = MFS_ENABLED | (mii.wID == selectedHash ? MFS_CHECKED : 0);
		SetMenuItemInfo(contextMenu, i, true, &mii);
	}
}

void AppWindowMenuHandler::ReloadSourceMenu()
{
	// Clear
	while (GetMenuItemCount(contextMenu) > baseMenuItemCount)
		DeleteMenu(contextMenu, baseMenuItemCount, MF_BYPOSITION);

	// Add items
	std::wstring text;
	int identifier = baseMenuItemCount;
	windowMonitor->IterateSources([&](WindowFilterItem const & item, bool const & selected)
	{
		// Get title text
		text.assign(item.title.substr(0, MaxMenuTextLength));

		// Add ellipsis if truncated
		if (item.title.length() > MaxMenuTextLength) text.append(L"...");

		// Create menu item
		bool breakMenu = identifier % MenuItemBreakPoint == 0;
		MENUITEMINFO mii;
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_DATA | MIIM_ID;
		mii.fType = MFT_STRING | (breakMenu ? MF_MENUBARBREAK : 0);
		mii.fState = MFS_ENABLED | (selected ? MFS_CHECKED : 0);
		mii.dwTypeData = const_cast<LPWSTR>(text.c_str());
		mii.cch = static_cast<unsigned int>(text.length());
		mii.wID = static_cast<unsigned int>(item.hash);
		InsertMenuItem(contextMenu, identifier, true, &mii);

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

void AppWindowMenuHandler::RefreshPresetMenu()
{
	int count = GetMenuItemCount(presetMenu);
	std::wstring selectedPreset = windowMonitor->GetSelectedPresetName();

	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);

	for (int i = 2; i < count; i++)
	{
		// Get string size
		mii.fMask = MIIM_STRING;
		mii.dwTypeData = NULL;
		GetMenuItemInfo(presetMenu, i, true, &mii);

		// Get string
		std::vector<wchar_t> buffer(mii.cch + 1, '\0');
		mii.dwTypeData = &buffer[0];
		mii.cch++;
		GetMenuItemInfo(presetMenu, i, true, &mii);

		// Set state
		mii.fMask = MIIM_STATE;
		mii.fState = MFS_ENABLED | (wcscmp(&buffer[0], selectedPreset.c_str()) == 0 ? MFS_CHECKED : 0);
		SetMenuItemInfo(presetMenu, i, true, &mii);
	}
}

void AppWindowMenuHandler::ReloadPresetMenu()
{
	// Clear
	while (GetMenuItemCount(presetMenu) > 2)
		DeleteMenu(presetMenu, 2, MF_BYPOSITION);

	// Add items
	std::wstring text;
	int identifier = 2;
	windowMonitor->IteratePresets([&](std::wstring const & name, bool const & selected)
	{
		// Create menu item
		bool breakMenu = identifier % MenuItemBreakPoint == 0;
		AppendMenu(presetMenu, MF_STRING | (breakMenu ? MF_MENUBARBREAK : 0) | (selected ? MF_CHECKED : 0), identifier, name.c_str());

		// Next item
		++identifier;
	});

	// Add blank item if no windows were added
	if (identifier == 2) 
	{
		std::wstring text;
		WindowHelper::GetResourceString(WindowHelper::GetCurrentModuleHandle(), IDS_NOPRESETSFOUND, text);
		AppendMenuW(presetMenu, MF_STRING | MF_GRAYED, 0, text.c_str());
	}
}

void AppWindowMenuHandler::HandleMenuCmd(WPARAM const & wParam, LPARAM const & lParam)
{
	// Get menu info
	HMENU menu = reinterpret_cast<HMENU>(lParam);
	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_ID;
	GetMenuItemInfo(menu, static_cast<UINT>(wParam), TRUE, &mii);

	switch (mii.wID)
	{
	case ID_MENU_RESET:
		windowMonitor->ResetAndScaleToFitMonitor(appWindow->GetWindowHandle());
		return;
	case ID_MENU_TOGGLEBORDER: appWindow->ToggleBorder(); return;
	case ID_MENU_FULLSCREEN: appWindow->ToggleFullscreen(); return;
	case ID_MENU_EXIT: appWindow->Destroy(); return;
	case ID_ZOOM_25: windowMonitor->Scale(0.25); return;
	case ID_ZOOM_50: windowMonitor->Scale(0.5); return;
	case ID_ZOOM_75: windowMonitor->Scale(0.75); return;
	case ID_ZOOM_100: windowMonitor->Scale(1.0); return;
	case ID_ZOOM_125: windowMonitor->Scale(1.25); return;
	case ID_ZOOM_150: windowMonitor->Scale(1.5); return;
	case ID_ZOOM_175: windowMonitor->Scale(1.75); return;
	case ID_ZOOM_200: windowMonitor->Scale(2.0); return;
	case ID_MENU_MANAGEPRESETS: appWindow->ShowPresetWindow(); return;
	}

	int selection = static_cast<int>(wParam);

	// Select source
	if (menu == contextMenu && selection >= baseMenuItemCount)
	{
		windowMonitor->SelectSourceByHash(static_cast<size_t>(mii.wID));
		return;
	}

	// Select preset
	if (menu == presetMenu && selection >= 2)
	{
		// Get string size
		mii.fMask = MIIM_STRING;
		mii.dwTypeData = NULL;
		GetMenuItemInfo(presetMenu, selection, TRUE, &mii);

		// Get string
		std::vector<wchar_t> buffer(mii.cch + 1, '\0');
		mii.dwTypeData = &buffer[0];
		mii.cch++;
		GetMenuItemInfo(presetMenu, selection, TRUE, &mii);

		// Select preset
		windowMonitor->SelectPreset(&buffer[0]);
		return;
	}
}

void AppWindowMenuHandler::OnWindowMonitorEvent(WindowMonitorEvent const & event)
{
	if (event & WindowMonitorEvent::SourcesUpdated)
	{
		ReloadSourceMenu();
		RefreshPresetMenu();
		return;
	}
	else if (event & WindowMonitorEvent::SourceSelected)
	{
		RefreshSourceMenu();
		RefreshPresetMenu();
		return;
	}

	switch (event)
	{
	case WindowMonitorEvent::DimensionsReset:
	case WindowMonitorEvent::PresetSelected:
	case WindowMonitorEvent::PresetDeselected:
		RefreshPresetMenu();
		break;
	case WindowMonitorEvent::PresetSaved:
	case WindowMonitorEvent::PresetDeleted:
	case WindowMonitorEvent::PresetsUpdated:
		ReloadPresetMenu();
		break;
	}
}