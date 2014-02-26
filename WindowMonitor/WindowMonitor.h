#pragma once
#include "DoubleRect.h"
#include "WindowFilter.h"
#include "PresetManager.h"
#include "WindowMonitorObserver.h"

class WindowMonitor
{
public:
	WindowMonitor(WindowFilter * sources, PresetManager * presets);
	~WindowMonitor();

	void UpdateSources();
	void SelectSourceByHash(std::size_t const & hash);
	void SelectFirstSource();
	void SelectNextSource();
	void SelectPreviousSource();
	void UpdatePresets();
	void SelectPreset(std::wstring const & name);
	void UnselectPreset();
	void SavePreset(std::wstring const & name);
	void DeleteSelectedPreset();

	void Shift(long const & x, long const & y);
	void Crop(long const & x, long const & y);
	void Scale(double const & newScale);
	void ScaleToFitWindow(HWND const & hWnd);
	void ScaleToFitMonitor(HWND const & hWnd, bool const & maximize = false);
	void ResetAndScaleToFitMonitor(HWND const & hWnd);

	inline size_t GetSourceHash() { return selectedSource.hash; }
	inline HWND GetSourceWindow() { return selectedSource.hwnd; }
	inline std::wstring GetSelectedPresetName() { return selectedPreset; }
	inline double GetWidth() { return dimensions.right - dimensions.left; }
	inline double GetHeight() { return dimensions.bottom - dimensions.top; }
	inline double GetScaledWidth() { return (dimensions.right - dimensions.left) * scale; }
	inline double GetScaledHeight() { return (dimensions.bottom - dimensions.top) * scale; }
	inline double GetAspect(){ return (dimensions.right - dimensions.left) / (dimensions.bottom - dimensions.top); }
	inline DoubleRect GetDimensions() { return dimensions; }
	RECT GetScaledRect();

	void IterateSources(std::function<void(WindowFilterItem const & item, bool const & selected)> action);
	void IteratePresets(std::function<void(std::wstring const & text, bool const & selected)> action);

	void RegisterObserver(WindowMonitorObserver * observer);
	void UnregisterObserver(WindowMonitorObserver * observer);

private:
	DoubleRect dimensions;
	double scale;

	WindowFilterItem selectedSource;
	std::wstring selectedPreset;

	WindowFilter * sources;
	PresetManager * presets;

	std::vector<WindowMonitorObserver *> observers;

	void SetDimensionsToSelectedSource();
	void ScaleToFitMonitorWithoutNotification(HWND const & hWnd, bool const & maximize = false); 
	void NotifyObservers(WindowMonitorEvent const & event);

	static const std::wregex WhitespacePattern;
};

