#pragma once
#include "WindowBase.h"
#include <functional>

class Control : public WindowBase
{
public:
	Control(WindowBase * parent);

protected:
	void Create(std::function<void(CREATESTRUCT &)> const & configureWindowStruct);
	WNDPROC controlWndProc;
	WindowBase * parent;
};

