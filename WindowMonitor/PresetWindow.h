#pragma once
#include "CWindow.h"
#include "WindowMonitorObserver.h"
#include "CustomEditControl.h"

class WindowMonitor;

enum class PresetCommand
{
	SavePreset = 100,
	RemovePreset,
	ListboxSelect
};

class PresetWindow : public CWindow, public WindowMonitorObserver
{
public:
	PresetWindow(WindowMonitor * const windowMonitor);
	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void OnWindowMonitorEvent(WindowMonitorEvent const & event);

protected:
	virtual void PreCreate(CREATESTRUCT & cs, WNDCLASSEX & wcex);

private:
	void OnCreate();
	void OnDestroy();
	bool OnCommand(WPARAM const & wParam, LPARAM const & lParam);
	void OnListboxSelect();

	void UpdatePresetList();
	void UpdateListSelection();
	void UpdateDimensions();

	WindowMonitor * windowMonitor;

	int previousListboxSelection;
	HFONT defaultFont;
	HWND presetListbox, removeButton,
		titleLabel, 
		sizeLabel, posxText, posyText, widthText, heightText,
		saveButton;
	CustomEditControl titleText;
};

