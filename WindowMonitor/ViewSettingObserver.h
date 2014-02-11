#pragma once

enum class ViewSettingObserverState
{
	SetFromClientRect = 1,
	Shift,
	Crop,
	CopyFrom
};

class ViewSettingObserver
{
public:
	virtual void ViewSettingUpdated(ViewSettingObserverState const & state) = 0;
};