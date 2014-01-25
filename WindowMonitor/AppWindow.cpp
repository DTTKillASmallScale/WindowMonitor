#include "stdafx.h"
#include "AppWindow.h"
#include "Resource.h"
#include "WindowHelper.h"
#include <sstream>

const int AppWindow::MaxMenuTextLength = 32;
const int AppWindow::MenuItemBreakPoint = 24;
const int AppWindow::CursorArrow = 32512;
const int AppWindow::CursorMove = 32646;
const int AppWindow::CursorScale = 32642;

AppWindow::AppWindow() : 
	CWindow(),
	chromeWidth(0),
	chromeHeight(0),
	scale(1.0)
{
	lastPos.x = lastPos.y = 0;
}

void AppWindow::PreCreate(CREATESTRUCT& cs)
{
	accelerators = LoadAccelerators(cs.hInstance, MAKEINTRESOURCE(IDW_MAIN));
	cs.lpszClass = _T("DwmWindowMonitorApp");
	cs.style = WS_VISIBLE | WS_POPUPWINDOW | WS_SIZEBOX;
}

void AppWindow::OnInitialUpdate()
{
	// Set title and icons
	WindowHelper::SetTitle(windowHandle, instance, IDS_TITLE);
	WindowHelper::SetIcon(windowHandle, instance, IDW_MAIN);
	WindowHelper::SetIcon(windowHandle, instance, IDW_MAIN, true);

	// Select source window
	SelectSource(0);

	// Set thumbnail to source
	adjustableThumbnail.SetThumbnail(windowHandle, sourceWindow);

	// Set source selection
	GetClientRect(sourceWindow, &selectionRect);

	// Resize window
	SetWindowSize(1.0);
}

LRESULT AppWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SIZING:
		OnSizing(wParam, lParam);
		return TRUE;
	case WM_SIZE:
		ScaleThumbnail();
		break;
	case WM_MOUSEMOVE:
		if (OnMouseMove(wParam, lParam)) return 0;
		break;
	case WM_LBUTTONDBLCLK:
		ToggleBorder();
		return 0;
	case WM_DESTROY:
		OnDestroy();
		break;
	}

	// Use the default message handling for remaining messages
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void AppWindow::OnSizing(WPARAM const & wParam, LPARAM const & lParam)
{
	LPRECT newRect = (LPRECT)lParam;
	double width = static_cast<double>(newRect->right - newRect->left - chromeWidth);
	double height = static_cast<double>(newRect->bottom - newRect->top - chromeHeight);
	double aspect = static_cast<double>(selectionRect.right - selectionRect.left) / static_cast<double>(selectionRect.bottom - selectionRect.top);
	long newWidth = static_cast<long>(height * aspect);
	long newHeight = static_cast<long>(width * (1.0 / aspect));
	long newValue;

	switch (wParam)
	{
	case WMSZ_TOP:
	case WMSZ_BOTTOM:
		newRect->right = newRect->left + newWidth + chromeWidth;
		break;

	case WMSZ_LEFT:
	case WMSZ_RIGHT:
		newRect->bottom = newRect->top + newHeight + chromeHeight;
		break;

	case WMSZ_TOPRIGHT:
		newValue = newRect->left + newWidth + chromeWidth;
		if (newValue < newRect->right) newRect->top = newRect->bottom - newHeight - chromeHeight;
		else newRect->right = newValue;
		break;

	case WMSZ_BOTTOMRIGHT:
		newValue = newRect->left + newWidth + chromeWidth;
		if (newValue < newRect->right) newRect->bottom = newRect->top + newHeight + chromeHeight;
		else newRect->right = newValue;
		break;

	case WMSZ_TOPLEFT:
		newValue = newRect->right - newWidth - chromeWidth;
		if (newValue > newRect->left) newRect->top = newRect->bottom - newHeight - chromeHeight;
		else newRect->left = newValue;
		break;

	case WMSZ_BOTTOMLEFT:
		newValue = newRect->right - newWidth - chromeWidth;
		if (newValue > newRect->left) newRect->bottom = newRect->top + newHeight + chromeHeight;
		else newRect->left = newValue;
		break;
	}

	CalcScale();
}

