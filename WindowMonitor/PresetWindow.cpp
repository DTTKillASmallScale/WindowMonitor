#include "stdafx.h"
#include "resource.h"
#include "PresetWindow.h"
#include "WindowHelper.h"
#include "PresetManager.h"
#include "ViewSetting.h"

enum class PresetCommand
{
	SavePreset = 100,
	RemovePreset,
	ListboxSelect
};

PresetWindow::PresetWindow(PresetManager * const presetManager, ViewSetting * const currentViewSetting) :
	CWindow(),
	presetManager(presetManager),
	currentViewSetting(currentViewSetting),
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
		OnSetFocus();
		break;
	}

	// Use the default message handling for remaining messages
	return DefWindowProc(hWnd, message, wParam, lParam);
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

	// Load data
	UpdatePresetList();

	// Add view setting observer
	RegisterObserver(this);
}

void PresetWindow::OnDestroy()
{
	// Remove view setting observer
	UnregisterObserver(this);

	// Clean up resources
	DeleteObject(defaultFont);
	previousListboxSelection = -1;
}

bool PresetWindow::OnCommand(WPARAM const & wParam, LPARAM const & lParam)
{
	auto low = LOWORD(wParam);
	auto high = HIWORD(wParam);

	switch (static_cast<PresetCommand>(low))
	{
	case PresetCommand::SavePreset:
		SavePreset();
		return true;
	case PresetCommand::RemovePreset:
		DeletePreset();
		return true;
	case PresetCommand::ListboxSelect:
		if (high == LBN_SELCHANGE) SelectPreset();
		return true;
	default:
		return false;
	}
}

void PresetWindow::OnSetFocus()
{
	try
	{
		if (presetManager->LoadFromBinaryFile())
			UpdatePresetList();
	}
	catch (std::runtime_error e)
	{
		WindowHelper::DisplayExceptionMessage(IDS_PRESET_ERROR_TITLE, IDS_PRESET_ERROR_FILE, e);
		this->Destroy();
	}
}

void PresetWindow::SavePreset()
{
	// Get name from textbox
	std::wstring editText;
	WindowHelper::GetEditText(titleText, editText);

	// Check
	if (editText.length() < 1) return;

	// If listbox has a selection, rename it
	int index = static_cast<int>(SendMessage(presetListbox, LB_GETCURSEL, 0, 0));
	if (index != LB_ERR)
	{
		std::wstring listText;
		WindowHelper::GetListboxItemText(presetListbox, index, listText);
		presetManager->RenamePreset(listText, editText);
	}

	// Save preset
	presetManager->SavePreset(editText, *currentViewSetting);
	ViewSettingObserver::NotifyObservers(ViewSettingObserverSource::SavePreset);
}

void PresetWindow::DeletePreset()
{
	int index = static_cast<int>(SendMessage(presetListbox, LB_GETCURSEL, 0, 0));
	if (index != LB_ERR)
	{
		std::wstring listText;
		WindowHelper::GetListboxItemText(presetListbox, index, listText);
		presetManager->RemovePreset(listText);
		UpdatePresetList();
	}
}

void PresetWindow::SelectPreset()
{
	// Get selected index
	int index = static_cast<int>(SendMessage(presetListbox, LB_GETCURSEL, 0, 0));
	if (index == LB_ERR) return;
	
	// Deselect if selection hasn't changed
	if (previousListboxSelection == index)
	{
		SendMessage(presetListbox, LB_SETCURSEL, -1, 0);
		previousListboxSelection = -1;
		return;
	}
	
	// Store selection
	previousListboxSelection = index;

	// Get title of selected preset
	std::wstring listText;
	WindowHelper::GetListboxItemText(presetListbox, index, listText);

	// Select preset
	presetManager->GetPreset(listText, *currentViewSetting);
	ViewSettingObserver::NotifyObservers(ViewSettingObserverSource::SelectPresetFromManager);
}

void PresetWindow::UpdatePresetList()
{
	SendMessage(presetListbox, LB_RESETCONTENT, 0, 0);
	previousListboxSelection = -1;

	presetManager->IterateNames([&](std::wstring const & name)
	{
		SendMessage(presetListbox, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(name.c_str()));
	});
}

void PresetWindow::UpdateDimensions()
{
	std::wostringstream temp;
	temp << std::setw(4) << currentViewSetting->top;
	SetWindowText(posyText, temp.str().c_str());

	temp.str(L"");
	temp << std::setw(4) << currentViewSetting->left;
	SetWindowText(posxText, temp.str().c_str());

	temp.str(L"");
	temp << std::setw(4) << currentViewSetting->bottom;
	SetWindowText(heightText, temp.str().c_str());

	temp.str(L"");
	temp << std::setw(4) << currentViewSetting->right;
	SetWindowText(widthText, temp.str().c_str());
}

void PresetWindow::ViewSettingUpdated(ViewSettingObserverSource const & eventSource, void * data)
{
	switch (eventSource)
	{
	case ViewSettingObserverSource::SelectPresetFromMenu:
	{
		// Get title of selected item from data
		std::wstring * text = reinterpret_cast<std::wstring *>(data);

		// Select listbox item
		int index = static_cast<int>(SendMessage(presetListbox, LB_FINDSTRINGEXACT, 0, reinterpret_cast<LPARAM>(text->c_str())));
		if (index != LB_ERR)
		{
			SendMessage(presetListbox, LB_SETCURSEL, index, 0);
			previousListboxSelection = index;
		}

		// Set text box
		SetWindowText(titleText, text->c_str());

		// Set dimension display
		UpdateDimensions();
		break;
	}
	case ViewSettingObserverSource::SelectPresetFromManager:
	{
		// Move this window back to top
		SetWindowPos(windowHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		// Get title of selected item from listbox
		std::wstring listText;
		WindowHelper::GetListboxItemText(presetListbox, previousListboxSelection, listText);

		// Set text box
		SetWindowText(titleText, listText.c_str());

		// Set dimension display
		UpdateDimensions();
		break;
	}
	case ViewSettingObserverSource::SavePreset:
	{
		// Get title
		std::wstring editText;
		WindowHelper::GetEditText(titleText, editText);

		// Refresh listbox
		UpdatePresetList();

		// Reselect item
		SendMessage(presetListbox, LB_SELECTSTRING, -1, reinterpret_cast<LPARAM>(reinterpret_cast<wchar_t const *>(editText.c_str())));
		previousListboxSelection = static_cast<int>(SendMessage(presetListbox, LB_GETCURSEL, 0, 0));

		// Set dimension display
		UpdateDimensions();
		break;
	}
	case ViewSettingObserverSource::Shift:
	case ViewSettingObserverSource::Crop:
		// Set dimension display
		UpdateDimensions();
		break;
	case ViewSettingObserverSource::Reset:
	{
		SetWindowText(titleText, L"");
		SendMessage(presetListbox, LB_SETCURSEL, -1, 0);
		previousListboxSelection = -1;
		UpdateDimensions();
		break;
	}
	default:
		break;
	}
}
