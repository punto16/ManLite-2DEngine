#include "ResourceManager.h"
#include "Log.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

ResourceManager& ResourceManager::GetInstance() {
    static ResourceManager instance;
    return instance;
}

GLuint ResourceManager::LoadTexture(const std::string& path, int& tex_width, int& tex_height)
{
    if (path == "") return 0;
    auto it = textures.find(path);

    if (it != textures.end())
    {
        it->second.refCount++;
        tex_width = it->second.w;
        tex_height = it->second.h;
        return it->second.id;
    }

    int channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &tex_width, &tex_height, &channels, 0);

    if (!data) {
        LOG(LogType::LOG_ERROR, "Failed to load texture: %s", path.c_str());
        return 0;
    }

    GLuint textureID = -1;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, tex_width, tex_height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    TextureData newData;
    newData.id = textureID;
    newData.refCount = 1;
    newData.w = tex_width;
    newData.h = tex_height;
    textures[path] = newData;

    return textureID;
}

GLuint ResourceManager::GetTexture(const std::string& path) const
{
    if (path == "") return 0;
    auto it = textures.find(path);
    return (it != textures.end()) ? it->second.id : 0;
}

void ResourceManager::ReleaseTexture(const std::string& path)
{
    if (path == "") return;
    auto it = textures.find(path);
    if (it != textures.end() && --it->second.refCount <= 0)
    {
        glDeleteTextures(1, &it->second.id);
        textures.erase(it);
    }
}

Animation* ResourceManager::LoadAnimation(const std::string& path)
{
    if (path == "") return nullptr;
    auto it = animations.find(path);
    if (it != animations.end()) {
        it->second.refCount++;
        return &it->second.animation;
    }

    Animation newAnim;
    if (!newAnim.LoadFromFile(path)) {
        LOG(LogType::LOG_ERROR, "Failed to load animation: %s", path.c_str());
        return nullptr;
    }

    animations[path] = { newAnim, 1 };
    return &animations[path].animation;
}

Animation* ResourceManager::GetAnimation(const std::string& key)
{
    if (key == "") return nullptr;
    auto it = animations.find(key);
    return (it != animations.end()) ? &it->second.animation : nullptr;
}

void ResourceManager::ReleaseAnimation(const std::string& key)
{
    if (key == "") return;
    auto it = animations.find(key);
    if (it != animations.end())
        if (--it->second.refCount <= 0)
            animations.erase(it);
}

Mix_Chunk* ResourceManager::LoadSound(const std::string& path)
{
    if (path.empty()) return nullptr;
    auto it = sounds.find(path);
    if (it != sounds.end())
    {
        it->second.refCount++;
        return it->second.chunk;
    }

    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    if (!chunk)
    {
        LOG(LogType::LOG_ERROR, "Failed to load sound: %s", path.c_str());
        return nullptr;
    }

    sounds[path] = { chunk, 1 };
    return chunk;
}

void ResourceManager::ReleaseSound(const std::string& path)
{
    if (path.empty()) return;
    auto it = sounds.find(path);
    if (it != sounds.end() && --it->second.refCount <= 0)
    {
        Mix_FreeChunk(it->second.chunk);
        sounds.erase(it);
    }
}

// Audio: Music
Mix_Music* ResourceManager::LoadMusic(const std::string& path)
{
    if (path.empty()) return nullptr;
    auto it = musics.find(path);
    if (it != musics.end())
    {
        it->second.refCount++;
        return it->second.music;
    }

    Mix_Music* music = Mix_LoadMUS(path.c_str());
    if (!music)
    {
        LOG(LogType::LOG_ERROR, "Failed to load music: %s", path.c_str());
        return nullptr;
    }

    musics[path] = { music, 1 };
    return music;
}

void ResourceManager::ReleaseMusic(const std::string& path)
{
    if (path.empty()) return;
    auto it = musics.find(path);
    if (it != musics.end() && --it->second.refCount <= 0) {
        Mix_FreeMusic(it->second.music);
        musics.erase(it);
    }
}

std::future<GLuint> ResourceManager::LoadTextureAsync(const std::string& path, int& tex_width, int& tex_height)
{
    if (path.empty()) return {};

    auto it = textures.find(path);
    if (it != textures.end()) {
        tex_width = it->second.w;
        tex_height = it->second.h;
        std::promise<GLuint> p;
        p.set_value(it->second.id);
        return p.get_future();
    }

    int channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &tex_width, &tex_height, &channels, 0);

    if (!data) {
        LOG(LogType::LOG_ERROR, "Failed to load texture: %s", path.c_str());
        return {};
    }

    TextureLoadTask task;
    task.path = path;
    task.pixel_data = data;
    task.width = tex_width;
    task.height = tex_height;
    task.channels = channels;

    std::future<GLuint> future = task.promise.get_future();

    {
        std::lock_guard<std::mutex> lock(texture_queue_mutex);
        texture_load_queue.push(std::move(task));
    }

    return future;
}

void ResourceManager::ProcessTextures()
{
    {
        std::lock_guard<std::mutex> lock(texture_queue_mutex);

        while (!texture_load_queue.empty()) {
            TextureLoadTask task = std::move(texture_load_queue.front());
            texture_load_queue.pop();

            GLuint textureID;
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);

            GLenum format = (task.channels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_2D, 0, format, task.width, task.height, 0,
                format, GL_UNSIGNED_BYTE, task.pixel_data);
            glGenerateMipmap(GL_TEXTURE_2D);

            stbi_image_free(task.pixel_data);

            TextureData newData;
            newData.id = textureID;
            newData.refCount = 1;
            newData.w = task.width;
            newData.h = task.height;
            textures[task.path] = newData;

            task.promise.set_value(textureID);
        }
    }
}

void ResourceManager::CleanUnusedResources()
{
    for (auto it = textures.begin(); it != textures.end();) {
        if (it->second.refCount <= 0) {
            glDeleteTextures(1, &it->second.id);
            it = textures.erase(it);
        }
        else {
            ++it;
        }
    }
    
    for (auto it = animations.begin(); it != animations.end();) {
        if (it->second.refCount <= 0) {
            it = animations.erase(it);
        }
        else {
            ++it;
        }
    }

    for (auto it = sounds.begin(); it != sounds.end();) {
        if (it->second.refCount <= 0) {
            it = sounds.erase(it);
        }
        else {
            ++it;
        }
    }

    for (auto it = musics.begin(); it != musics.end();) {
        if (it->second.refCount <= 0) {
            it = musics.erase(it);
        }
        else {
            ++it;
        }
    }
}