bool AppWindow::OnMouseMove(WPARAM const & wParam, LPARAM const & lParam)
{
	POINTS pos = MAKEPOINTS(lParam);
	bool lmb = (wParam == MK_LBUTTON);
	bool shiftLmb = (wParam == (MK_LBUTTON | MK_SHIFT));
	bool ctrlLmb = (wParam == (MK_LBUTTON | MK_CONTROL));

	// Drag window
	if (lmb)
	{
		SendMessage(windowHandle, WM_SYSCOMMAND, SC_MOVE | 0x0002, NULL);
		SetWindowPos(windowHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		return true;
	}
	
	if (!(lastPos.x == 0 && lastPos.y == 0))
	{
		// Shift view
		if (shiftLmb)
		{
			selectionRect.left -= pos.x - lastPos.x;
			selectionRect.top -= pos.y - lastPos.y;
			SetWindowSize();
		}
		// Crop view
		else if (ctrlLmb)
		{
			selectionRect.right += pos.x - lastPos.x;
			selectionRect.bottom += pos.y - lastPos.y;
			SetWindowSize();
		}
	}

	// Store last mouse position
	if (shiftLmb || ctrlLmb)
	{
		lastPos.x = pos.x;
		lastPos.y = pos.y;
	}
	else
	{
		lastPos.x = lastPos.y = 0;
	}

	return true;
}

void AppWindow::OnDestroy()
{
	adjustableThumbnail.UnsetThumbnail();
	
	// Quit
	PostQuitMessage(0);
}

void AppWindow::ToggleBorder()
{
	// Get old style
	DWORD oldStyle = (DWORD)GetWindowLong(windowHandle, GWL_STYLE);

	// Calc new style
	LONG_PTR newStyle = WS_VISIBLE | WS_POPUPWINDOW;
	if ((oldStyle & WS_SIZEBOX) == false) newStyle = newStyle | WS_SIZEBOX;

	// Set new style
	SetWindowLongPtr(windowHandle, GWL_STYLE, newStyle);
	SetWindowSize();
}

void AppWindow::CalcScale()
{
	RECT baseRect{ 0, 0, selectionRect.right - selectionRect.left, selectionRect.bottom - selectionRect.top };
	DWORD dwStyle = (DWORD)GetWindowLong(windowHandle, GWL_STYLE);
	AdjustWindowRect(&baseRect, dwStyle, FALSE);

	RECT windowRect;
	GetWindowRect(windowHandle, &windowRect);

	scale = static_cast<double>(windowRect.right - windowRect.left) / static_cast<double>(baseRect.right - baseRect.left);
}

void AppWindow::SetWindowSize(double const & scale)
{
	this->scale = scale;
	SetWindowSize();
}

void AppWindow::SetWindowSize()
{
	long width = static_cast<long>(static_cast<double>(selectionRect.right - selectionRect.left) * scale);
	long height = static_cast<long>(static_cast<double>(selectionRect.bottom - selectionRect.top) * scale);

	// Calc window size
	RECT windowRect{ 0, 0, width, height };
	DWORD dwStyle = (DWORD)GetWindowLong(windowHandle, GWL_STYLE);
	AdjustWindowRect(&windowRect, dwStyle, FALSE);

	// Set window size
	SetWindowPos(windowHandle, HWND_TOPMOST, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_NOMOVE);

	// Get size of window chrome
	chromeWidth = (windowRect.right - windowRect.left) - (selectionRect.right - selectionRect.left);
	chromeHeight = (windowRect.bottom - windowRect.top) - (selectionRect.bottom - selectionRect.top);
}

void AppWindow::ScaleThumbnail()
{
	RECT sourceRect;
	GetClientRect(sourceWindow, &sourceRect);

	sourceRect.left = static_cast<long>(static_cast<double>(-selectionRect.left) * scale);
	sourceRect.top = static_cast<long>(static_cast<double>(-selectionRect.top) * scale);
	sourceRect.right = static_cast<long>(static_cast<double>(sourceRect.right - selectionRect.left) * scale);
	sourceRect.bottom = static_cast<long>(static_cast<double>(sourceRect.bottom - selectionRect.top) * scale);

	adjustableThumbnail.SetSize(sourceRect);
}

void AppWindow::SelectSource(int const & index)
{
	// Get filtered windows
	windowFilter.Execute();

	// Get size
	std::size_t size = windowFilter.items.size();

	// Check size
	if (size < 1) return;

	// Set source index
	if (index < 0) sourceIndex = size - 1;
	else if (static_cast<size_t>(index) >= size) sourceIndex = 0;
	else sourceIndex = index;

	// Get source window handle
	sourceWindow = windowFilter.items.at(sourceIndex).hwnd;
}