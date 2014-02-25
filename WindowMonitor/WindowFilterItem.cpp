#include "stdafx.h"
#include "WindowFilterItem.h"

const std::hash<HWND> WindowFilterItem::HwndHash;
const std::hash<std::wstring> WindowFilterItem::StringHash;

WindowFilterItem::WindowFilterItem(HWND const & hwnd, std::wstring const & title, std::wstring const & className) :
	hwnd(hwnd),
	title(title),
	className(className),
	hash(HwndHash(hwnd) ^ StringHash(className))
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