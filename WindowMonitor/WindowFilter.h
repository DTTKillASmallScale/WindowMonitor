#pragma once
#include "WindowFilterItem.h"

class WindowFilter
{
public:
	std::vector<WindowFilterItem> items;
	void Execute();

private:
	std::vector<HWND> windows;
	bool IsFilteredByClassName(std::wstring const & className);
	bool EmptyTitleAllowedByClassName(std::wstring const & className);
	bool OwnsWindowWithClassName(HWND const & ownerHwnd, std::wstring const & ownedClassName);
	static BOOL CALLBACK WindowFilter::AddWindowToList(_In_ HWND hwnd, _In_ LPARAM lParam);
};

