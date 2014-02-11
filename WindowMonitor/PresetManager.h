#pragma once
#include "DoubleRect.h"

class PresetManager
{
public:
	PresetManager();
	void SavePreset(std::wstring const & name, DoubleRect const & dimensions);
	bool GetPreset(std::wstring const & name, DoubleRect & dimensions);
	bool RenamePreset(std::wstring const & currentName, std::wstring const & newName);
	bool RemovePreset(std::wstring const & name);
	void SaveToBinaryFile();
	void LoadFromBinaryFile();
	char FileUpdatedSinceLastRead();
	void IterateNames(std::function<void(std::wstring const &)> step);

private:
	std::map<std::wstring, DoubleRect> presets;
	std::wstring filename;
	time_t lastSaveTime;

	static const unsigned char FileVersion;
};