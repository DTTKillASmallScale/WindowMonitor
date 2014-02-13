// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <Windowsx.h>
#include <tchar.h>

#include <stdlib.h>
#include <memory>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <functional>
#include <regex>

#include <shellapi.h>
#include <Dwmapi.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Prevent window from being on top in debug mode
#ifdef _DEBUG
#pragma warning(suppress: 4005)
#define HWND_TOPMOST NULL
#endif
