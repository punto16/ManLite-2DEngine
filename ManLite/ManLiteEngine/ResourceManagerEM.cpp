#include "ResourceManagerEM.h"
#include "Log.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

ResourceManagerEM::ResourceManagerEM(EngineCore* parent) : EngineModule(parent)
{
}

bool ResourceManagerEM::PostUpdate()
{
    bool ret = true;

    CleanUnusedResources();

    return ret;
}

ResourceManagerEM& ResourceManagerEM::GetInstance() {
    static ResourceManagerEM instance;
    return instance;
}

GLuint ResourceManagerEM::LoadTexture(const std::string& path)
{
    auto it = textures.find(path);

    if (it != textures.end()) {
        // Si existe, incrementar refCount y retornar ID
        it->second.refCount++;
        return it->second.id;
    }

    // 2. Cargar textura nueva
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (!data) {
        LOG(LogType::LOG_ERROR, "Failed to load texture: {}", path);
        return 0;
    }

    // 3. Crear textura OpenGL
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    // 4. Guardar en el mapa con refCount = 1
    TextureData newData;
    newData.id = textureID;
    newData.refCount = 1;
    textures[path] = newData;

    return textureID;
}

GLuint ResourceManagerEM::GetTexture(const std::string& path) const {
    auto it = textures.find(path);
    return (it != textures.end()) ? it->second.id : 0;
}

void ResourceManagerEM::ReleaseTexture(const std::string& path)
{
    auto it = textures.find(path);
    if (it != textures.end() && --it->second.refCount <= 0) {
        glDeleteTextures(1, &it->second.id);
        textures.erase(it);
    }
}

void ResourceManagerEM::CleanUnusedResources()
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
}
