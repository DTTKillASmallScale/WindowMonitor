#pragma once

namespace WindowHelper
{
	void GetResourceString(HINSTANCE const & hInstance, UINT const & nID, std::wstring & str);
	void GetResourceString(HINSTANCE const & hInstance, UINT const & nID, std::string & str);
	HICON GetIcon(HINSTANCE const & hInstance, UINT const & nID, bool big = false);
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
	void GetPathToExecutable(std::wstring & filePath);
	void FillRegion(HDC const & hdc, HBRUSH const & brush, int const & x1, int const & y1, int const & x2, int const & y2);
}
