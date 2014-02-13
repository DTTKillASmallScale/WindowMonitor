#include "stdafx.h"
#include "ViewSetting.h"
#include "ViewSettingObserver.h"

const double ViewSetting::MaxResetRatio = 0.9;

ViewSetting::ViewSetting() :
	scale(1.0)
{
}

ViewSetting::~ViewSetting()
{
}

void ViewSetting::SetFromClientRect(HWND const & hWnd)
{
	RECT tmp;
	if (GetClientRect(hWnd, &tmp))
	{
		bottom = static_cast<double>(tmp.bottom);
		left = static_cast<double>(tmp.left);
		right = static_cast<double>(tmp.right);
		top = static_cast<double>(tmp.top);
		//NotifyObservers(ViewSettingObserverState::SetFromClientRect);
	}
}

void ViewSetting::Shift(long x, long y)
{
	left -= x / scale;
	top -= y / scale;
	right -= x / scale;
	bottom -= y / scale;
	//NotifyObservers(ViewSettingObserverState::Shift);
}

void ViewSetting::Crop(long x, long y)
{
	right += x / scale;
	bottom += y / scale;
	//NotifyObservers(ViewSettingObserverState::Crop);
}

void ViewSetting::CopyFrom(DoubleRect const & other)
{
	DoubleRect::CopyFrom(other);
	//NotifyObservers(ViewSettingObserverState::CopyFrom);
}

void ViewSetting::SetScale(double const & newScale)
{
	scale = newScale;
}

void ViewSetting::SetScaleToWindow(RECT const & clientRect)
{
	scale = static_cast<double>(clientRect.right - clientRect.left) / ceil(right - left);
}

void ViewSetting::SetScaleToMonitorSize(RECT const & monitorRect)
{
	double sourceMonitorRatio = (right - left) / static_cast<double>(monitorRect.right - monitorRect.left);
	sourceMonitorRatio = max(sourceMonitorRatio, (bottom - top) / static_cast<double>(monitorRect.bottom - monitorRect.top));
	if (sourceMonitorRatio > MaxResetRatio) scale = MaxResetRatio / sourceMonitorRatio;
	else scale = 1.0;
}

double ViewSetting::GetAspect()
{
	return (right - left) / (bottom - top);
}

void ViewSetting::GetScaledDimensions(long & width, long & height)
{
	width = static_cast<long>((right - left) * scale);
	height = static_cast<long>((bottom - top) * scale);
}

void ViewSetting::GetScaledRect(double const & width, double const & height, RECT & rect)
{
	rect.left = static_cast<long>(-left * scale);
	rect.top = static_cast<long>(-top * scale);
	rect.right = static_cast<long>((width - left) * scale);
	rect.bottom = static_cast<long>((height - top) * scale);
}

double ViewSetting::GetWidth()
{
	return (right - left);
}

double ViewSetting::GetHeight()
{
	return (bottom - top);
}
