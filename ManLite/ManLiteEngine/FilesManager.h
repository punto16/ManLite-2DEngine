#ifndef __FILES_MANAGER_EM_H__
#define __FILES_MANAGER_EM_H__
#pragma once

#include "string"
#include "vector"
#include "filesystem"

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

	void ProcessFromRoot();

	FileData& GetFileData() { return assets_folder; }

private:
	FilesManager() = default;

	void ProcessDirectory(const std::filesystem::path& path, FileData& parent);
	FileType DetermineFileType(const std::string& extension);

	FileData assets_folder;
};

#endif // !__FILES_MANAGER_EM_H__