#pragma once

class WindowFilterItem
{
public:
	HWND hwnd;
	std::wstring title;
	std::wstring className;
	size_t hash;

	WindowFilterItem() : hwnd(NULL), hash(0) { }
	WindowFilterItem(HWND const & hwnd, std::wstring const & title, std::wstring const & className);
	bool operator<(WindowFilterItem const & rhs) const;
	bool operator==(WindowFilterItem const & rhs) const;

private:
	static size_t MakeHash(HWND const & hwnd, std::wstring const & className);
};