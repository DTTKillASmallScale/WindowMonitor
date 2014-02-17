#pragma once
#include "DoubleRect.h"

typedef std::function<void(std::wstring const & name)> PresetManagerIterateAction;

class PresetManager
{
public:
	PresetManager();
	void SavePreset(std::wstring const & name, DoubleRect const & dimensions);
	bool GetPreset(std::wstring const & name, DoubleRect & dimensions);
	bool RenamePreset(std::wstring const & currentName, std::wstring const & newName);
	bool RemovePreset(std::wstring const & name);
	void IterateNames(PresetManagerIterateAction action);
	void SaveToBinaryFile();

	// Returns true if items are loaded
	bool LoadFromBinaryFile(bool const & skipIfCurrent = true);

private:
	std::map<std::wstring, DoubleRect> presets;
	std::wstring filename;
	time_t lastSaveTime;

	static const unsigned char FileVersion;
};