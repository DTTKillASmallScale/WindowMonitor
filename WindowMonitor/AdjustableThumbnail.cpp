#include "stdafx.h"
#include "AdjustableThumbnail.h"

const double AdjustableThumbnail::SCALE_MIN = 0.125;
const double AdjustableThumbnail::SCALE_MAX = 4.0;
const double AdjustableThumbnail::SCALE_INC = 0.0625;

AdjustableThumbnail::AdjustableThumbnail(void) :
	scale(1.0),
	offsetx(0),
	offsety(0),
	thumbnail()
{
}

AdjustableThumbnail::~AdjustableThumbnail(void)
{
	UnsetThumbnail();
}

bool AdjustableThumbnail::SetThumbnail(HWND const & target, HWND const & source)
{
	// Check
	if (target == NULL) return false;
	if (source == NULL) return false;

	// Reset offset
	offsetx = offsety = 0;

	// Register thumbnail
	bool success = thumbnail.Register(target, source);

	// Resize
	if (success) success = ResizeThumbnail(target, source);

	// Done
	return success;
}

bool AdjustableThumbnail::UnsetThumbnail()
{
	return thumbnail.Unregister();
}

bool AdjustableThumbnail::ResizeThumbnail(HWND const & target, HWND const & source)
{
	// Check
	if (target == NULL) return false;
	if (source == NULL) return false;

	// Update thumbnail
	return UpdateThumbnailScale(target, source);
}

bool AdjustableThumbnail::StepScaleThumbnail(HWND const & target, HWND const & source, short const & delta)
{
	// Check
	if (delta == 0) return true;
	if (scale == SCALE_MIN && delta < 0) return true;
	else if (scale == SCALE_MAX && delta > 0) return true;

	// Get scale inc
	double deltaScale;
	if (delta > 0) deltaScale = SCALE_INC;
	else if (delta < 0) deltaScale = -SCALE_INC;
	
	// Set scale
	return SetScaleThumbnail(target, source, scale + deltaScale);
}

bool AdjustableThumbnail::SetScaleThumbnail(HWND const & target, HWND const & source, double const & newScale)
{
	double deltaScale = newScale - scale;

	// Set scale
	scale = newScale; 
	if (scale < SCALE_MIN) scale = SCALE_MIN;
	else if (scale > SCALE_MAX) scale = SCALE_MAX;

	// Scale thumbnail
	return UpdateThumbnailScale(target, source);
}

bool AdjustableThumbnail::OffsetThumbnail(HWND const & target, HWND const & source, int const & x, int const & y)
{
	// Set offset
	offsetx += static_cast<double>(x) / scale;
	offsety += static_cast<double>(y) / scale;

	// Update
	return UpdateThumbnailScale(target, source);
}

bool AdjustableThumbnail::UpdateThumbnailScale(HWND const & target, HWND const & source)
{
	RECT rect;

	// Get client area rect for target
	GetClientRect(target, &rect);

	// Scale dest rect
	if (!ScaleDestRect(source, rect)) return false;

	// Update thumbnail
	return thumbnail.Scale(rect);
}

bool AdjustableThumbnail::ScaleDestRect(HWND const & source, RECT & rect)
{
	// Check
	if (source == NULL) return false;
	
	// Get source rect
	RECT sourceRect;
	BOOL ok = GetClientRect(source, &sourceRect);
	if (!ok) return false;

	// Calc width, height and offset
	double sourceWidth(sourceRect.right - sourceRect.left);
	double sourceHeight(sourceRect.bottom - sourceRect.top);
	double x = offsetx * scale;
	double y = offsety * scale;

	// Set output
	rect.left = static_cast<long>(x);
	rect.top = static_cast<long>(y);
	rect.right = static_cast<long>(ceil(sourceWidth * scale) + x);
	rect.bottom = static_cast<long>(ceil(sourceHeight * scale) + y);
	return true;
}
