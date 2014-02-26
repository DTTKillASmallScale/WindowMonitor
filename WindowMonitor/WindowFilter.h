#pragma once
#include "WindowFilterItem.h"
#include "WindowFilterBlacklistItem.h"

typedef std::function<bool(WindowFilterItem const & item)> WindowFilterIterateAction;

class WindowFilter
{
public:
	WindowFilter();
	std::size_t ItemCount();
	WindowFilterItem GetItem(std::size_t const & index);
	WindowFilterItem GetItemByHash(std::size_t const & hash);
	WindowFilterItem GetNextItem(WindowFilterItem const & currentItem);
	WindowFilterItem GetPreviousItem(WindowFilterItem const & currentItem);
	void IterateItems(WindowFilterIterateAction action);
	bool Refresh();

private:
	std::vector<HWND> windows;
	std::vector<WindowFilterItem> items;
	std::vector<WindowFilterBlacklistItem> blacklist;
	unsigned long long blacklistLastWrite;
	size_t lastChecksum;

	bool IsFilteredByClassName(std::wstring const & className);
	bool EmptyTitleAllowedByClassName(std::wstring const & className);
	bool OwnsWindowWithClassName(HWND const & ownerHwnd, std::wstring const & ownedClassName);
	bool IsFilteredByBlacklist(std::wstring const & className, std::wstring const & title);
	void LoadBlacklist();

	static BOOL CALLBACK WindowFilter::AddWindowToList(_In_ HWND hwnd, _In_ LPARAM lParam);
};

