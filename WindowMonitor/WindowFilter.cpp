#include "stdafx.h"
#include "WindowFilter.h"

std::size_t WindowFilter::ItemCount()
{
	return items.size();
}

HWND WindowFilter::GetWindowHandle(std::size_t const & index)
{
	return items.at(index).hwnd;
}

void WindowFilter::IterateItems(std::function<void(WindowFilterItem const &)> step)
{
	for (auto it = items.begin(); it != items.end(); ++it)
	{
		step(*it);
	}
}

void WindowFilter::Refresh()
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

		// Check client area size
		RECT clientRect;
		GetClientRect(hwnd, &clientRect);
		if (clientRect.bottom - clientRect.top < 1 || clientRect.right - clientRect.left < 1)
			continue;

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
		if (title.empty()) 
		{
			if (EmptyTitleAllowedByClassName(className)) title.assign(className);
			else continue;
		}

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

bool WindowFilter::EmptyTitleAllowedByClassName(std::wstring const & className)
{
	if (className.compare(L"Chrome_WidgetWin_1") == 0) return true;
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
	std::vector<HWND> * windows = reinterpret_cast<std::vector<HWND>*>(lParam);
	windows->push_back(hwnd);
	return TRUE;
}
