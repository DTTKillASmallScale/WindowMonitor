#include "stdafx.h"
#include "CWindowCreateStruct.h"
#include "WindowHelper.h"

void CWindowCreateStruct::Create(CWindow & window)
{
	CREATESTRUCT cs;
	SecureZeroMemory(&cs, sizeof(CREATESTRUCT));
	Configure(cs);
	cs.hInstance = WindowHelper::GetCurrentModuleHandle();
	cs.lpCreateParams = &window;
	::CreateWindowEx(cs.dwExStyle, cs.lpszClass, cs.lpszName, cs.style, cs.x, cs.y, cs.cx, cs.cy, cs.hwndParent, cs.hMenu, cs.hInstance, cs.lpCreateParams);
}