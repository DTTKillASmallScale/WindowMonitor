#pragma once

class WindowFilterItem
{
public:
	HWND hwnd;
	std::wstring title;
	std::wstring className;
	
	WindowFilterItem(HWND const & hwnd, std::wstring const & title, std::wstring const & className) : 
		hwnd(hwnd),
		title(title),
		className(className)
	{
	}

	bool operator<(WindowFilterItem const & rhs) const
	{
		return hwnd < rhs.hwnd;
	}
};

struct WindowFilterNamedChildItem
{
	std::wstring className;
	bool wasFound;
};

class WindowFilter
{
public:
	std::vector<WindowFilterItem> items;
	void Execute();

private:
	std::vector<HWND> windows;
	bool IsFilteredByClassName(std::wstring const & className);
	bool OwnsWindowWithClassName(HWND const & ownerHwnd, std::wstring const & ownedClassName);
	static BOOL CALLBACK WindowFilter::AddWindowToList(_In_ HWND hwnd, _In_ LPARAM lParam);
};

