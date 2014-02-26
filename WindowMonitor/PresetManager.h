#pragma once
#include "DoubleRect.h"

typedef std::function<void(std::wstring const & name)> PresetManagerIterateAction;

class PresetManager
{
private:
	enum class LoadStatus
	{
		NotLoaded,
		NotUpdated,
		Updated
	};

public:
	PresetManager();
	void SavePreset(std::wstring const & name, DoubleRect const & dimensions);
	bool GetPreset(std::wstring const & name, DoubleRect & dimensions);
	bool RenamePreset(std::wstring const & currentName, std::wstring const & newName);
	bool RemovePreset(std::wstring const & name);
	void IterateNames(PresetManagerIterateAction action);
	void SaveToBinaryFile();
	bool LoadFromBinaryFile(bool const & skipIfCurrent = true);

private:
	std::map<std::wstring, DoubleRect> presets;
	std::wstring filename;
	time_t lastSaveTime;

	void ReadFileVersion(std::ifstream & in, LoadStatus & result, bool & continueProcessing);
	void CompareSaveTimes(std::ifstream & in, bool const & skipIfCurrent, LoadStatus & result, bool & continueProcessing);
	void CheckItemCount(std::ifstream & in, unsigned long long & count, LoadStatus & result, bool & continueProcessing);
	void LoadItems(std::ifstream & in, unsigned long long & count, LoadStatus & result, bool & continueProcessing);

	static const unsigned char FileVersion;
};