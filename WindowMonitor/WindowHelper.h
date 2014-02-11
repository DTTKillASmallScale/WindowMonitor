#pragma once
#include <windows.h>
#include <string>
#include <vector>

// Acknowledgements:
// This code adapted from Win32++ cstring.h
// Adam Szulc for initial CString code

namespace WindowHelper
{
	inline std::basic_string<TCHAR> LoadString(HINSTANCE hInstance, UINT nID)
	{
		int nSize = 64;
		int nTChars = nSize;
		std::vector<TCHAR> vString;
		TCHAR* pTCharArray = 0;

		while (nSize - 1 <= nTChars)
		{
			nSize = nSize * 4;
			vString.assign(nSize+1, _T('\0'));
			pTCharArray = &vString[0];
			nTChars = ::LoadString(hInstance, nID, pTCharArray, nSize);
		}

		std::basic_string<TCHAR> result;
		if (nTChars > 0) result.assign(pTCharArray);
		return result;
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
		SetWindowText(hWnd, WindowHelper::LoadString(hInstance, nID).c_str());
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
