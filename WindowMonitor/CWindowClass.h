#pragma once

class CWindowClass
{
public:
	CWindowClass(std::wstring className) : registered(false), className(className) { }
	void Register(WNDPROC wndProc);
	void Unregister();

protected:
	bool registered;
	std::wstring className;
	virtual void Configure(WNDCLASSEX & wcex) = 0;
};

class CWindow;
class CWindowStruct
{
public:
	void Create(CWindow & window);

protected:
	virtual void Configure(CREATESTRUCT & cs) = 0;
};