#pragma once

class WindowFilterItem
{
public:
	HWND hwnd;
	std::wstring title;
	std::wstring className;

	WindowFilterItem(HWND const & hwnd, std::wstring const & title, std::wstring const & className);
	bool operator<(WindowFilterItem const & rhs) const;
};