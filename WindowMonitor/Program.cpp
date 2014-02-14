#include "stdafx.h"
#include "WindowFilter.h"
#include "PresetManager.h"
#include "WindowMonitor.h"
#include "PresetWindow.h"
#include "AppWindow.h"

// WindowMonitor displays a DWM thumbnail for a selected window
// TV Icon from http://www.iconarchive.com/show/media-icons-by-bevel-and-emboss/tv-icon.html

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	WindowFilter windowFilter;
	PresetManager presetManager;
	WindowMonitor windowMonitor(&windowFilter, &presetManager);

	// Create preset window
	PresetWindow presetWindow(&windowMonitor);

	// Create main window
	AppWindow appWindow(&windowMonitor, &presetWindow);
	appWindow.Create();

	// Run loop
	CWindow::Run();
	
	return 0;
}
