#pragma once
#include "WindowFilterItem.h"

class WindowFilter
{
public:
	std::size_t ItemCount();
	HWND GetWindowHandle(std::size_t const & index);
	void IterateItems(std::function<void(WindowFilterItem const &)> step);
	void Refresh();

private:
	std::vector<HWND> windows;
	std::vector<WindowFilterItem> items;
	bool IsFilteredByClassName(std::wstring const & className);
	bool EmptyTitleAllowedByClassName(std::wstring const & className);
	bool OwnsWindowWithClassName(HWND const & ownerHwnd, std::wstring const & ownedClassName);
	static BOOL CALLBACK WindowFilter::AddWindowToList(_In_ HWND hwnd, _In_ LPARAM lParam);
};

