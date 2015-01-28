#include "stdafx.h"
#include "Resource.h"
#include "AppWindow.h"
#include "WindowMonitor.h"
#include "WindowHelper.h"
#include "EventHookManager.h"

const COLORREF AppWindow::BackgroundColour = RGB(255, 255, 255);

AppWindow::AppWindow(WindowMonitor * const windowMonitor, PresetWindow * const presetWindow) :
	cursorHandler(this, windowMonitor),
	acceleratorhandler(this, windowMonitor),
	menuHandler(this, windowMonitor),
	windowMonitor(windowMonitor),
	presetWindow(presetWindow),
	adjustableThumbnail(),
	chromeWidth(0),
	chromeHeight(0),
	wasSizing(false),
	borderVisible(true),
	fullScreen(false),
	hookedSource(NULL),
	drawBackground(false),
	tmpPreset()
{
	lastPos.x = lastPos.y = 0;
	SetAccelerators(IDW_MAIN);
}

void AppWindow::Create()
{
	if (GetWindowHandle() != NULL) return;

	static const auto configureWindowClass = [&](WNDCLASSEX & wcex)
	{
		wcex.lpszClassName = L"DwmWindowMonitorApp";
		wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_OWNDC;
		wcex.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(32512));
		wcex.hbrBackground = CreateSolidBrush(AppWindow::BackgroundColour);
		wcex.hIcon = WindowHelper::GetIcon(WindowHelper::GetCurrentModuleHandle(), IDW_MAIN);
		wcex.hIconSm = WindowHelper::GetIcon(WindowHelper::GetCurrentModuleHandle(), IDW_MAIN, false);
	};

	static const auto configureWindowStruct = [&](CREATESTRUCT & cs)
	{
		cs.lpszClass = L"DwmWindowMonitorApp";
		cs.style = WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX;
		cs.dwExStyle = NULL;
		cs.hwndParent = NULL;
		cs.x = 0;
		cs.y = 0;
		cs.cx = 640;
		cs.cy = 480;
		cs.hMenu = NULL;
	};

	Window::Create(configureWindowClass, configureWindowStruct);
}

void AppWindow::OnCreate()
{
	// Set window options
	WindowHelper::SetTitle(GetWindowHandle(), WindowHelper::GetCurrentModuleHandle(), IDS_TITLE);
	SetLayeredWindowAttributes(GetWindowHandle(), AppWindow::BackgroundColour, 0, LWA_COLORKEY);
	CenterWindow();

	// Add observer
	windowMonitor->RegisterObserver(this);

	// Select first source
	windowMonitor->UpdateSources();
	if (windowMonitor->GetSourceCount() > 0) windowMonitor->SelectFirstSource();
	else drawBackground = true;
}

void AppWindow::OnDestroy()
{
	// Remove view setting observer
	windowMonitor->UnregisterObserver(this);

	// Close preset window
	presetWindow->Destroy();

	// Close program
	PostQuitMessage(0);
}

