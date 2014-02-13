#include "stdafx.h"
#include "ViewSettingObserver.h"

std::vector<ViewSettingObserver*> ViewSettingObserver::observers;

void ViewSettingObserver::NotifyObservers(ViewSettingObserverSource const & eventSource, void * data)
{
	for (auto it = observers.begin(); it != observers.end(); ++it)
	{
		(*it)->ViewSettingUpdated(eventSource, data);
	}
}

void ViewSettingObserver::RegisterObserver(ViewSettingObserver * obs)
{
	observers.push_back(obs);
}

void ViewSettingObserver::UnregisterObserver(ViewSettingObserver * obs)
{
	observers.erase(std::remove(observers.begin(), observers.end(), obs), observers.end());
}
