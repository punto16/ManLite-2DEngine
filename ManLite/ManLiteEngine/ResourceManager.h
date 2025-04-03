#ifndef __RESOURCE_MANAGER_EM_H__
#define __RESOURCE_MANAGER_EM_H__
#pragma once

#include "EngineModule.h"
#include "Animation.h"

#include <unordered_map>
#include <string>
#include <GL/glew.h>
#include "SDL2/SDL_mixer.h"

#include <ft2build.h>
#include FT_FREETYPE_H

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

struct FontCharacter {
    GLuint textureID;
    glm::ivec2 size;
    glm::ivec2 bearing;
    FT_Pos advance;
};

struct FontData {
    FT_Face face;
    int refCount = 0;
    std::map<FT_ULong, FontCharacter> characters;
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

    //fonts
    FontData* LoadFont(const std::string& path, int fontSize);
    FontData* GetFont(const std::string& path);
    void ReleaseFont(const std::string& path);
    FontCharacter* LoadFontCharacter(FontData* fontData, FT_ULong charCode);

    //thread management
    std::future<GLuint> LoadTextureAsync(const std::string& path, int& tex_width, int& tex_height);
    void ProcessTextures();

private:
    ResourceManager() = default;
    std::unordered_map<std::string, TextureData> textures;
    std::unordered_map<std::string, AnimationData> animations;
    std::unordered_map<std::string, SoundData> sounds;
    std::unordered_map<std::string, MusicData> musics;
    std::unordered_map<std::string, FontData> fonts;

    //thread management
    std::queue<TextureLoadTask> texture_load_queue;
    std::mutex texture_queue_mutex;
    std::mutex textures_mutex;
};

#endif // !__RESOURCE_MANAGER_EM_H__