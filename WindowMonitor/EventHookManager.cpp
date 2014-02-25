#include "stdafx.h"
#include "EventHookManager.h"
#include "EventHookHandler.h"

std::map<size_t, EventHookManager::EventHookParameters> EventHookManager::hooks;

EventHookManager::~EventHookManager()
{
	for (auto it = hooks.begin(); it != hooks.end(); ++it)
		if (it->second.winEventHook) UnhookWinEvent(it->second.winEventHook);

	hooks.clear();
}

EventHookManager & EventHookManager::GetInstance()
{
	static EventHookManager instance;
	return instance;
}

bool EventHookManager::AddHook(HWND const & targetWindow, UINT const & eventMin, UINT const & eventMax, EventHookHandler * const handler)
{
	// Check map for existing entry
	auto hash = std::hash<HWND>()(targetWindow) ^ std::hash<UINT>()(eventMin) ^ std::hash<UINT>()(eventMax);
	auto it = hooks.find(hash);
	if (it != hooks.end()) return false;

	// Create hook
	DWORD processId;
	DWORD threadId = GetWindowThreadProcessId(targetWindow, &processId);
	HWINEVENTHOOK winEventHook = SetWinEventHook(eventMin, eventMax, NULL, WinEventProc, processId, threadId, WINEVENT_OUTOFCONTEXT);
	if (winEventHook == 0) return false;

	// Add to map
	EventHookParameters params{ targetWindow, eventMin, eventMax, handler, winEventHook };
	auto result = hooks.insert(std::make_pair(hash, params));
	return result.second;
}

bool EventHookManager::RemoveHook(HWND const & targetWindow, UINT const & eventMin, UINT const & eventMax)
{
	// Check map for existing entry
	auto hash = std::hash<HWND>()(targetWindow) ^ std::hash<UINT>()(eventMin) ^ std::hash<UINT>()(eventMax);
	auto it = hooks.find(hash);
	if (it == hooks.end()) return false;

	// Erase
	if (it->second.winEventHook) UnhookWinEvent(it->second.winEventHook);
	auto erased = hooks.erase(hash);
	return erased == 1;
}

struct WinEventProcItem
{
	DWORD event;
	HWND hwnd;
	LONG obj;
	LONG child;
	EventHookHandler * handler;
};

void CALLBACK EventHookManager::WinEventProc(HWINEVENTHOOK winEventHook, DWORD event, HWND hwnd, LONG obj, LONG child, DWORD eventThread, DWORD eventTime)
{
	std::vector<WinEventProcItem> items;
	
	// Handlers can remove entries, so make a separate list to avoid iterator problems
	for (auto it = hooks.begin(); it != hooks.end(); ++it)
	{
		if (hwnd == it->second.targetWindow && event >= it->second.eventMin && event <= it->second.eventMax)
			items.push_back(WinEventProcItem{event, hwnd, obj, child, it->second.handler});
	}

	// Call handlers
	for (auto it = items.begin(); it != items.end(); ++it)
	{
		it->handler->OnEventHookTriggered(it->event, it->hwnd, it->obj, it->child);
	}
}
