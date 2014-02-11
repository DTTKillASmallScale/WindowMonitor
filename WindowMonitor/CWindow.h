#pragma once

class CWindow
{
public:
	CWindow();
	void Create();
	void Destroy();
	inline HWND GetWindowHandle() { return windowHandle; }
	inline HINSTANCE GetInstance() { return instance; }
	static void Run();

protected:
	HINSTANCE instance;
	HWND windowHandle;
	HACCEL accelerators;

	virtual void PreCreate(CREATESTRUCT & cs, WNDCLASSEX & wcex);
	virtual void OnInitialUpdate();
	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	static bool PreTranslateMessage(MSG msg);
	static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};