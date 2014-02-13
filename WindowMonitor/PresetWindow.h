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
	void ViewSettingUpdated(ViewSettingObserverSource const & eventSource, void * data);

protected:
	virtual void PreCreate(CREATESTRUCT & cs, WNDCLASSEX & wcex);

private:
	void OnCreate();
	void OnDestroy();
	bool OnCommand(WPARAM const & wParam, LPARAM const & lParam);
	void OnSetFocus();

	void SavePreset();
	void DeletePreset();
	void SelectPreset();

	void UpdatePresetList();
	void UpdateDimensions();

	PresetManager * presetManager;
	ViewSetting * currentViewSetting;

	int previousListboxSelection;
	HFONT defaultFont;
	HWND presetListbox, removeButton,
		titleLabel, titleText, 
		sizeLabel, posxText, posyText, widthText, heightText,
		saveButton;
};

