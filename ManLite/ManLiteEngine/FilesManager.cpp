#include "FilesManager.h"

#include "EngineCore.h"
#include "ResourceManager.h"
#include "ScriptingEM.h"

#include <algorithm> 

#include <regex>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "chrono"

namespace fs = std::filesystem;
using namespace std::chrono_literals;

FilesManager& FilesManager::GetInstance()
{
    static FilesManager instance;
    return instance;
}

bool FilesManager::Update(float dt)
{
    bool ret = true;
    update_happened_this_frame = false;
    if (watching)
    {
        watch_timer += dt;
        if (watch_timer >= watch_frequency)
        {
            WatchFiles();
            watch_timer = 0.0f;
        }
    }
    return ret;
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

bool FilesManager::MoveFile_(const std::string& sourcePath, const std::string& destFolderPath)
{
    std::error_code ec;
    fs::path absSource = fs::absolute(sourcePath, ec);
    if (ec || !fs::exists(absSource)) return false;

    fs::path absDestFolder = fs::absolute(destFolderPath, ec);
    if (ec || !fs::is_directory(absDestFolder)) return false;

    fs::path newPath = absDestFolder / absSource.filename();

    if (fs::exists(newPath)) return false;

    bool success = false;
    try {
        fs::rename(absSource, newPath);
        success = true;
    }
    catch (...)
    {
        try
        {
            if (fs::is_directory(absSource))
            {
                fs::copy(absSource, newPath, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
            }
            else
            {
                fs::copy_file(absSource, newPath, fs::copy_options::overwrite_existing);
            }
            fs::remove_all(absSource);
            success = true;
        }
        catch (...)
        {
            success = false;
        }
    }

    if (success)
    {
        CallUpdateFiles();
        return true;
    }
    return false;
}

bool FilesManager::DuplicateFile(const std::string& originalPath)
{
    std::error_code ec;
    fs::path source = fs::absolute(originalPath, ec);

    if (ec || !fs::exists(source)) return false;

    fs::path parentDir = source.parent_path();
    std::string extension = source.extension().string();
    std::string originalStem = source.stem().string();

    int copyNumber = 1;
    std::string newStem;
    fs::path newPath;

    std::regex copyRegex(R"((.*) - Copy(?: \((\d+)\))?$)");

    do {
        std::smatch match;
        std::string currentStem = (newStem.empty()) ? originalStem : newStem;

        if (std::regex_match(currentStem, match, copyRegex))
        {
            std::string baseName = match[1].str();
            if (match[2].matched) {
                copyNumber = std::stoi(match[2].str()) + 1;
            }
            else {
                copyNumber = 2;
            }
            newStem = baseName + " - Copy (" + std::to_string(copyNumber) + ")";
        }
        else {
            newStem = currentStem + " - Copy";
        }

        newPath = parentDir / (newStem + extension);

    } while (fs::exists(newPath));

    try
    {
        if (fs::is_directory(source))
        {
            fs::copy(source, newPath, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        }
        else
        {
            fs::copy_file(source, newPath);
        }

        StopWatching();
        ProcessFromRoot();
        StartWatching();

        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool FilesManager::DeleteFile_(const std::string& path)
{
    std::error_code ec;
    fs::path absPath = fs::absolute(path, ec);
    if (ec || !fs::exists(absPath)) return false;

    try
    {
        fs::remove_all(absPath);
        CallUpdateFiles();
        return true;
    }
    catch (...)
    {
        return false;
    }
}

void FilesManager::OpenFile(const std::string& path)
{
#ifdef _WIN32
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
    wchar_t* widePath = new wchar_t[wideLen];
    MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, widePath, wideLen);
    ShellExecuteW(nullptr, L"open", widePath, nullptr, nullptr, SW_SHOWNORMAL);
    delete[] widePath;
#elif defined(__APPLE__)
    CFStringRef pathStr = CFStringCreateWithCString(
        kCFAllocatorDefault,
        path.c_str(),
        kCFStringEncodingUTF8
    );
    CFURLRef url = CFURLCreateWithFileSystemPath(
        kCFAllocatorDefault,
        pathStr,
        kCFURLPOSIXPathStyle,
        false
    );
    LSOpenCFURLRef(url, nullptr);
    CFRelease(url);
    CFRelease(pathStr);
#else
    std::string command = "xdg-open \"" + path + "\"";
    system(command.c_str());
#endif
}

bool FilesManager::CreateFolder(const std::string& path)
{
    std::error_code ec;
    bool created = fs::create_directory(path, ec);
    if (created && !ec)
    {
        CallUpdateFiles();
        return true;
    }
    return false;
}

bool FilesManager::RenameFile(const std::string& old_path, const std::string& new_name)
{
    std::error_code ec;
    fs::path oldFsPath = fs::absolute(old_path, ec);

    if (ec || !fs::exists(oldFsPath)) return false;
    if (new_name.empty() || new_name.find_first_of("\\/:*?\"<>|") != std::string::npos) return false;

    fs::path parentDir = oldFsPath.parent_path();
    fs::path newFsPath = parentDir / new_name;

    if (fs::exists(newFsPath))
    {  
        LOG(LogType::LOG_WARNING, "File \"%s\" already exists!\n", newFsPath.c_str());
        return false;
    }

    try
    {
        fs::rename(oldFsPath, newFsPath);

        CallUpdateFiles();

        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

void FilesManager::CallUpdateFiles()
{
    watch_timer = watch_frequency;
}

void FilesManager::StartWatching()
{
    watching = true;
    watch_timer = watch_frequency;
}

void FilesManager::StopWatching()
{
    watching = false;
    watch_timer = 0.0f;
}

FileData* FilesManager::GetFileDataByPath(std::string path)
{
    std::replace(path.begin(), path.end(), '\\', '/');

    std::vector<std::string> path_parts;
    std::stringstream ss(path);
    std::string part;

    while (std::getline(ss, part, '/')) {
        if (!part.empty()) {
            path_parts.push_back(part);
        }
    }

    FileData* current = &assets_folder;

    if (!path_parts.empty() && path_parts[0] != "Assets") {
        return &assets_folder;
    }

    for (size_t i = 0; i < path_parts.size(); ++i) {
        bool found = false;

        if (i == 0 && path_parts[i] == "Assets") continue;

        for (auto& child : current->children) {
            if (child.name == path_parts[i]) {
                current = &child;
                found = true;
                break;
            }
        }

        if (!found) return &assets_folder;
    }

    return current;
}

void FilesManager::CheckForRemovals(const std::unordered_map<std::string, fs::file_time_type>& current_files) {
    std::vector<std::string> to_remove;

    for (const auto& [old_path, _] : file_timestamps)
    {
        if (!current_files.count(old_path))
        {
            changed_files.push_back("[DELETED]" + old_path);
            to_remove.push_back(old_path);
        }
    }
}

void FilesManager::UpdateStructure()
{
    ProcessFromRoot();
}

void FilesManager::HandleDeletedItem(const std::string& path)
{
    ProcessFromRoot();

    ResourceManager::GetInstance().CleanUnusedResources();
}

void FilesManager::WatchFiles()
{
    std::unordered_map<std::string, fs::file_time_type> current_files;
    std::vector<std::string> new_changes;

    for (auto& entry : fs::recursive_directory_iterator("Assets"))
    {
        try
        {
            const auto path_str = entry.path().string();

            current_files[path_str] = entry.last_write_time();

            if (!file_timestamps.count(path_str))
            {
                new_changes.push_back(path_str);
            }
            else if (file_timestamps[path_str] != entry.last_write_time())
            {
                new_changes.push_back(path_str);
            }
        }
        catch (...) {}
    }

    CheckForRemovals(current_files);

    file_timestamps = std::move(current_files);
    changed_files.insert(changed_files.end(), new_changes.begin(), new_changes.end());

    ProcessChanges();
}

void FilesManager::ProcessChanges()
{
    if (changed_files.empty()) return;

    bool scripts_changed = false;
    bool resources_changed = false;

    for (const auto& path : changed_files)
    {
        if (path.starts_with("[DELETED]")) {
            const auto real_path = path.substr(9);
            HandleDeletedItem(real_path);
            continue;
        }

        std::string ext = std::filesystem::path(path).extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".cs") {
            scripts_changed = true;
        }
        else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".wav" ||
            ext == ".mp3" || ext == ".ogg" || ext == ".ttf" || ext == ".otf" ||
            ext == ".animation") {
            resources_changed = true;
        }
    }

    ProcessFromRoot();

    if (resources_changed)
    {
        ResourceManager::GetInstance().ReloadAll();
    }

    if (scripts_changed)
    {
        if (engine && engine->scripting_em)
        {
            engine->scripting_em->RecompileScripts();
        }
    }

    changed_files.clear();
    update_happened_this_frame = true;
}

std::string FilesManager::GetFileTypeByExtension(std::string ext)
{
    switch (DetermineFileType(ext))
    {
    case UNKNOWN:               return "";
        break;
    case FOLDER:                return "FOLDER";
        break;
    case IMAGE:                 return "IMAGE";
        break;
    case ANIMATION:             return "ANIMATION";
        break;
    case AUDIO:                 return "AUDIO";
        break;
    case FONT:                  return "FONT";
        break;
    case PARTICLES:             return "PARTICLES";
        break;
    case SCENE:                 return "SCENE";
        break;
    case SCRIPT:                return "SCRIPT";
        break;
    case TILED:                 return "TILED";
        break;
    case PREFAB:                return "PREFAB";
        break;
    case COUNT_FILE_TYPE:       return "";
        break;
    default:                    return "";
        break;
    }
    return "";
}

void FilesManager::ProcessDirectory(const std::filesystem::path& path, FileData& parent)
{
    std::vector<FileData> folders;
    std::vector<FileData> files;

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
            folders.push_back(child);
        }
        else
        {
            std::string extension = entry.path().extension().string();
            child.type = DetermineFileType(extension);
            files.push_back(child);
        }
    }

    for (const auto& item : folders)
        parent.children.push_back(item);
    folders.clear();

    for (const auto& item : files)
        parent.children.push_back(item);
    files.clear();
}

FileType FilesManager::DetermineFileType(const std::string& extension)
{
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == "") return FOLDER;
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
