#include "stdafx.h"
#include "PresetManager.h"
#include "WindowHelper.h"
#include <fstream>

const unsigned char PresetManager::FileVersion = 1;

PresetManager::PresetManager() :
	lastSaveTime(0)
{
	// Create config path
	WindowHelper::GetPathToExecutable(filename);
	filename.append(L"config.dat");
}

void PresetManager::SavePreset(std::wstring const & name, DoubleRect const & dimensions)
{
	auto it = presets.find(name);
	bool found = (it != presets.end());
	if (found) it->second.CopyFrom(dimensions);
	else presets.insert(std::make_pair(name, dimensions));
	SaveToBinaryFile();
}

bool PresetManager::GetPreset(std::wstring const & name, DoubleRect & dimensions)
{
	auto it = presets.find(name);
	bool found = (it != presets.end());
	if (found) dimensions.CopyFrom(it->second);
	return found;
}

bool PresetManager::RenamePreset(std::wstring const & currentName, std::wstring const & newName)
{
	bool foundNew = (presets.find(newName) != presets.end());
	if (foundNew) return false;
	
	auto it = presets.find(currentName);
	bool foundCurrent = (it != presets.end());
	if (!foundCurrent) return false;

	presets.insert(std::make_pair(newName, it->second));
	presets.erase(it);

	SaveToBinaryFile();

	return true;
}

bool PresetManager::RemovePreset(std::wstring const & name)
{
	auto it = presets.find(name);
	bool found = (it != presets.end());
	if (found) 
	{
		presets.erase(it);
		SaveToBinaryFile();
	}
	return found;
}

void PresetManager::IterateNames(PresetManagerIterateAction action)
{
	for (auto it = presets.begin(); it != presets.end(); ++it)
	{
		action(it->first);
	}
}

void PresetManager::SaveToBinaryFile()
{
	// Open file
	std::ofstream out(filename, std::ios::binary);

	// Write file version
	out.write(reinterpret_cast<const char*>(&PresetManager::FileVersion), sizeof(PresetManager::FileVersion));

	// Write save time
	time(&lastSaveTime);
	out.write(reinterpret_cast<const char*>(&lastSaveTime), sizeof(lastSaveTime));

	// Write item count
	unsigned long long count = presets.size();
	out.write(reinterpret_cast<const char*>(&count), sizeof(count));

	// Write items
	for (auto it = presets.begin(); it != presets.end(); ++it)
	{
		// Write size of string in bytes
		unsigned long long size = it->first.size() * sizeof(wchar_t);
		out.write(reinterpret_cast<const char*>(&size), sizeof(size));

		// Write string
		byte const * data = reinterpret_cast<byte const *>(it->first.c_str());
		out.write(reinterpret_cast<const char*>(data), size);

		// Write dimensions
		out.write(reinterpret_cast<const char*>(&it->second.top), sizeof(it->second.top));
		out.write(reinterpret_cast<const char*>(&it->second.left), sizeof(it->second.left));
		out.write(reinterpret_cast<const char*>(&it->second.bottom), sizeof(it->second.bottom));
		out.write(reinterpret_cast<const char*>(&it->second.right), sizeof(it->second.right));
	}

	// Close
	out.flush();
	out.close();
}

bool PresetManager::LoadFromBinaryFile(bool const & skipIfCurrent)
{
	LoadStatus result = LoadStatus::NotLoaded;
	bool continueProcessing = false;
	unsigned long long count = 0;

	// Open file
	std::ifstream in(filename, std::ios::binary);
	if (in) continueProcessing = true;
	else presets.clear();

	// Process file
	if (continueProcessing) ReadFileVersion(in, result, continueProcessing);
	if (continueProcessing) CompareSaveTimes(in, skipIfCurrent, result, continueProcessing);
	if (continueProcessing) CheckItemCount(in, count, result, continueProcessing);
	if (continueProcessing) LoadItems(in, count, result, continueProcessing);

	// Done
	in.close();
	return result != LoadStatus::NotUpdated;
}

void PresetManager::ReadFileVersion(std::ifstream & in, LoadStatus & result, bool & continueProcessing)
{
	// Read file version
	unsigned char version = 0;
	in.read(reinterpret_cast<char*>(&version), sizeof(version));

	// Check version
	if (version != PresetManager::FileVersion)
	{
		throw std::runtime_error("Preset Manager: File version is unsupported");
		result = LoadStatus::NotLoaded;
		continueProcessing = false;
	}
	else
	{
		continueProcessing = true;
	}
}

void PresetManager::CompareSaveTimes(std::ifstream & in, bool const & skipIfCurrent, LoadStatus & result, bool & continueProcessing)
{
	// Read save time
	time_t fileTime;
	in.read(reinterpret_cast<char*>(&fileTime), sizeof(fileTime));

	// Compare times
	if (skipIfCurrent == true && fileTime <= lastSaveTime)
	{
		result = LoadStatus::NotUpdated;
		continueProcessing = false;
	}
	else
	{
		continueProcessing = true;
	}

	lastSaveTime = fileTime;
}

void PresetManager::CheckItemCount(std::ifstream & in, unsigned long long & count, LoadStatus & result, bool & continueProcessing)
{
	// Read item count
	in.read(reinterpret_cast<char*>(&count), sizeof(count));

	// Store old item count and clear
	auto oldItemCount = presets.size();
	presets.clear();

	// Check file has items
	if (count < 1) continueProcessing = false;
	else continueProcessing = true;
	result = LoadStatus::Updated;
}

void PresetManager::LoadItems(std::ifstream & in, unsigned long long & count, LoadStatus & result, bool & continueProcessing)
{
	// Read items
	while (in && count--)
	{
		unsigned long long size = 0;
		std::wstring name;
		DoubleRect dimensions;

		// Read size of string
		in.read(reinterpret_cast<char*>(&size), sizeof(size));

		// Read string
		if (in && size)
		{
			// Create buffer for wstring with space for null terminator
			size_t count = static_cast<size_t>(size) / sizeof(wchar_t)+1;
			std::vector<wchar_t> buffer(count, '\0');

			// Read data
			in.read(reinterpret_cast<char*>(&buffer[0]), size);

			// Assign to string
			name.assign(&buffer[0]);
		}
		else
		{
			if (!in) throw std::runtime_error("Preset Manager: Stream ended when reading preset name");
			if (!size) throw std::runtime_error("Preset Manager: Preset name length was zero");
			presets.clear();
			result = LoadStatus::NotLoaded;
			continueProcessing = false;
			return;
		}

		// Read dimensions
		in.read(reinterpret_cast<char*>(&dimensions.top), sizeof(dimensions.top));
		in.read(reinterpret_cast<char*>(&dimensions.left), sizeof(dimensions.left));
		in.read(reinterpret_cast<char*>(&dimensions.bottom), sizeof(dimensions.bottom));
		in.read(reinterpret_cast<char*>(&dimensions.right), sizeof(dimensions.right));

		// Add to map
		presets.emplace(std::make_pair(name, dimensions));
	}

	result = LoadStatus::Updated;
	continueProcessing = true;
}
