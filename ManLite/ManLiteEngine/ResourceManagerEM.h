#ifndef __RESOURCE_MANAGER_EM_H__
#define __RESOURCE_MANAGER_EM_H__
#pragma once

#include "EngineModule.h"

#include <unordered_map>
#include <string>
#include <GL/glew.h>

struct TextureData {
    GLuint id;
    int refCount = 0;
    int w, h;
};

class ResourceManagerEM : public EngineModule {
public:
    ResourceManagerEM(EngineCore* parent);

    bool PostUpdate();


    static ResourceManagerEM& GetInstance();

    GLuint LoadTexture(const std::string& path, int& tex_width, int& tex_height);
    GLuint GetTexture(const std::string& path) const;
    void ReleaseTexture(const std::string& path);


private:
    ResourceManagerEM() = default;
    std::unordered_map<std::string, TextureData> textures;

    void CleanUnusedResources();
};

#endif // !__RESOURCE_MANAGER_EM_H__