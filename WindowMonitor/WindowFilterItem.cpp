#include "stdafx.h"
#include "WindowFilterItem.h"

WindowFilterItem::WindowFilterItem(HWND const & hwnd, std::wstring const & title, std::wstring const & className) :
	hwnd(hwnd),
	title(title),
	className(className),
	hash(MakeHash(hwnd, className))
{
}

bool WindowFilterItem::operator<(WindowFilterItem const & rhs) const
{
	return hwnd < rhs.hwnd;
}

bool WindowFilterItem::operator == (WindowFilterItem const & rhs) const
{
	return hash == rhs.hash;
}

size_t WindowFilterItem::MakeHash(HWND const & hwnd, std::wstring const & className)
{
	static const std::hash<HWND> hwndHash;
	static const std::hash<std::wstring> stringHash;
	return hwndHash(hwnd) ^ stringHash(className);
}