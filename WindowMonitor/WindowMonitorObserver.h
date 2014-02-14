#pragma once

enum class WindowMonitorEvent
{
	Moved = 1,
	Cropped,
	Scaled,
	ScaledToWindow,
	ScaledToMonitor,
	DimensionsReset,
	SourceSelected,
	PresetSelected,
	PresetDeselected,
	PresetSaved,
	PresetDeleted,
	PresetsUpdated
};

class WindowMonitorObserver
{
public:
	virtual void OnWindowMonitorEvent(WindowMonitorEvent const & event) = 0;
};