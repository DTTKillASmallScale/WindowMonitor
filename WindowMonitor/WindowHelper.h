#pragma once

namespace WindowHelper
{
	void GetResourceString(HINSTANCE const & hInstance, UINT const & nID, std::wstring & str);
	void GetResourceString(HINSTANCE const & hInstance, UINT const & nID, std::string & str);
	HICON GetLargeIcon(HINSTANCE const & hInstance, UINT const & nID);
	HICON GetSmallIcon(HINSTANCE const & hInstance, UINT const & nID);
	void SetIcon(HWND const & hWnd, HINSTANCE const & hInstance, int const & nIcon, bool const & big = false);
	void SetTitle(HWND const & hWnd, HINSTANCE const & hInstance, UINT const & nID);
	HFONT CreateFont(HWND const & hWnd, LPCWSTR const & faceName, double const & points, int const & weight,
		bool const & italic, bool const & underline, bool const & strikeout);
	void GetEditText(HWND const & hWnd, std::wstring & text);
	void GetClassNameText(HWND const & hWnd, std::wstring & text);
	void GetListboxItemText(HWND const & hWnd, int const & index, std::wstring & text);
	void GetMonitorRect(HWND const & hWnd, RECT & rect);
	void DisplayExceptionMessage(unsigned int const & titleId, unsigned int const & textId, std::exception const & e);
	HINSTANCE GetCurrentModuleHandle();
	inline bool IsKeyDown(int virtKey) { return static_cast<unsigned short>(GetKeyState(virtKey)) >> 15 == 1; }
}