LRESULT AppWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	// Handle accelerators
	if (acceleratorhandler.WndProc(hWnd, message, wParam, lParam, result))
		return result;

	// Handle cursor
	if (cursorHandler.WndProc(hWnd, message, wParam, lParam, result))
		return result;

	// Handle menu
	if (menuHandler.WndProc(hWnd, message, wParam, lParam, result))
		return result;

	switch (message)
	{
	case WM_RBUTTONUP:
		if (wParam == MK_LBUTTON) return 0;
		break;
	case WM_MOUSEMOVE:
		if (OnMouseMove(wParam, lParam)) return 0;
		break;
	case WM_GETMINMAXINFO:
	{
		DefWindowProc(hWnd, message, wParam, lParam);
		LPMINMAXINFO info = reinterpret_cast<LPMINMAXINFO>(lParam);
		info->ptMinTrackSize.x = 0;
		info->ptMinTrackSize.y = 0;
		break;
	}
	case WM_SIZING:
		wasSizing = true;
		if (OnSizing(wParam, lParam)) return TRUE;
		break;
	case WM_SIZE:
		if (wasSizing) { wasSizing = false; windowMonitor->ScaleToFitWindow(GetWindowHandle()); }
		break;
	case WM_EXITSIZEMOVE:
		wasSizing = false;
		break;
	case WM_LBUTTONDBLCLK:
		ToggleBorder();
		return 0;
	case WM_SYSCOMMAND:
		// Suppress alt-key menu activation
		if (wParam == SC_KEYMENU && (lParam >> 16) <= 0) return 0;
		break;
	case WM_PAINT:
		DrawBackground();
		return 0;
	case WM_CREATE:
		OnCreate();
		return 0;
	case WM_DESTROY:
		OnDestroy();
		return 0;
	}

	// Use the default message handling for remaining messages
	return DefWindowProc(hWnd, message, wParam, lParam);
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
		SendMessage(GetWindowHandle(), WM_SYSCOMMAND, SC_MOVE | 0x0002, NULL);
		return true;
	}

	if (!(lastPos.x == 0 && lastPos.y == 0))
	{
		long x = pos.x - lastPos.x;
		long y = pos.y - lastPos.y;

		// Shift view
		if (shiftLmb)
		{
			windowMonitor->Shift(x, y);
		}
		// Crop view
		else if (ctrlLmb)
		{
			windowMonitor->Crop(x, y);
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

bool AppWindow::OnSizing(WPARAM const & wParam, LPARAM const & lParam)
{
	LPRECT newRect = (LPRECT)lParam;
	double width = static_cast<double>(newRect->right - newRect->left - chromeWidth);
	double height = static_cast<double>(newRect->bottom - newRect->top - chromeHeight);
	double aspect = windowMonitor->GetAspect();
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

	// Set scale
	windowMonitor->ScaleToFitWindow(GetWindowHandle());
	return true;
}

void AppWindow::OnWindowMonitorEvent(WindowMonitorEvent const & event)
{
	if (event & WindowMonitorEvent::SourceSelected)
	{
		// If we have an existing source, remove hook
		if (hookedSource != NULL) EventHookManager::GetInstance().RemoveHook(hookedSource, EVENT_OBJECT_DESTROY, EVENT_OBJECT_DESTROY);

		// Store current source, and add hook if not NULL
		hookedSource = windowMonitor->GetSourceWindow();
		if (hookedSource != NULL) EventHookManager::GetInstance().AddHook(hookedSource, EVENT_OBJECT_DESTROY, EVENT_OBJECT_DESTROY, this);

		// Update window
		if (adjustableThumbnail.Register(GetWindowHandle(), windowMonitor->GetSourceWindow()))
		{
			if (tmpPreset.right - tmpPreset.left > 0)
			{
				windowMonitor->SetDimensions(tmpPreset);
				adjustableThumbnail.Scale(windowMonitor->GetScaledRect());
			}
			else
			{
				RECT sourceRect;
				GetClientRect(windowMonitor->GetSourceWindow(), &sourceRect);
				sourceRect.right -= sourceRect.left;
				sourceRect.bottom -= sourceRect.top;
				sourceRect.left = 0;
				sourceRect.top = 0;
				adjustableThumbnail.Scale(sourceRect);
			}
		}

		// Resize
		if (tmpPreset.right - tmpPreset.left <= 0) windowMonitor->ResetAndScaleToFitMonitor(GetWindowHandle());
		tmpPreset.bottom = tmpPreset.left = tmpPreset.right = tmpPreset.top = 0;
		drawBackground = false;
		InvalidateRect(GetWindowHandle(), NULL, TRUE);
		return;
	}

	switch (event)
	{
	case WindowMonitorEvent::Moved:
		adjustableThumbnail.Scale(windowMonitor->GetScaledRect());
		break;
	case WindowMonitorEvent::Cropped:
		UpdateWindow();
		break;
	case WindowMonitorEvent::PresetSelected:
	case WindowMonitorEvent::Scaled:
		adjustableThumbnail.Scale(windowMonitor->GetScaledRect());
		UpdateWindow();
		break;
	case WindowMonitorEvent::DimensionsReset:
	{
		// Toggle border if set
		DWORD style = static_cast<DWORD>(GetWindowLong(GetWindowHandle(), GWL_STYLE));
		if ((style & WS_THICKFRAME) == 0) ToggleBorder();

		// Clear fullscreen flag
		fullScreen = false;

		// Continue onto next case
	}
	case WindowMonitorEvent::ScaledToMonitor:
		adjustableThumbnail.Scale(windowMonitor->GetScaledRect());
		UpdateWindow();
		CenterWindow();
		break;
	case WindowMonitorEvent::ScaledToWindow:
		adjustableThumbnail.Scale(windowMonitor->GetScaledRect());
		break;
	default:
		break;
	}
}

void AppWindow::OnEventHookTriggered(DWORD const & event, HWND const & hwnd, LONG const & obj, LONG const & child)
{
	if (obj == OBJID_WINDOW && child == INDEXID_CONTAINER)
	{
		tmpPreset.CopyFrom(windowMonitor->GetDimensions());
		drawBackground = true;
		InvalidateRect(GetWindowHandle(), NULL, TRUE);
	}
}

void AppWindow::ToggleBorder()
{
	// Toggle window border
	borderVisible = !borderVisible;

	// Set new style
	DWORD style = static_cast<DWORD>(GetWindowLong(GetWindowHandle(), GWL_STYLE));
	style ^= WS_THICKFRAME;
	style ^= WS_BORDER;
	style ^= WS_DLGFRAME;
	SetWindowLongPtr(GetWindowHandle(), GWL_STYLE, style);

	// Update window
	UpdateWindow();
}

void AppWindow::ToggleFullscreen()
{
	fullScreen = !fullScreen;
	if ((fullScreen && borderVisible) || (!fullScreen && !borderVisible)) ToggleBorder();
	windowMonitor->ScaleToFitMonitor(GetWindowHandle(), fullScreen);
}

void AppWindow::ToggleClickThrough()
{
	// Toggle window transparency
	DWORD style = static_cast<DWORD>(GetWindowLong(GetWindowHandle(), GWL_EXSTYLE));
	style ^= WS_EX_TRANSPARENT;
	style ^= WS_EX_LAYERED;

	// Set new style
	SetWindowLongPtr(GetWindowHandle(), GWL_EXSTYLE, style);
}

void AppWindow::ShowPresetWindow()
{
	HWND hwnd = presetWindow->GetWindowHandle();
	if (hwnd != NULL) SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	else presetWindow->Create();
}

void AppWindow::UpdateWindow()
{
	long width = static_cast<long>(windowMonitor->GetScaledWidth());
	long height = static_cast<long>(windowMonitor->GetScaledHeight());

	// Calc window dimensions
	RECT windowRect{ 0, 0, width, height };
	DWORD dwStyle = static_cast<DWORD>(GetWindowLong(GetWindowHandle(), GWL_STYLE));
	AdjustWindowRect(&windowRect, dwStyle, FALSE);

	// Set window size
	SetWindowPos(GetWindowHandle(), NULL, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

	// Get size of window chrome
	chromeWidth = (windowRect.right - windowRect.left) - width;
	chromeHeight = (windowRect.bottom - windowRect.top) - height;
}

void AppWindow::DrawBackground()
{
	// Set up
	PAINTSTRUCT ps;
	HWND hwnd = GetWindowHandle();
	HDC hdc = BeginPaint(hwnd, &ps);

	if (drawBackground)
	{
		HBRUSH brush;
		RECT rect;
		GetClientRect(hwnd, &rect);

		// Calc size
		int barWidth = static_cast<int>(ceil(static_cast<double>(rect.right) / 7.0));
		int barHeight = static_cast<int>(ceil(static_cast<double>(rect.bottom) * 0.8));

		// Draw black
		brush = CreateSolidBrush(RGB(0, 0, 0));
		WindowHelper::FillRegion(hdc, brush, barWidth, barHeight, barWidth * 2, rect.bottom);
		WindowHelper::FillRegion(hdc, brush, barWidth * 3, barHeight, barWidth * 4, rect.bottom);
		WindowHelper::FillRegion(hdc, brush, barWidth * 5, barHeight, barWidth * 6, rect.bottom);
		DeleteObject(brush);

		// Draw blue
		brush = CreateSolidBrush(RGB(0, 0, 255));
		WindowHelper::FillRegion(hdc, brush, 0, barHeight, barWidth, rect.bottom);
		WindowHelper::FillRegion(hdc, brush, barWidth * 6, 0, barWidth * 7, barHeight);
		DeleteObject(brush);

		// Draw cyan
		brush = CreateSolidBrush(RGB(0, 255, 255));
		WindowHelper::FillRegion(hdc, brush, barWidth * 2, 0, barWidth * 3, barHeight);
		WindowHelper::FillRegion(hdc, brush, barWidth * 4, barHeight, barWidth * 5, rect.bottom);
		DeleteObject(brush);

		// Draw magenta
		brush = CreateSolidBrush(RGB(255, 0, 255));
		WindowHelper::FillRegion(hdc, brush, barWidth * 4, 0, barWidth * 5, barHeight);
		WindowHelper::FillRegion(hdc, brush, barWidth * 2, barHeight, barWidth * 3, rect.bottom);
		DeleteObject(brush);

		// Draw yellow
		brush = CreateSolidBrush(RGB(255, 255, 0));
		WindowHelper::FillRegion(hdc, brush, barWidth, 0, barWidth * 2, barHeight);
		DeleteObject(brush);

		// Draw green
		brush = CreateSolidBrush(RGB(0, 255, 0));
		WindowHelper::FillRegion(hdc, brush, barWidth * 3, 0, barWidth * 4, barHeight);
		DeleteObject(brush);

		// Draw red
		brush = CreateSolidBrush(RGB(255, 0, 0));
		WindowHelper::FillRegion(hdc, brush, barWidth * 5, 0, barWidth * 6, barHeight);
		DeleteObject(brush);
	}

	// Finish
	EndPaint(hwnd, &ps);
}

void AppWindow::CenterWindow()
{
	RECT windowRect, monitorRect;
	GetWindowRect(GetWindowHandle(), &windowRect);
	WindowHelper::GetMonitorRect(GetWindowHandle(), monitorRect);
	int x = (monitorRect.right + monitorRect.left - windowRect.right + windowRect.left) / 2;
	int y = (monitorRect.bottom + monitorRect.top - windowRect.bottom + windowRect.top) / 2;
	SetWindowPos(GetWindowHandle(), HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
}