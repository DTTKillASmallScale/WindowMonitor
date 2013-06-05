#include "stdafx.h"
#include "WindowFilter.h"

void WindowFilter::Execute()
{
	// Enum windows
	windows.clear();
	EnumWindows(WindowFilter::AddWindowToList, LPARAM(&windows));

	// Filter windows
	HWND hwnd = NULL;
	const int bufferSize = 256;
	TCHAR buffer[bufferSize];
	std::wstring className;
	std::wstring title;

	// Clear set
	items.clear();

	// Iterate over windows
	for (auto iter = windows.begin(); iter != windows.end(); ++iter)
	{
		// Get hwnd
		hwnd = (*iter);
		
		// Exit if window not visible
		if (!IsWindowVisible(hwnd)) continue;

		// Exit if minimized
		if (IsIconic(hwnd)) continue;

		// Get class name
		GetClassName(hwnd, buffer, bufferSize);
		className.assign(buffer);

		// Filter by window class
		if (IsFilteredByClassName(className)) continue;
	
		// Filter windows sidebar backgrounds
		if (className.compare(L"BasicWindow") == 0 && OwnsWindowWithClassName(hwnd, L"SideBar_HTMLHostWindow"))
			continue;

		// Get title
		GetWindowText(hwnd, buffer, bufferSize);
		title.assign(buffer);
	
		// Re-title windows sidebar inner windows
		if (className.compare(L"SideBar_HTMLHostWindow") == 0)
		{
			HWND parent = GetParent(hwnd);
			if (parent != NULL)
			{
				GetWindowText(parent, buffer, bufferSize);
				title.assign(buffer);
			}
		}

		// Filter untitled windows
		if (title.empty()) continue;

		// Insert item at start
		items.insert(items.begin(), WindowFilterItem(hwnd, title, className));
	}
}

bool WindowFilter::IsFilteredByClassName(std::wstring const & className)
{
	if (className.compare(L"Progman") == 0) return true;
	if (className.compare(L"Shell_TrayWnd") == 0) return true;
	if (className.compare(L"Button") == 0) return true;
	if (className.compare(L"DwmWindowMonitorApp") == 0) return true;
	return false;
}

bool WindowFilter::OwnsWindowWithClassName(HWND const & ownerHwnd, std::wstring const & ownedClassName)
{
	const int bufferSize = 256;
	TCHAR buffer[bufferSize];
	std::wstring testClassName;

	for (auto iter = windows.begin(); iter != windows.end(); ++iter)
	{
		if (GetWindow((*iter), GW_OWNER) == ownerHwnd)
		{
			// Test class name
			GetClassName((*iter), buffer, bufferSize);
			testClassName.assign(buffer);
			if (testClassName.compare(ownedClassName) == 0) return true;
		}
	}
	
	return false;
}

BOOL CALLBACK WindowFilter::AddWindowToList(_In_ HWND hwnd, _In_ LPARAM lParam)
{
	std::vector<HWND>* windows = (std::vector<HWND>*)lParam;
	windows->push_back(hwnd);
	return TRUE;
}
