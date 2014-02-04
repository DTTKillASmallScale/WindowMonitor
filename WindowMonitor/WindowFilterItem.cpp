#include "stdafx.h"
#include "WindowFilterItem.h"

WindowFilterItem::WindowFilterItem(HWND const & hwnd, std::wstring const & title, std::wstring const & className) :
	hwnd(hwnd),
	title(title),
	className(className)
{
}

bool WindowFilterItem::operator<(WindowFilterItem const & rhs) const
{
	return hwnd < rhs.hwnd;
}