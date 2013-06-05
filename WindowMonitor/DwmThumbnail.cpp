#include "stdafx.h"
#include "DwmThumbnail.h"

DwmThumbnail::DwmThumbnail(void) : 
	thumbnail(NULL)
{
}


DwmThumbnail::~DwmThumbnail(void)
{
	Unregister();
}

bool DwmThumbnail::IsCreated() const
{
	return thumbnail != NULL;
}

bool DwmThumbnail::Register(HWND const & targetWindow, HWND const & sourceWindow)
{
	// Check
	if (targetWindow == NULL) return false;
	if (sourceWindow == NULL) return false;

	// Unregister exisiting thumbnail
	if (IsCreated()) Unregister();

	// Register thumbnail
	HRESULT hr = DwmRegisterThumbnail(targetWindow, sourceWindow, &thumbnail);

	// Check thumbnail is registered
	if (!SUCCEEDED(hr))
	{
		thumbnail = NULL;
		return false;
	}

	// Update props
	InitializeProperties();

	// Done
	return true;
}

bool DwmThumbnail::Scale(RECT const & destRect)
{
	if (!IsCreated()) return false;

	// Set thumb props
	DWM_THUMBNAIL_PROPERTIES thumbProps;
	SecureZeroMemory(&thumbProps, sizeof(DWM_THUMBNAIL_PROPERTIES));
	thumbProps.dwFlags = DWM_TNP_RECTDESTINATION;
	thumbProps.rcDestination = destRect;

	// Update thumb props
	HRESULT hr = DwmUpdateThumbnailProperties(thumbnail, &thumbProps);

	// Check thumbnail is updated
	if (!SUCCEEDED(hr)) return false;
	else return true;
}

bool DwmThumbnail::InitializeProperties()
{
	if (!IsCreated()) return false;

	// Set thumb props
	DWM_THUMBNAIL_PROPERTIES thumbProps;
	SecureZeroMemory(&thumbProps, sizeof(DWM_THUMBNAIL_PROPERTIES));
	thumbProps.dwFlags = DWM_TNP_SOURCECLIENTAREAONLY | DWM_TNP_VISIBLE | DWM_TNP_OPACITY;
	thumbProps.fSourceClientAreaOnly = TRUE;
	thumbProps.fVisible = TRUE;
	thumbProps.opacity = 255;

	// Update thumb props
	HRESULT hr = DwmUpdateThumbnailProperties(thumbnail, &thumbProps);

	// Check thumbnail is updated
	if (!SUCCEEDED(hr)) return false;
	else return true;
}

bool DwmThumbnail::Unregister()
{
	// Check
	if (!IsCreated()) return true;
	
	// Unregister thumbnail
	HRESULT hr = DwmUnregisterThumbnail(thumbnail);
	
	// Check thumbnail is unregistered
	if (!SUCCEEDED(hr)) return false;
	
	// Success
	thumbnail = NULL;
	return true;
}
