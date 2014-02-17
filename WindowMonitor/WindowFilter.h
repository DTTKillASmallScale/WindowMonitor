#pragma once
#include "WindowFilterItem.h"
#include "WindowFilterBlacklistItem.h"

typedef std::function<void(WindowFilterItem const & name)> WindowFilterIterateAction;

class WindowFilter
{
public:
	WindowFilter();
	std::size_t ItemCount();
	HWND GetWindowHandle(std::size_t const & index);
	void IterateItems(WindowFilterIterateAction action);
	void Refresh();

private:
	std::vector<HWND> windows;
	std::vector<WindowFilterItem> items;
	std::vector<WindowFilterBlacklistItem> blacklist;
	unsigned long long blacklistLastWrite;

	bool IsFilteredByClassName(std::wstring const & className);
	bool EmptyTitleAllowedByClassName(std::wstring const & className);
	bool OwnsWindowWithClassName(HWND const & ownerHwnd, std::wstring const & ownedClassName);
	bool IsFilteredByBlacklist(std::wstring const & className, std::wstring const & title);
	void LoadBlacklist();

	static BOOL CALLBACK WindowFilter::AddWindowToList(_In_ HWND hwnd, _In_ LPARAM lParam);
};

