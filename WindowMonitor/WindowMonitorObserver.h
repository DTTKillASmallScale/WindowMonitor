#pragma once

enum class WindowMonitorEvent : unsigned int
{
	Nothing = 0,
	Moved =				1 << 0,
	Cropped =			1 << 1,
	Scaled =			1 << 2,
	ScaledToWindow =	1 << 3,
	ScaledToMonitor =	1 << 4,
	DimensionsReset =	1 << 5,
	SourcesUpdated =	1 << 6,
	SourceSelected =	1 << 7,
	PresetSelected =	1 << 8,
	PresetDeselected =	1 << 9,
	PresetSaved =		1 << 10,
	PresetDeleted =		1 << 11,
	PresetsUpdated =	1 << 12
};

inline bool operator&(WindowMonitorEvent const & lhs, WindowMonitorEvent const & rhs)
{
	return (static_cast<int>(lhs) & static_cast<int>(rhs)) != 0;
}

inline WindowMonitorEvent operator|(WindowMonitorEvent const & lhs, WindowMonitorEvent const & rhs)
{
	return static_cast<WindowMonitorEvent>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

#ifdef _DEBUG
inline void DebugWindowMonitorEventValue(WindowMonitorEvent const & event)
{
	std::wstringstream ss;
	ss << time(NULL) << L"\t";
	if (WindowMonitorEvent::Moved & event) ss << L"Moved ";
	if (WindowMonitorEvent::Cropped & event) ss << L"Cropped ";
	if (WindowMonitorEvent::Scaled & event) ss << L"Scaled ";
	if (WindowMonitorEvent::ScaledToWindow & event) ss << L"ScaledToWindow ";
	if (WindowMonitorEvent::ScaledToMonitor & event) ss << L"ScaledToMonitor ";
	if (WindowMonitorEvent::DimensionsReset & event) ss << L"DimensionsReset ";
	if (WindowMonitorEvent::SourcesUpdated & event) ss << L"SourcesUpdated ";
	if (WindowMonitorEvent::SourceSelected & event) ss << L"SourceSelected ";
	if (WindowMonitorEvent::PresetSelected & event) ss << L"PresetSelected ";
	if (WindowMonitorEvent::PresetDeselected & event) ss << L"PresetDeselected ";
	if (WindowMonitorEvent::PresetSaved & event) ss << L"PresetSaved ";
	if (WindowMonitorEvent::PresetDeleted & event) ss << L"PresetDeleted ";
	if (WindowMonitorEvent::PresetsUpdated & event) ss << L"PresetsUpdated ";
	ss << L"\n";
	OutputDebugStringW(ss.str().c_str());
}
#endif

class WindowMonitorObserver
{
public:
	virtual void OnWindowMonitorEvent(WindowMonitorEvent const & event) = 0;
};