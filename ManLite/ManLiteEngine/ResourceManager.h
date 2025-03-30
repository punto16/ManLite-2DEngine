#ifndef __RESOURCE_MANAGER_EM_H__
#define __RESOURCE_MANAGER_EM_H__
#pragma once

#include "EngineModule.h"
#include "Animation.h"

#include <unordered_map>
#include <string>
#include <GL/glew.h>
#include "SDL2/SDL_mixer.h"

//thread management
#include <queue>
#include <mutex>
#include <future>

//thread management
struct TextureLoadTask {
    std::string path;
    unsigned char* pixel_data;
    int width;
    int height;
    int channels;
    std::promise<GLuint> promise;
};

struct TextureData {
    GLuint id;
    int refCount = 0;
    int w, h;
};

struct AnimationData {
    Animation animation;
    int refCount = 0;
};

struct SoundData {
    Mix_Chunk* chunk = nullptr;
    int refCount = 0;
};

struct MusicData {
    Mix_Music* music = nullptr;
    int refCount = 0;
};

class ResourceManager {
public:
    //general
    static ResourceManager& GetInstance();
    void CleanUnusedResources();
    
    //textures
    GLuint LoadTexture(const std::string& path, int& tex_width, int& tex_height);
    GLuint GetTexture(const std::string& path) const;
    void ReleaseTexture(const std::string& path);

    //animations
    Animation* LoadAnimation(const std::string& path);
    Animation* GetAnimation(const std::string& key);
    void ReleaseAnimation(const std::string& key);

    // Audio
    Mix_Chunk* LoadSound(const std::string& path);
    void ReleaseSound(const std::string& path);
    Mix_Music* LoadMusic(const std::string& path);
    void ReleaseMusic(const std::string& path);

    //thread management
    std::future<GLuint> LoadTextureAsync(const std::string& path, int& tex_width, int& tex_height);
    void ProcessTextures();

private:
    ResourceManager() = default;
    std::unordered_map<std::string, TextureData> textures;
    std::unordered_map<std::string, AnimationData> animations;
    std::unordered_map<std::string, SoundData> sounds;
    std::unordered_map<std::string, MusicData> musics;

    //thread management
    std::queue<TextureLoadTask> texture_load_queue;
    std::mutex texture_queue_mutex;
    std::mutex textures_mutex;
};

#endif // !__RESOURCE_MANAGER_EM_H__