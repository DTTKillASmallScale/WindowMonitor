#pragma once
#include "WindowBase.h"
#include <functional>

class Window : public WindowBase
{
public:
	Window();
	~Window();

protected:
	void Create(
		std::function<void(WNDCLASSEX &)> const & configureWindowClass,
		std::function<void(CREATESTRUCT &)> const & configureWindowStruct);

private:
	std::wstring customClassname;
	bool customClassRegistered;
};
