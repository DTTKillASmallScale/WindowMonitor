#include "stdafx.h"
#include "AdjustableThumbnail.h"

const double AdjustableThumbnail::SCALE_MIN = 0.125;
const double AdjustableThumbnail::SCALE_MAX = 4.0;
const double AdjustableThumbnail::SCALE_INC = 0.0625;

AdjustableThumbnail::AdjustableThumbnail(void) :
	scale(0.5),
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
	RECT rc;
	if (!UpdateThumbnailScale(target, source, rc)) return false;

	// Adjust rect to size of window accounting for styles
	LONG dwStyle = GetWindowLong(target, GWL_STYLE);
	LONG dwExStyle = GetWindowLong(target, GWL_EXSTYLE);
	AdjustWindowRectEx(&rc, dwStyle, FALSE, dwExStyle);

	// Set window size and position
	SetWindowPos(target, NULL, 0, 0, (rc.right - rc.left), (rc.bottom - rc.top), SWP_NOMOVE | SWP_NOZORDER);

	return true;
}

bool AdjustableThumbnail::StepScaleThumbnail(HWND const & target, HWND const & source, short const & delta)
{
	// Check
	if (scale == SCALE_MIN && delta < 0) return true;
	else if (scale == SCALE_MAX && delta > 0) return true;

	// Get scale inc
	double deltaScale;
	if (delta > 0) deltaScale = SCALE_INC;
	else if (delta < 0) deltaScale = -SCALE_INC;
	
	// Set scale
	scale += deltaScale; 
	if (scale < SCALE_MIN) scale = SCALE_MIN;
	else if (scale > SCALE_MAX) scale = SCALE_MAX;

	RECT rc;

	// Scale thumbnail
	UpdateThumbnailScale(target, source, rc);

	// Scale window client area
	GetClientRect(target, &rc);
	double windowScale = 1.0 - (deltaScale / scale);
	rc.right = long(double(rc.right) / windowScale);
	rc.bottom = long(double(rc.bottom) / windowScale);

	// Adjust rect to size of window accounting for styles
	LONG dwStyle = GetWindowLong(target, GWL_STYLE);
	LONG dwExStyle = GetWindowLong(target, GWL_EXSTYLE);
	AdjustWindowRectEx(&rc, dwStyle, FALSE, dwExStyle);

	// Set window size and position
	SetWindowPos(target, NULL, 0, 0, (rc.right - rc.left), (rc.bottom - rc.top), SWP_NOMOVE | SWP_NOZORDER);

	return true;
}

bool AdjustableThumbnail::OffsetThumbnail(HWND const & target, HWND const & source, int const & x, int const & y)
{
	// Set offset
	offsetx += (double)x / scale;
	offsety += (double)y / scale;

	// Update
	RECT rc;
	return UpdateThumbnailScale(target, source, rc);
}

bool AdjustableThumbnail::UpdateThumbnailScale(HWND const & target, HWND const & source, RECT & rect)
{
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

	// Check
	if (!ok)
	{
		return false;
	}

	// Calc width and height
	double sourceWidth(sourceRect.right - sourceRect.left);
	double sourceHeight(sourceRect.bottom - sourceRect.top);

	double x = (double)offsetx * scale;
	double y = (double)offsety * scale;

	// Set output
	rect.left = long(x);
	rect.top = long(y);
	rect.right = long(ceil(sourceWidth * scale) + x);
	rect.bottom = long(ceil(sourceHeight * scale) + y);
	return true;
}
