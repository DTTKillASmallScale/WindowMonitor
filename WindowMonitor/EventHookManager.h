#pragma once

class EventHookHandler;

class EventHookManager
{
public:
	static EventHookManager & GetInstance();
	bool AddHook(HWND const & targetWindow, UINT const & eventMin, UINT const & eventMax, EventHookHandler * const handler);
	bool RemoveHook(HWND const & targetWindow, UINT const & eventMin, UINT const & eventMax);

private:
	EventHookManager() {};
	EventHookManager(EventHookManager const &) = delete;
	EventHookManager & operator= (EventHookManager const &) = delete;
	~EventHookManager();

	struct EventHookParameters
	{
		HWND targetWindow;
		UINT eventMin;
		UINT eventMax;
		EventHookHandler * handler;
		HWINEVENTHOOK winEventHook;
	};

	static std::map<size_t, EventHookParameters> hooks;
	static void CALLBACK WinEventProc(HWINEVENTHOOK winEventHook, DWORD event, HWND hwnd, LONG obj, LONG child, DWORD eventThread, DWORD eventTime);
};
