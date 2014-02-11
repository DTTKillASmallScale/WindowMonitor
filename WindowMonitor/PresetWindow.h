#pragma once
#include "CWindow.h"
#include "ViewSettingObserver.h"

class PresetManager;
class ViewSetting;

class PresetWindow : public CWindow, public ViewSettingObserver
{
public:
	PresetWindow(PresetManager * const presetManager, ViewSetting * const currentViewSetting);
	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void ViewSettingUpdated(ViewSettingObserverState const & state);

protected:
	virtual void PreCreate(CREATESTRUCT & cs, WNDCLASSEX & wcex);

private:
	void OnCreate();
	void OnDestroy();
	bool OnCommand(WPARAM const & wParam, LPARAM const & lParam);
	void OnSetFocus();
	void UpdatePresetList();
	void UpdateSelectedPreset();
	void UpdateDimensions();
	void DeletePreset();
	void SavePreset();
	std::wstring FormatDouble(double const & value);

	PresetManager * presetManager;
	ViewSetting * currentViewSetting;

	int previousListboxSelection;
	HFONT defaultFont;
	HWND presetListbox, removeButton,
		titleLabel, titleText, 
		sizeLabel, posxText, posyText, widthText, heightText,
		saveButton;
};

