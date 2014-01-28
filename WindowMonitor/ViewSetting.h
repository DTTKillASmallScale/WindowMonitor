#pragma once

class ViewSetting
{
public:
	ViewSetting();
	~ViewSetting();

	void SetFromClientRect(HWND const & hWnd);
	void Shift(long x, long y);
	void Crop(long x, long y);
	double GetAspect();
	void GetScaledDimensions(long & width, long & height);
	void GetScaledRect(double const & width, double const & height, RECT & rect);
	double GetWidth();
	double GetHeight();
	void SetScale(double const & newScale);
	void SetScaleToWindow(RECT const & clientRect);
	void SetScaleToMonitorSize(RECT const & monitorRect);

private:
	double left;
	double top;
	double right;
	double bottom;
	double scale;
	static const double MaxResetRatio;
};

