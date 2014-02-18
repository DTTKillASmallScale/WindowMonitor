#pragma once

class CWindow
{
public:
	CWindow();
	virtual void Create();
	void Destroy();
	inline HWND const & GetWindowHandle() { return windowHandle; }
	static void Run();

protected:
	inline void SetWindowHandle(HWND const & newWindowHandle) { windowHandle = newWindowHandle; }
	void SetAccelerators(int const & resourceId);
	virtual void PreCreate(CREATESTRUCT & cs, WNDCLASSEX & wcex);
	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	HWND windowHandle;
	HACCEL accelerators;
	static bool PreTranslateMessage(MSG msg);
};