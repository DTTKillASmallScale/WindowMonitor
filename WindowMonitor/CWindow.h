#pragma once
#include <windows.h>
#include <string>

class CWindow
{
public:
	CWindow();
	void Create();
	void Run();
	inline HWND GetWindowHandle() { return windowHandle; }

protected:
	HINSTANCE instance;
	HWND windowHandle;
	HACCEL accelerators;

	virtual void PreCreate(CREATESTRUCT & cs) { }
	virtual void OnInitialUpdate() { }
	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	ATOM RegisterWindowClass(CREATESTRUCT const & cs);
	bool MakeWindow(CREATESTRUCT const & cs);
	bool PreTranslateMessage(MSG msg);
	static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};