#pragma once

class CWindow;

enum class ViewSettingObserverSource
{
	SelectPresetFromManager = 1,
	SelectPresetFromMenu,
	Shift,
	Crop,
	Scale,
	SavePreset,
	Reset
};

class ViewSettingObserver
{
public:
	virtual void ViewSettingUpdated(ViewSettingObserverSource const & eventSource, void * data) = 0;

	static void NotifyObservers(ViewSettingObserverSource const & eventSource, void * data = NULL);
	static void RegisterObserver(ViewSettingObserver * obs);
	static void UnregisterObserver(ViewSettingObserver * obs);

private:
	static std::vector<ViewSettingObserver*> observers;
};