#include "stdafx.h"
#include "WindowFilter.h"

void WindowFilter::Execute()
{
	// Clear set
	items.clear();

	// Enum windows
	EnumWindows(WindowFilter::FilterWindows, (LPARAM)this);
}

bool WindowFilter::AddWindow(HWND const & hwnd)
{
	const int bufferSize = 256;
	TCHAR buffer[bufferSize];
	std::wstring string;

	// Exit if window not visible
	if (!IsWindowVisible(hwnd)) return false;

	// Exit if minimized
	if (IsIconic(hwnd)) return false;

	// Exit if window class matches blacklist
	GetClassName(hwnd, buffer, bufferSize);
	string.assign(buffer);
	if (string.compare(L"Progman") == 0) return false;
	if (string.compare(L"Shell_TrayWnd") == 0) return false;
	if (string.compare(L"Button") == 0) return false;
	if (string.compare(L"DwmWindowMonitorApp") == 0) return false;

	// Get window text
	GetWindowText(hwnd, buffer, bufferSize);
	string.assign(buffer);

	// Check string
	if (string.empty()) return false;

	// Create item
	WindowFilterItem item(hwnd, string);

	// Insert at start
	items.insert(items.begin(), item);

	// Done
	return true;
}

BOOL CALLBACK WindowFilter::FilterWindows(_In_ HWND hwnd, _In_ LPARAM lParam)
{
	WindowFilter* windowFilter = (WindowFilter*)lParam;
	windowFilter->AddWindow(hwnd);
	return TRUE;
}
