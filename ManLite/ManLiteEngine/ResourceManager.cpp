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
        LOG(LogType::LOG_ERROR, "Failed to load texture: {}", path);
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, tex_width, tex_height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

Animation* ResourceManager::LoadAnimation(const std::string& key, const Animation& animation)
{
    if (key == "") return nullptr;
    auto it = animations.find(key);
    if (it != animations.end()) {
        it->second.refCount++;
        return &it->second.animation;
    }

    AnimationData newData;
    newData.animation = animation;
    newData.refCount = 1;
    animations[key] = newData;

    return &animations[key].animation;
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
}
