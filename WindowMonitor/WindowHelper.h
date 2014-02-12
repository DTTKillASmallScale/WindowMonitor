#pragma once
#include <windows.h>
#include <string>
#include <vector>

namespace WindowHelper
{
	inline void GetResourceString(HINSTANCE const & hInstance, UINT const & nID, std::wstring & str)
	{
		std::vector<wchar_t> buffer(256, '\0');
		LoadStringW(hInstance, nID, &buffer[0], 256);
		str.assign(&buffer[0]);
	}
	
	inline void GetResourceString(HINSTANCE const & hInstance, UINT const & nID, std::string & str)
	{
		std::vector<char> buffer(256, '\0');
		LoadStringA(hInstance, nID, &buffer[0], 256);
		str.assign(&buffer[0]);
	}

	inline void SetIcon(HWND hWnd, HINSTANCE hInstance, int nIcon, bool big = false)
	{
		HICON icon = (HICON) LoadImage(hInstance, 
			MAKEINTRESOURCE(nIcon), 
			IMAGE_ICON,
			GetSystemMetrics(big ? SM_CXICON : SM_CXSMICON), 
			GetSystemMetrics(big ? SM_CYICON : SM_CYSMICON), 
			0);

		if (icon) SendMessage(hWnd, WM_SETICON, WPARAM(big ? ICON_BIG : ICON_SMALL), LPARAM(icon));
	}

	inline void SetTitle(HWND hWnd, HINSTANCE hInstance, UINT nID)
	{
		std::wstring text;
		WindowHelper::GetResourceString(hInstance, nID, text);
		SetWindowTextW(hWnd, &text[0]);
	}

	inline HFONT CreateFont(HWND hWnd, LPCWSTR const & faceName, double const & points, int const & weight,
		bool const & italic, bool const & underline, bool const & strikeout)
	{
		HDC hdc = GetDC(hWnd);

		int fontHeight = -static_cast<int>((points * static_cast<double>(GetDeviceCaps(hdc, LOGPIXELSY))) / 72.0);

		return ::CreateFont(
			fontHeight,
			0, 0, 0,
			weight,
			italic ? TRUE : FALSE,
			underline ? TRUE : FALSE,
			strikeout ? TRUE : FALSE,
			ANSI_CHARSET, OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
			faceName);
	}

	inline void GetEditText(HWND const & hWnd, std::wstring & text)
	{
		// Get number of characters + null terminator
		int textLength = static_cast<int>(SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0)) + 1;

		// Create buffer
		std::vector<wchar_t> buffer(textLength, '\0');

		// Get winapi-friendly pointer to buffer
		wchar_t* bufferPointer = reinterpret_cast<wchar_t*>(&buffer[0]);

		// Get text from control
		SendMessage(hWnd, WM_GETTEXT, static_cast<WPARAM>(textLength), reinterpret_cast<LPARAM>(bufferPointer));

		// Set output string
		text.assign(&buffer[0]);
	}

	inline void GetListboxItemText(HWND const & hWnd, int const & index, std::wstring & text)
	{
		// Get number of characters for selected listbox item
		int textLength = static_cast<int>(SendMessage(hWnd, LB_GETTEXTLEN, static_cast<WPARAM>(index), 0));

		// Create buffer + null terminator
		std::vector<wchar_t> buffer(textLength + 1, '\0');

		// Get winapi-friendly pointer to buffer
		wchar_t* bufferPointer = reinterpret_cast<wchar_t*>(&buffer[0]);

		// Get text from control
		SendMessage(hWnd, LB_GETTEXT, static_cast<WPARAM>(index), reinterpret_cast<LPARAM>(bufferPointer));

		// Set output string
		text.assign(&buffer[0]);
	}

	inline void GetMonitorRect(HWND const & hWnd, RECT & rect)
	{
		HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO monitorInfo;
		monitorInfo.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(monitor, &monitorInfo);
		rect.bottom = monitorInfo.rcMonitor.bottom;
		rect.left = monitorInfo.rcMonitor.left;
		rect.right = monitorInfo.rcMonitor.right;
		rect.top = monitorInfo.rcMonitor.top;
	}
}
