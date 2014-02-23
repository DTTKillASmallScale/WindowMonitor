#pragma once

class CWindowWndClass
{
public:
	CWindowWndClass(std::wstring className) : registered(false), className(className) { }
	void Register(WNDPROC wndProc);
	void Unregister();

protected:
	bool registered;
	std::wstring className;
	virtual void Configure(WNDCLASSEX & wcex) = 0;
};
