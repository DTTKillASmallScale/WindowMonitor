#include "stdafx.h"
#include "resource.h"
#include "WindowMonitor.h"
#include "WindowHelper.h"

const std::wregex WindowMonitor::WhitespacePattern(L"^\\s+|\\s+$");

WindowMonitor::WindowMonitor(WindowFilter * sources, PresetManager * presets) :
	sources(sources),
	presets(presets),
	dimensions(),
	scale(1.0),
	selectedPreset()
{
}

WindowMonitor::~WindowMonitor()
{
}

void WindowMonitor::SelectSourceByHash(std::size_t const & hash)
{
	// Get filtered windows
	bool sourcesUpdated = sources->Refresh();
	
	if (sources->ItemCount() != 0)
	{
		// Set selected index
		selectedSource = sources->GetItemByHash(hash);

		// Set dimensions
		SetDimensionsToSelectedSource();

		// Send update event
		NotifyObservers(WindowMonitorEvent::SourceSelected | (sourcesUpdated ? WindowMonitorEvent::SourcesUpdated : WindowMonitorEvent::Nothing));
	}
	else
	{
		// Send update event
		if (sourcesUpdated) NotifyObservers(WindowMonitorEvent::SourcesUpdated);
	}
}

void WindowMonitor::SelectFirstSource()
{
	// Get filtered windows
	bool sourcesUpdated = sources->Refresh();

	if (sources->ItemCount() != 0)
	{
		// Set selected index
		selectedSource = sources->GetItem(0);

		// Set dimensions
		SetDimensionsToSelectedSource();

		// Send update event
		NotifyObservers(WindowMonitorEvent::SourceSelected | (sourcesUpdated ? WindowMonitorEvent::SourcesUpdated : WindowMonitorEvent::Nothing));
	}
	else
	{
		// Send update event
		if (sourcesUpdated) NotifyObservers(WindowMonitorEvent::SourcesUpdated);
	}
}

void WindowMonitor::SelectNextSource()
{
	// Get filtered windows
	bool sourcesUpdated = sources->Refresh();

	if (sources->ItemCount() != 0)
	{
		// Get next item
		selectedSource = sources->GetNextItem(selectedSource);

		// Set dimensions
		SetDimensionsToSelectedSource();

		// Send update event
		NotifyObservers(WindowMonitorEvent::SourceSelected | (sourcesUpdated ? WindowMonitorEvent::SourcesUpdated : WindowMonitorEvent::Nothing));
	}
	else
	{
		// Send update event
		if (sourcesUpdated) NotifyObservers(WindowMonitorEvent::SourcesUpdated);
	}
}

void WindowMonitor::SelectPreviousSource()
{
	// Get filtered windows
	bool sourcesUpdated = sources->Refresh();

	if (sources->ItemCount() != 0)
	{
		// Get previous item
		selectedSource = sources->GetPreviousItem(selectedSource);

		// Set dimensions
		SetDimensionsToSelectedSource();

		// Send update event
		NotifyObservers(WindowMonitorEvent::SourceSelected | (sourcesUpdated ? WindowMonitorEvent::SourcesUpdated : WindowMonitorEvent::Nothing));
	}
	else
	{
		// Send update event
		if (sourcesUpdated) NotifyObservers(WindowMonitorEvent::SourcesUpdated);
	}
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

void WindowMonitor::SetDimensionsToSelectedSource()
{
	RECT tmp;
	if (GetClientRect(selectedSource.hwnd, &tmp))
	{
		dimensions.bottom = static_cast<double>(tmp.bottom);
		dimensions.left = static_cast<double>(tmp.left);
		dimensions.right = static_cast<double>(tmp.right);
		dimensions.top = static_cast<double>(tmp.top);
	}
}

void WindowMonitor::ResetAndScaleToFitMonitor(HWND const & hWnd)
{
	SetDimensionsToSelectedSource();
	selectedPreset.clear();
	ScaleToFitMonitorWithoutNotification(hWnd);
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
	scale = static_cast<double>(rect.right - rect.left) / (dimensions.right - dimensions.left);
	NotifyObservers(WindowMonitorEvent::ScaledToWindow);
}

void WindowMonitor::ScaleToFitMonitor(HWND const & hWnd, bool const & maximize)
{
	ScaleToFitMonitorWithoutNotification(hWnd, maximize);
	NotifyObservers(WindowMonitorEvent::ScaledToMonitor);
}

void WindowMonitor::ScaleToFitMonitorWithoutNotification(HWND const & hWnd, bool const & maximize)
{
	// Get monitor rect for window
	RECT monitorRect;
	WindowHelper::GetMonitorRect(hWnd, monitorRect);

	// Get horizontal ratio
	double sourceMonitorRatio = (dimensions.right - dimensions.left) / static_cast<double>(monitorRect.right - monitorRect.left);

	// Use vertical ratio if larger
	sourceMonitorRatio = max(sourceMonitorRatio, (dimensions.bottom - dimensions.top) / static_cast<double>(monitorRect.bottom - monitorRect.top));

	// Set scale
	if (maximize)
	{
		scale = 1.0 / sourceMonitorRatio;
	}
	else
	{
		double const maxMonitorFitRatio = 0.9;
		if (sourceMonitorRatio > maxMonitorFitRatio) scale = maxMonitorFitRatio / sourceMonitorRatio;
		else scale = 1.0;
	}
}

RECT WindowMonitor::GetScaledRect()
{
	RECT rect;
	GetClientRect(selectedSource.hwnd, &rect);
	rect.left = static_cast<long>(ceil(-dimensions.left * scale));
	rect.top = static_cast<long>(ceil(-dimensions.top * scale));
	rect.right = static_cast<long>(ceil((static_cast<double>(rect.right) - dimensions.left) * scale));
	rect.bottom = static_cast<long>(ceil((static_cast<double>(rect.bottom) - dimensions.top) * scale));
	return rect;
}

void WindowMonitor::UpdateSources()
{
	if (sources->Refresh()) NotifyObservers(WindowMonitorEvent::SourcesUpdated);
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

void WindowMonitor::IterateSources(std::function<void(WindowFilterItem const & item, bool const & selected)> action)
{
	sources->IterateItems([&](WindowFilterItem const & item)
	{
		action(item, (item == selectedSource));
		return false;
	});
}

void WindowMonitor::IteratePresets(std::function<void(std::wstring const & text, bool const & selected)> action)
{
	presets->IterateNames([&](std::wstring const & name)
	{
		action(name, (name == selectedPreset));
	});
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
#ifdef _DEBUG
	DebugWindowMonitorEventValue(event);
#endif

	for (auto it = observers.begin(); it != observers.end(); ++it)
	{
		(*it)->OnWindowMonitorEvent(event);
	}
}