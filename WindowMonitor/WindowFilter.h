#pragma once

class WindowFilterItem
{
public:
	HWND hwnd;
	std::wstring title;
	
	WindowFilterItem(HWND const & hwnd, std::wstring const & title) : 
		hwnd(hwnd),
		title(title)
	{
	}

	bool operator<(WindowFilterItem const & rhs) const
	{
		return hwnd < rhs.hwnd;
	}
};

class WindowFilter
{
public:
	std::vector<WindowFilterItem> items;

	void Execute();
	bool AddWindow(HWND const & hwnd);

private:
	static BOOL CALLBACK WindowFilter::FilterWindows(_In_ HWND hwnd, _In_ LPARAM lParam);
};

