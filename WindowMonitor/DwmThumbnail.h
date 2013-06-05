#pragma once

class DwmThumbnail
{
public:
	DwmThumbnail(void);
	~DwmThumbnail(void);

	bool IsCreated() const;
	bool Register(HWND const & targetWindow, HWND const & sourceWindow);
	bool Scale(RECT const & destRect);
	bool Unregister();

private:
	HTHUMBNAIL thumbnail;

	bool InitializeProperties();
};

