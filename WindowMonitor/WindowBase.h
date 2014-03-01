#pragma once

class WindowBase
{
public:
	WindowBase();

	virtual void Create();
	void Destroy();

	HWND const & GetWindowHandle();
	HACCEL const & GetAccelerators();

	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;
	LRESULT ApplicationWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	void SetWindowHandle(HWND const & newWindowHandle);
	void SetAccelerators(int const & resourceId);

private:
	HWND windowHandle;
	HACCEL accelerators;
};

