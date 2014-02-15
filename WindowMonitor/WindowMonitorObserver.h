#pragma once

#define WindowMonitorEvent(DO) \
	DO(Nothing) \
	DO(Moved) \
	DO(Cropped) \
	DO(Scaled) \
	DO(ScaledToWindow) \
	DO(ScaledToMonitor) \
	DO(DimensionsReset) \
	DO(SourceSelected) \
	DO(PresetSelected) \
	DO(PresetDeselected) \
	DO(PresetSaved) \
	DO(PresetDeleted) \
	DO(PresetsUpdated)

#define MAKE_ENUM(VAR) VAR,
enum class WindowMonitorEvent
{
	WindowMonitorEvent(MAKE_ENUM)
};

#ifdef _DEBUG
#define MAKE_STRINGS(VAR) #VAR,
const char* const WindowMonitorEventNames[] = {
	WindowMonitorEvent(MAKE_STRINGS)
};
#endif

class WindowMonitorObserver
{
public:
	virtual void OnWindowMonitorEvent(WindowMonitorEvent const & event) = 0;
};