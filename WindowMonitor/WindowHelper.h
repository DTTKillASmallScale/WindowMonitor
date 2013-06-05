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

	inline void SetIcon(HINSTANCE hInstance, HWND hWnd, int nIcon, bool big = false)
	{
		HICON icon = (HICON) LoadImage(hInstance, 
			MAKEINTRESOURCE(nIcon), 
			IMAGE_ICON,
			GetSystemMetrics(big ? SM_CXICON : SM_CXSMICON), 
			GetSystemMetrics(big ? SM_CYICON : SM_CYSMICON), 
			0);

		if (icon) SendMessage(hWnd, WM_SETICON, WPARAM(big ? ICON_BIG : ICON_SMALL), LPARAM(icon));
	}
}
