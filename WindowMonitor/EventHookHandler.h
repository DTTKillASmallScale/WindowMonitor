#pragma once

class EventHookHandler
{
public:
	virtual void OnEventHookTriggered(DWORD const & event, HWND const & hwnd, LONG const & obj, LONG const & child) = 0;
};
