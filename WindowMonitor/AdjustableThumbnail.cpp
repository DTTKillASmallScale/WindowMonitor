#include "stdafx.h"
#include "AdjustableThumbnail.h"

AdjustableThumbnail::AdjustableThumbnail(void) :
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

bool AdjustableThumbnail::SetSize(RECT const & rect)
{
	return thumbnail.Scale(rect);
}

bool AdjustableThumbnail::ResizeThumbnail(HWND const & target, HWND const & source)
{
	// Check
	if (target == NULL) return false;
	if (source == NULL) return false;

	// Get source rect
	RECT sourceRect;
	BOOL ok = GetClientRect(source, &sourceRect);
	if (!ok) return false;

	// Set target rect
	RECT targetRect;
	targetRect.left = 0;
	targetRect.top = 0;
	targetRect.right = sourceRect.right - sourceRect.left;
	targetRect.bottom = sourceRect.bottom - sourceRect.top;

	// Update thumbnail
	return thumbnail.Scale(targetRect);
}
