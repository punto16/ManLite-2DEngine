#ifndef __RESOURCE_MANAGER_EM_H__
#define __RESOURCE_MANAGER_EM_H__
#pragma once

#include "EngineModule.h"
#include "Animation.h"

#include <unordered_map>
#include <string>
#include <GL/glew.h>

struct TextureData {
    GLuint id;
    int refCount = 0;
    int w, h;
};

struct AnimationData {
    Animation animation;
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

private:
    ResourceManager() = default;
    std::unordered_map<std::string, TextureData> textures;
    std::unordered_map<std::string, AnimationData> animations;
};

#endif // !__RESOURCE_MANAGER_EM_H__