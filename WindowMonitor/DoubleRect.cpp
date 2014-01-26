#include "stdafx.h"
#include "DoubleRect.h"

DoubleRect::DoubleRect() :
bottom(0.0),
left(0.0),
right(0.0),
top(0.0)
{
}

void DoubleRect::SetFromClientRect(HWND const & hWnd)
{
	RECT tmp;
	if (GetClientRect(hWnd, &tmp))
	{
		bottom = static_cast<double>(tmp.bottom);
		left = static_cast<double>(tmp.left);
		right = static_cast<double>(tmp.right);
		top = static_cast<double>(tmp.top);
	}
}
