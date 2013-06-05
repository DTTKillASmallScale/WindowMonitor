#pragma once
#include "DwmThumbnail.h"

class AdjustableThumbnail
{
public:
	AdjustableThumbnail(void);
	~AdjustableThumbnail(void);

	bool SetThumbnail(HWND const & target, HWND const & source);
	bool UnsetThumbnail();
	bool ResizeThumbnail(HWND const & target, HWND const & source);
	bool StepScaleThumbnail(HWND const & target, HWND const & source, short const & delta);
	bool OffsetThumbnail(HWND const & target, HWND const & source, int const & x, int const & y);

private:
	bool UpdateThumbnailScale(HWND const & target, HWND const & source, RECT & rect);
	bool ScaleDestRect(HWND const & source, RECT & rect);

	double offsetx;
	double offsety;
	double scale;
	DwmThumbnail thumbnail;

	// Constants
	static const double SCALE_MIN;
	static const double SCALE_MAX;
	static const double SCALE_INC;
};

