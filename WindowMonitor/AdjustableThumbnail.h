#pragma once
#include "DwmThumbnail.h"

class AdjustableThumbnail
{
public:
	AdjustableThumbnail(void);
	~AdjustableThumbnail(void);

	bool SetThumbnail(HWND const & target, HWND const & source);
	bool UnsetThumbnail();
	bool SetSize(RECT const & rect);

private:
	DwmThumbnail thumbnail;
};

