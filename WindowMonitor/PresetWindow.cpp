#include "stdafx.h"
#include "resource.h"
#include "PresetWindow.h"
#include "WindowMonitor.h"
#include "WindowHelper.h"

PresetWindow::PresetWindow(WindowMonitor * const windowMonitor) :
	CWindow(),
	windowMonitor(windowMonitor),
	previousListboxSelection(-1)
{
}

void PresetWindow::PreCreate(CREATESTRUCT & cs, WNDCLASSEX & wcex)
{
	RECT monitorRect;
	WindowHelper::GetMonitorRect(windowHandle, monitorRect);

	cs.lpszClass = _T("DwmWindowMonitorPresets");
	cs.style = WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	cs.cx = 400;
	cs.cy = 220;
	cs.x = (monitorRect.right + monitorRect.left - cs.cx) / 2;
	cs.y = (monitorRect.bottom + monitorRect.top - cs.cy) / 2;
	wcex.hbrBackground = CreateSolidBrush(RGB(240, 240, 240));
}

void PresetWindow::OnCreate()
{
	// Set window options
	WindowHelper::SetTitle(windowHandle, instance, IDS_PRESETS);
	WindowHelper::SetIcon(windowHandle, instance, IDW_MAIN);
	WindowHelper::SetIcon(windowHandle, instance, IDW_MAIN, true);
	SetWindowPos(windowHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	// Create controls
	presetListbox = CreateWindowEx(WS_EX_CLIENTEDGE, L"Listbox", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT, 12, 12, 165, 121, windowHandle, reinterpret_cast<HMENU>(PresetCommand::ListboxSelect), instance, NULL);
	removeButton = CreateWindow(L"Button", L"Remove", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 12, 146, 75, 23, windowHandle, reinterpret_cast<HMENU>(PresetCommand::RemovePreset), instance, NULL);
	titleLabel = CreateWindow(L"Static", L"Name:", WS_CHILD | WS_VISIBLE | SS_LEFT, 192, 12, 30, 13, windowHandle, NULL, instance, NULL);
	titleText = CreateWindowEx(WS_EX_CLIENTEDGE, L"Edit", L"", WS_CHILD | WS_VISIBLE | SS_LEFT, 195, 28, 175, 20, windowHandle, NULL, instance, NULL);
	sizeLabel = CreateWindow(L"Static", L"Size:", WS_CHILD | WS_VISIBLE | SS_LEFT, 192, 60, 30, 13, windowHandle, NULL, instance, NULL);
	posxText = CreateWindowEx(WS_EX_CLIENTEDGE, L"Edit", L"", WS_CHILD | WS_VISIBLE | SS_LEFT | ES_READONLY, 202, 90, 50, 20, windowHandle, NULL, instance, NULL);
	posyText = CreateWindowEx(WS_EX_CLIENTEDGE, L"Edit", L"", WS_CHILD | WS_VISIBLE | SS_LEFT | ES_READONLY, 258, 78, 50, 20, windowHandle, NULL, instance, NULL);
	heightText = CreateWindowEx(WS_EX_CLIENTEDGE, L"Edit", L"", WS_CHILD | WS_VISIBLE | SS_LEFT | ES_READONLY, 258, 104, 50, 20, windowHandle, NULL, instance, NULL);
	widthText = CreateWindowEx(WS_EX_CLIENTEDGE, L"Edit", L"", WS_CHILD | WS_VISIBLE | SS_LEFT | ES_READONLY, 314, 90, 50, 20, windowHandle, NULL, instance, NULL);
	saveButton = CreateWindow(L"Button", L"Save", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 297, 146, 75, 23, windowHandle, reinterpret_cast<HMENU>(PresetCommand::SavePreset), instance, NULL);

	// Set fonts
	defaultFont = WindowHelper::CreateFont(windowHandle, L"Microsoft Sans Serif", 8.25, FW_NORMAL, false, false, false);
	SendMessage(presetListbox, WM_SETFONT, WPARAM(defaultFont), TRUE);
	SendMessage(removeButton, WM_SETFONT, WPARAM(defaultFont), TRUE);
	SendMessage(titleLabel, WM_SETFONT, WPARAM(defaultFont), TRUE);
	SendMessage(titleText, WM_SETFONT, WPARAM(defaultFont), TRUE);
	SendMessage(sizeLabel, WM_SETFONT, WPARAM(defaultFont), TRUE);
	SendMessage(posxText, WM_SETFONT, WPARAM(defaultFont), TRUE);
	SendMessage(posyText, WM_SETFONT, WPARAM(defaultFont), TRUE);
	SendMessage(widthText, WM_SETFONT, WPARAM(defaultFont), TRUE);
	SendMessage(heightText, WM_SETFONT, WPARAM(defaultFont), TRUE);
	SendMessage(saveButton, WM_SETFONT, WPARAM(defaultFont), TRUE);

	windowMonitor->RegisterObserver(this);
	UpdatePresetList();
	UpdateListSelection();
	UpdateDimensions();
}

void PresetWindow::OnDestroy()
{
	// Remove view setting observer
	windowMonitor->UnregisterObserver(this);

	// Clean up resources
	DeleteObject(defaultFont);
	previousListboxSelection = -1;
}

LRESULT PresetWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		OnCreate();
		return 0;
	case WM_DESTROY:
		OnDestroy();
		return 0;
	case WM_COMMAND:
		if (OnCommand(wParam, lParam)) return 0;
		break;
	case WM_SETFOCUS:
		windowMonitor->UpdatePresets();
		break;
	}

	// Use the default message handling for remaining messages
	return DefWindowProc(hWnd, message, wParam, lParam);
}

