#include "FilesManager.h"
#include <algorithm> 

namespace fs = std::filesystem;

FilesManager& FilesManager::GetInstance()
{
    static FilesManager instance;
    return instance;
}

void FilesManager::ProcessFromRoot()
{
    assets_folder.children.clear();

    assets_folder.type = FOLDER;
    assets_folder.name = "Assets";
    assets_folder.relative_path = "Assets";
    assets_folder.absolute_path = fs::absolute("Assets").string();

    ProcessDirectory(assets_folder.relative_path, assets_folder);
}

void FilesManager::ProcessDirectory(const std::filesystem::path& path, FileData& parent)
{
    for (const auto& entry : fs::directory_iterator(path))
    {
        FileData child;
        child.relative_path = parent.relative_path + "/" + entry.path().filename().string();
        child.absolute_path = fs::absolute(child.relative_path).string();
        child.name = entry.path().filename().string();

        if (entry.is_directory())
        {
            child.type = FOLDER;
            ProcessDirectory(child.relative_path, child);
        }
        else
        {
            std::string extension = entry.path().extension().string();
            child.type = DetermineFileType(extension);
        }

        parent.children.push_back(child);
    }
}

FileType FilesManager::DetermineFileType(const std::string& extension)
{
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".gif") return IMAGE;
    if (ext == ".wav" || ext == ".mp3" || ext == ".ogg") return AUDIO;
    if (ext == ".ttf" || ext == ".otf") return FONT;
    if (ext == ".mlparticle") return PARTICLES;
    if (ext == ".cs") return SCRIPT;
    if (ext == ".animation") return ANIMATION;
    if (ext == ".mlscene") return SCENE;
    if (ext == ".tmx" || ext == ".json") return TILED;

    return UNKNOWN;
}
