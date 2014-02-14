#include "stdafx.h"
#include "resource.h"
#include "WindowMonitor.h"
#include "WindowHelper.h"

const double WindowMonitor::MaxMonitorFitRatio = 0.9;
const std::wregex WindowMonitor::WhitespacePattern(L"^\\s+|\\s+$");

WindowMonitor::WindowMonitor(WindowFilter * sources, PresetManager * presets) :
	sources(sources),
	presets(presets),
	dimensions(),
	scale(1.0),
	selectedSource(0),
	selectedPreset()
{
}

WindowMonitor::~WindowMonitor()
{
}

void WindowMonitor::SelectSource(std::size_t const & index)
{
	// Get filtered windows
	sources->Refresh();
	if (sources->ItemCount() < 1) return;

	// Set selected index
	selectedSource = index;

	// Set dimensions
	RECT tmp;
	if (GetClientRect(sources->GetWindowHandle(selectedSource), &tmp))
	{
		dimensions.bottom = static_cast<double>(tmp.bottom);
		dimensions.left = static_cast<double>(tmp.left);
		dimensions.right = static_cast<double>(tmp.right);
		dimensions.top = static_cast<double>(tmp.top);
	}

	// Send update event
	NotifyObservers(WindowMonitorEvent::SourceSelected);
}

void WindowMonitor::SelectNextSource()
{
	auto index = selectedSource + 1;
	if (index >= sources->ItemCount()) index = 0;
	SelectSource(index);
}

void WindowMonitor::SelectPreviousSource()
{
	if (selectedSource == 0) SelectSource(sources->ItemCount() - 1);
	else SelectSource(selectedSource - 1);
}

void WindowMonitor::SelectPreset(std::wstring const & name)
{
	bool success = presets->GetPreset(name, dimensions);
	if (success) 
	{
		selectedPreset.assign(name);
		NotifyObservers(WindowMonitorEvent::PresetSelected);
	}
}

void WindowMonitor::UnselectPreset()
{
	if (!selectedPreset.empty())
	{
		selectedPreset.clear();
		NotifyObservers(WindowMonitorEvent::PresetDeselected);
	}
}

void WindowMonitor::SavePreset(std::wstring const & name)
{
	// Check
	if (name.empty()) return;
	auto result = std::regex_replace(name, WindowMonitor::WhitespacePattern, L"");
	if (result != name) return;

	// Rename preset if different to name
	if (!selectedPreset.empty() && selectedPreset != name)
	{
		presets->RenamePreset(selectedPreset, name);
	}

	// Save preset
	presets->SavePreset(name, dimensions);
	selectedPreset.assign(name);
	NotifyObservers(WindowMonitorEvent::PresetSaved);
}

void WindowMonitor::DeleteSelectedPreset()
{
	if (!selectedPreset.empty())
	{
		presets->RemovePreset(selectedPreset);
		selectedPreset.clear();
		NotifyObservers(WindowMonitorEvent::PresetDeleted);
	}
}

void WindowMonitor::Shift(long const & x, long const & y)
{
	dimensions.left -= x / scale;
	dimensions.top -= y / scale;
	dimensions.right -= x / scale;
	dimensions.bottom -= y / scale;
	NotifyObservers(WindowMonitorEvent::Moved);
}

void WindowMonitor::Crop(long const & x, long const & y)
{
	dimensions.right += x / scale;
	dimensions.bottom += y / scale;
	NotifyObservers(WindowMonitorEvent::Cropped);
}

void WindowMonitor::ResetDimensions()
{
	RECT tmp;
	if (GetClientRect(sources->GetWindowHandle(selectedSource), &tmp))
	{
		dimensions.bottom = static_cast<double>(tmp.bottom);
		dimensions.left = static_cast<double>(tmp.left);
		dimensions.right = static_cast<double>(tmp.right);
		dimensions.top = static_cast<double>(tmp.top);
	}
	selectedPreset.clear();
	NotifyObservers(WindowMonitorEvent::DimensionsReset);
}

void WindowMonitor::Scale(double const & newScale)
{
	scale = newScale;
	NotifyObservers(WindowMonitorEvent::Scaled);
}

void WindowMonitor::ScaleToFitWindow(HWND const & hWnd)
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	scale = static_cast<double>(rect.right - rect.left) / ceil(dimensions.right - dimensions.left);
	NotifyObservers(WindowMonitorEvent::ScaledToWindow);
}

void WindowMonitor::ScaleToFitMonitor(HWND const & hWnd)
{
	// Get monitor rect for window
	RECT monitorRect;
	WindowHelper::GetMonitorRect(hWnd, monitorRect);

	// Get horizontal ratio
	double sourceMonitorRatio = (dimensions.right - dimensions.left) / static_cast<double>(monitorRect.right - monitorRect.left);

	// Use vertical ratio if larger
	sourceMonitorRatio = max(sourceMonitorRatio, (dimensions.bottom - dimensions.top) / static_cast<double>(monitorRect.bottom - monitorRect.top));

	// Set scale
	if (sourceMonitorRatio > MaxMonitorFitRatio) scale = MaxMonitorFitRatio / sourceMonitorRatio;
	else scale = 1.0;
	NotifyObservers(WindowMonitorEvent::ScaledToMonitor);
}

RECT WindowMonitor::GetScaledRect()
{
	HWND hwnd = sources->GetWindowHandle(selectedSource);
	RECT rect;
	GetClientRect(hwnd, &rect);
	rect.left = static_cast<long>(-dimensions.left * scale);
	rect.top = static_cast<long>(-dimensions.top * scale);
	rect.right = static_cast<long>((static_cast<double>(rect.right) - dimensions.left) * scale);
	rect.bottom = static_cast<long>((static_cast<double>(rect.bottom) - dimensions.top) * scale);
	return rect;
}

void WindowMonitor::IterateSources(std::function<void(std::wstring const & title, bool const & selected)> action)
{
	HWND hwnd = sources->GetWindowHandle(selectedSource);
	sources->Refresh();
	if (sources->ItemCount() < 1) return;
	sources->IterateItems([&](WindowFilterItem const & item)
	{
		action(item.title, (item.hwnd == hwnd));
	});
}

void WindowMonitor::IteratePresets(std::function<void(std::wstring const & name, bool const & selected)> action)
{
	UpdatePresets();
	presets->IterateNames([&](std::wstring const & name)
	{
		action(name, (name == selectedPreset));
	});
}

void WindowMonitor::UpdatePresets()
{
	try
	{
		if (presets->LoadFromBinaryFile())
			NotifyObservers(WindowMonitorEvent::PresetsUpdated);
	}
	catch (std::runtime_error e)
	{
		WindowHelper::DisplayExceptionMessage(IDS_PRESET_ERROR_TITLE, IDS_PRESET_ERROR_FILE, e);
	}
}

void WindowMonitor::RegisterObserver(WindowMonitorObserver * observer)
{
	observers.push_back(observer);
}

void WindowMonitor::UnregisterObserver(WindowMonitorObserver * observer)
{
	observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}

void WindowMonitor::NotifyObservers(WindowMonitorEvent const & event)
{
	for (auto it = observers.begin(); it != observers.end(); ++it)
	{
		(*it)->OnWindowMonitorEvent(event);
	}
}