bool PresetWindow::OnCommand(WPARAM const & wParam, LPARAM const & lParam)
{
	auto low = LOWORD(wParam);
	auto high = HIWORD(wParam);

	switch (static_cast<PresetCommand>(low))
	{
	case PresetCommand::SavePreset:
	{
		std::wstring editText;
		WindowHelper::GetEditText(titleText, editText);
		windowMonitor->SavePreset(editText);
		return true;
	}
	case PresetCommand::RemovePreset:
	{
		int index = static_cast<int>(SendMessage(presetListbox, LB_GETCURSEL, 0, 0));
		if (index != LB_ERR) windowMonitor->DeleteSelectedPreset();
		return true;
	}
	case PresetCommand::ListboxSelect:
		if (high == LBN_SELCHANGE) OnListboxSelect();
		return true;
	default:
		return false;
	}
}

void PresetWindow::OnListboxSelect()
{
	// Get selected index
	int index = static_cast<int>(SendMessage(presetListbox, LB_GETCURSEL, 0, 0));
	if (index == LB_ERR) return;
	
	if (previousListboxSelection == index)
	{
		// Deselect if selection hasn't changed
		windowMonitor->UnselectPreset();
	}
	else
	{
		// Select preset
		std::wstring listText;
		WindowHelper::GetListboxItemText(presetListbox, index, listText);
		windowMonitor->SelectPreset(listText);
	}
}

void PresetWindow::UpdatePresetList()
{
	SendMessage(presetListbox, LB_RESETCONTENT, 0, 0);
	previousListboxSelection = -1;

	windowMonitor->IteratePresets([&](std::wstring const & name, bool const & selected)
	{
		SendMessage(presetListbox, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(name.c_str()));
	});
}

void PresetWindow::UpdateListSelection()
{
	std::wstring selectedName = windowMonitor->GetSelectedPresetName();
	int index = static_cast<int>(SendMessage(presetListbox, LB_FINDSTRINGEXACT, 0, reinterpret_cast<LPARAM>(selectedName.c_str())));
	SetWindowText(titleText, selectedName.c_str());
	SendMessage(presetListbox, LB_SETCURSEL, index, 0);
	previousListboxSelection = index;
}

void PresetWindow::UpdateDimensions()
{
	DoubleRect dimensions = windowMonitor->GetDimensions();

	std::wostringstream temp;
	temp << std::setw(4) << dimensions.top;
	SetWindowText(posyText, temp.str().c_str());

	temp.str(L"");
	temp << std::setw(4) << dimensions.left;
	SetWindowText(posxText, temp.str().c_str());

	temp.str(L"");
	temp << std::setw(4) << dimensions.bottom;
	SetWindowText(heightText, temp.str().c_str());

	temp.str(L"");
	temp << std::setw(4) << dimensions.right;
	SetWindowText(widthText, temp.str().c_str());
}

void PresetWindow::OnWindowMonitorEvent(WindowMonitorEvent const & event)
{
	switch (event)
	{
	case WindowMonitorEvent::DimensionsReset:
	case WindowMonitorEvent::PresetSelected:
	{
		UpdateListSelection();
		UpdateDimensions();
		break;
	}
	case WindowMonitorEvent::PresetsUpdated:
		UpdatePresetList();
		UpdateDimensions();
		break;
	case WindowMonitorEvent::PresetDeselected:
		UpdateListSelection();
		break;
	case WindowMonitorEvent::Moved:
	case WindowMonitorEvent::Cropped:
		UpdateDimensions();
		break;
	case WindowMonitorEvent::PresetSaved:
	case WindowMonitorEvent::PresetDeleted:
		UpdatePresetList();
		UpdateListSelection();
		break;
	default:
		break;
	}
}
