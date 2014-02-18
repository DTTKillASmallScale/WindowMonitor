#pragma once

namespace WindowHelper
{
	void GetResourceString(HINSTANCE const & hInstance, UINT const & nID, std::wstring & str);
	void GetResourceString(HINSTANCE const & hInstance, UINT const & nID, std::string & str);
	void SetIcon(HWND hWnd, HINSTANCE hInstance, int nIcon, bool big = false);
	void SetTitle(HWND hWnd, HINSTANCE hInstance, UINT nID);
	HFONT CreateFont(HWND hWnd, LPCWSTR const & faceName, double const & points, int const & weight,
		bool const & italic, bool const & underline, bool const & strikeout);
	void GetEditText(HWND const & hWnd, std::wstring & text);
	void GetClassNameText(HWND const & hWnd, std::wstring & text);
	void GetListboxItemText(HWND const & hWnd, int const & index, std::wstring & text);
	void GetMonitorRect(HWND const & hWnd, RECT & rect);
	void DisplayExceptionMessage(unsigned int const & titleId, unsigned int const & textId, std::exception const & e);
	HINSTANCE GetCurrentModuleHandle();
}
