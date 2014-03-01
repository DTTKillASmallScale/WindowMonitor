#include "stdafx.h"
#include "Application.h"
#include "WindowFilter.h"
#include "PresetManager.h"
#include "WindowMonitor.h"
#include "PresetWindow.h"
#include "AppWindow.h"

#ifdef _DEBUG
#include <vld.h>
#endif

// WindowMonitor displays a DWM thumbnail for a selected window
// TV Icon from http://www.iconarchive.com/show/media-icons-by-bevel-and-emboss/tv-icon.html

int APIENTRY WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
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
	Application::Run();
	
	return 0;
}
