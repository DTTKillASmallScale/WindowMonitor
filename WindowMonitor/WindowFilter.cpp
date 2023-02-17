#include "stdafx.h"
#include "WindowFilter.h"
#include "WindowHelper.h"
#include <fstream>

WindowFilter::WindowFilter() :
	blacklistLastWrite(0),
	lastChecksum(0)
{
}

std::size_t WindowFilter::ItemCount()
{
	return items.size();
}

WindowFilterItem WindowFilter::GetItem(std::size_t const & index)
{
	if (index < items.size()) return items[index];
	else return WindowFilterItem();
}

WindowFilterItem WindowFilter::GetItemByHash(std::size_t const & hash)
{
	int index = -1;
	
	IterateItems([&](WindowFilterItem const & item)
	{
		index++;
		if (item.hash == hash) return true;
		return false;
	});

	if (index > -1) return items[index];
	else return WindowFilterItem();
}

WindowFilterItem WindowFilter::GetNextItem(WindowFilterItem const & currentItem)
{
	std::size_t index = 0;

	IterateItems([&](WindowFilterItem const & item)
	{
		index++;
		if (item == currentItem) return true;
		return false;
	});

	if (index >= ItemCount()) index = 0;
	return GetItem(index);
}

WindowFilterItem WindowFilter::GetPreviousItem(WindowFilterItem const & currentItem)
{
	std::size_t index = 0;

	IterateItems([&](WindowFilterItem const & item)
	{
		if (item == currentItem) return true;
		index++;
		return false;
	});

	if (index == 0) index = ItemCount() - 1;
	else index--;
	return GetItem(index);
}

void WindowFilter::IterateItems(WindowFilterIterateAction action)
{
	for (auto it = items.begin(); it != items.end(); ++it)
	{
		if (action(*it) == true) break;
	}
}

bool WindowFilter::Refresh()
{
	// Load blacklist
	LoadBlacklist();
	
	// Enum windows
	windows.clear();
	EnumWindows(WindowFilter::AddWindowToList, LPARAM(&windows));

	// Filter windows
	HWND hwnd, parent;
	std::wstring className, title;
	RECT clientRect;
	size_t checksum(1);

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
		GetClientRect(hwnd, &clientRect);
		if (clientRect.bottom - clientRect.top < 1 || clientRect.right - clientRect.left < 1)
			continue;

		// Get class name
		WindowHelper::GetClassNameText(hwnd, className);

		// Filter by window class
		if (IsFilteredByClassName(className)) continue;
	
		// Filter windows sidebar backgrounds
		if (className.compare(L"BasicWindow") == 0 && OwnsWindowWithClassName(hwnd, L"SideBar_HTMLHostWindow"))
			continue;

		// Get title
		WindowHelper::GetEditText(hwnd, title);

		// Check blacklist
		if (IsFilteredByBlacklist(className, title))
			continue;
	
		// Re-title windows sidebar inner windows
		if (className.compare(L"SideBar_HTMLHostWindow") == 0)
		{
			parent = GetParent(hwnd);
			if (parent != NULL) WindowHelper::GetEditText(parent, title);
		}

		// Filter untitled windows
		if (title.empty()) 
		{
			if (EmptyTitleAllowedByClassName(className)) title.assign(className);
			else continue;
		}

		// Insert item at start
		auto item = WindowFilterItem(hwnd, title, className);
		items.insert(items.begin(), item);

		// Update checksum
		static const std::hash<std::wstring> strHash;
		checksum ^= item.hash;
		checksum ^= strHash(title);
	}

	bool updated = (checksum != lastChecksum);
	lastChecksum = checksum;

	std::sort(items.begin(), items.end(), [](WindowFilterItem const & x, WindowFilterItem const & y){ return x.title < y.title; });
	return updated;
}

bool WindowFilter::IsFilteredByClassName(std::wstring const & className)
{
	if (className.compare(L"Progman") == 0) return true;
	if (className.compare(L"Shell_TrayWnd") == 0) return true;
	if (className.compare(L"Button") == 0) return true;
	if (className.compare(L"DV2ControlHost") == 0) return true;
	if (className.compare(L"DwmWindowMonitorApp") == 0) return true;
	if (className.compare(L"DwmWindowMonitorPresets") == 0) return true;
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

bool WindowFilter::IsFilteredByBlacklist(std::wstring const & className, std::wstring const & title)
{
	for (auto iter = blacklist.begin(); iter != blacklist.end(); ++iter)
	{
		if (iter->IsFiltered(title, className)) return true;
	}
	
	return false;
}

void WindowFilter::LoadBlacklist()
{
	// Create blacklist path
	std::wstring filename;
	WindowHelper::GetPathToExecutable(filename);
	filename.append(L"blacklist.txt");

	// Get last write time
	WIN32_FIND_DATA fileData;
	HANDLE fileHandle = FindFirstFile(filename.c_str(), &fileData);
	if (fileHandle == INVALID_HANDLE_VALUE) return;
	ULARGE_INTEGER currentTime;
	currentTime.LowPart = fileData.ftLastWriteTime.dwLowDateTime;
	currentTime.HighPart = fileData.ftLastWriteTime.dwHighDateTime;

	// If last recorded write time is less, read file
	if (blacklistLastWrite < currentTime.QuadPart)
	{
		// Open file and reset list
		std::wifstream in(filename);
		if (in.is_open()) blacklist.clear();
		else return;

		// Read file
		while (in.eof() == false)
		{
			std::wstring line, title, className;
			std::getline(in, line);

			std::wstringstream linestream(line);
			std::getline(linestream, title, L'\t');
			std::getline(linestream, className);

			if (title.length() > 0 || className.length() > 0)
			{
				blacklist.push_back(WindowFilterBlacklistItem(title, className));
			}
		}

		in.close();
		blacklistLastWrite = currentTime.QuadPart;
	}
}

BOOL CALLBACK WindowFilter::AddWindowToList(_In_ HWND hwnd, _In_ LPARAM lParam)
{
	std::vector<HWND> * windows = reinterpret_cast<std::vector<HWND>*>(lParam);
	windows->push_back(hwnd);
	return TRUE;
}
