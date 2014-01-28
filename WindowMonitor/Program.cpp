#include "stdafx.h"
#include "AppWindow.h"
#include "resource.h"

// WindowMonitor displays a DWM thumbnail for a selected window
// TV Icon from http://www.iconarchive.com/show/media-icons-by-bevel-and-emboss/tv-icon.html

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// Create main window
	AppWindow window;
	window.Create();
	
	// Run loop
	CWindow::Run();
	
	return 0;
}
