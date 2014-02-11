#include "stdafx.h"
#include "WindowFilter.h"
#include "PresetManager.h"
#include "ViewSetting.h"
#include "AppWindow.h"
#include "PresetWindow.h"

// WindowMonitor displays a DWM thumbnail for a selected window
// TV Icon from http://www.iconarchive.com/show/media-icons-by-bevel-and-emboss/tv-icon.html

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	WindowFilter windowFilter;
	PresetManager presetManager;
	ViewSetting currentViewSetting;

	// Create main window
	AppWindow appWindow(&windowFilter, &presetManager, &currentViewSetting);
	appWindow.Create();

	// Run loop
	CWindow::Run();
	
	return 0;
}
