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
	// Unregister exisiting thumbnail
	Unregister();

	// Check
	if (targetWindow == NULL) return false;
	if (sourceWindow == NULL) return false;

	// Register thumbnail
	HRESULT hr = DwmRegisterThumbnail(targetWindow, sourceWindow, &thumbnail);

	// Check thumbnail is registered
	if (SUCCEEDED(hr))
	{
		InitializeProperties();
		return true;
	}
	else
	{
		thumbnail = NULL;
		return false;
	}
}

bool DwmThumbnail::Unregister()
{
	bool success = false;

	if (thumbnail != NULL) 
	{
		HRESULT hr = DwmUnregisterThumbnail(thumbnail);
		success = SUCCEEDED(hr);
	}

	thumbnail = NULL;
	return success;
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
	return SUCCEEDED(hr);
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
	return SUCCEEDED(hr);
}
