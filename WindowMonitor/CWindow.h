#pragma once

class CWindowClass;
class CWindowStruct;

class CWindow
{
public:
	CWindow();
	~CWindow();
	virtual void Create();
	void Destroy();
	inline HWND const & GetWindowHandle() { return windowHandle; }
	static void Run();

protected:
	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;

	template <class T> inline void SetWindowClass() { windowClass.reset(new T()); }
	template <class T> inline void SetWindowStruct() { windowStruct.reset(new T()); }
	inline void SetWindowHandle(HWND const & newWindowHandle) { windowHandle = newWindowHandle; }
	void SetAccelerators(int const & resourceId);
	
	static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	HWND windowHandle;
	HACCEL accelerators;
	std::shared_ptr<CWindowClass> windowClass;
	std::shared_ptr<CWindowStruct> windowStruct;
	static bool PreTranslateMessage(MSG msg);
};