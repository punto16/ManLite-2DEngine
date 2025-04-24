#ifndef __FILES_MANAGER_EM_H__
#define __FILES_MANAGER_EM_H__
#pragma once

#include "string"
#include "vector"
#include "filesystem"
#include "unordered_map"

enum FileType
{
	UNKNOWN			= 0,
	FOLDER			= 1,
	IMAGE			= 2,
	ANIMATION		= 3,
	AUDIO			= 4,
	FONT			= 5,
	PARTICLES		= 6,
	SCENE			= 7,
	SCRIPT			= 8,
	TILED			= 9,
	PREFAB			= 10,

	//
	COUNT_FILE_TYPE
};

struct FileData
{
	FileType type = FileType::UNKNOWN;
	std::string name = "";
	std::string relative_path = "";
	std::string absolute_path = "";

	//if folder
	std::vector<FileData> children;
};

class FilesManager
{
public:
	static FilesManager& GetInstance();

	bool Update(float dt);

	//reload filedata
	void ProcessFromRoot();

	//file watcher
	void StartWatching();
	void StopWatching();

	FileData& GetFileData() { return assets_folder; }

	void ProcessChanges();

private:
	FilesManager() = default;

	void CheckForRemovals(const std::unordered_map<std::string, std::filesystem::file_time_type>& current_files);
	void UpdateStructure();
	void HandleDeletedItem(const std::string& path);

	void WatchFiles();
	void ProcessDirectory(const std::filesystem::path& path, FileData& parent);
	FileType DetermineFileType(const std::string& extension);

	std::unordered_map<std::string, std::filesystem::file_time_type> file_timestamps;
	bool watching = true;
	//in seconds
	float watch_timer = 0.0f;
	//in seconds
	float watch_frequency = 1.5;

	FileData assets_folder;
	std::vector<std::string> changed_files;
};

#endif // !__FILES_MANAGER_EM_H__