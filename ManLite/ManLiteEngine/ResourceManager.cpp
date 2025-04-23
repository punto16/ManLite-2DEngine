#include "ResourceManager.h"

#include "EngineCore.h"
#include "FontEM.h"
#include "SceneManagerEM.h"
#include "GameObject.h"
#include "Sprite2D.h"
#include "Canvas.h"
#include "UIElement.h"

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

GLuint ResourceManager::GetTexture(const std::string& path, int& tex_width, int& tex_height)
{
    if (path == "") return 0;
    auto it = textures.find(path);
    if (it != textures.end())
    {
        tex_width = it->second.w;
        tex_height = it->second.h;
        return it->second.id;
    }
    return 0;
}

void ResourceManager::ReleaseTexture(const std::string& path)
{
    if (path == "") return;
    std::lock_guard<std::mutex> lock(textures_mutex);
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

FontData* ResourceManager::LoadFont(const std::string& path, int fontSize) {
    auto it = fonts.find(path);
    if (it != fonts.end()) {
        it->second.refCount++;
        return &it->second;
    }

    FT_Library ft = engine->font_em->GetFreeType();

    FT_Face face;
    FT_Error error = FT_New_Face(ft, path.c_str(), 0, &face);
    if (error) {
        LOG(LogType::LOG_ERROR, "Failed to load font: %s (Error: %d)", path.c_str(), error);
        return nullptr;
    }

    error = FT_Set_Pixel_Sizes(face, 0, fontSize);
    if (error) {
        LOG(LogType::LOG_ERROR, "Failed to set font size for: %s", path.c_str());
        FT_Done_Face(face);
        return nullptr;
    }

    FontData newFontData;
    newFontData.face = face;
    newFontData.refCount = 1;
    fonts[path] = newFontData;

    if (!newFontData.face) {
        LOG(LogType::LOG_ERROR, "Failed to create font face");
        return nullptr;
    }

    LOG(LogType::LOG_OK, "Loaded font: %s (Size: %d)", path.c_str(), fontSize);
    return &fonts[path];
}

FontData* ResourceManager::GetFont(const std::string& path)
{
    if (path == "") return nullptr;
    auto it = fonts.find(path);
    return (it != fonts.end()) ? &it->second : nullptr;
}

void ResourceManager::ReleaseFont(const std::string& path)
{
    auto it = fonts.find(path);
    if (it != fonts.end())
    {
        if (--it->second.refCount <= 0)
        {
            FT_Done_Face(it->second.face);

            for (auto& [charCode, character] : it->second.characters)
            {
                glDeleteTextures(1, &character.textureID);
            }

            fonts.erase(it);
            LOG(LogType::LOG_OK, "Released font: %s", path.c_str());
        }
    }
}

FontCharacter* ResourceManager::LoadFontCharacter(FontData* fontData, FT_ULong charCode)
{
    if (fontData->characters.find(charCode) != fontData->characters.end()) {
        return &fontData->characters[charCode];
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    FT_Error error = FT_Load_Char(fontData->face, charCode, FT_LOAD_RENDER);
    if (error) {
        LOG(LogType::LOG_ERROR, "Failed to load glyph for character: %s", (char)charCode);
        return nullptr;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        fontData->face->glyph->bitmap.width,
        fontData->face->glyph->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        fontData->face->glyph->bitmap.buffer
    );


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    FontCharacter character = {
        textureID,
        glm::ivec2(fontData->face->glyph->bitmap.width, fontData->face->glyph->bitmap.rows),
        glm::ivec2(fontData->face->glyph->bitmap_left, fontData->face->glyph->bitmap_top),
        fontData->face->glyph->advance.x,
        charCode
    };

    fontData->characters[charCode] = character;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    return &character;
}

std::future<GLuint> ResourceManager::LoadTextureAsync(const std::string& path, int& tex_width, int& tex_height)
{
    if (path.empty()) return {};

    std::lock_guard<std::mutex> lock(textures_mutex);
    auto it = textures.find(path);
    if (it != textures.end()) {
        it->second.refCount++;
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
    std::lock_guard<std::mutex> lock(texture_queue_mutex);

    while (!texture_load_queue.empty())
    {
        TextureLoadTask task = std::move(texture_load_queue.front());
        texture_load_queue.pop();

        auto it = textures.find(task.path);
        if (it != textures.end()) {
            it->second.refCount++;
            stbi_image_free(task.pixel_data);
            task.promise.set_value(it->second.id);
            continue;
        }

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

void ResourceManager::ReloadAll()
{
    for (auto& [path, texData] : textures) {
        int w, h, channels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 0);
        if (!data) {
            LOG(LogType::LOG_ERROR, "Failed to reload texture: %s", path.c_str());
            continue;
        }

        glBindTexture(GL_TEXTURE_2D, texData.id);
        GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);

        texData.w = w;
        texData.h = h;
    }

    for (auto& [path, animData] : animations) {
        Animation newAnim;
        if (newAnim.LoadFromFile(path)) {
            animData.animation = std::move(newAnim);
        }
    }

    for (auto& [path, soundData] : sounds) {
        Mix_Chunk* newChunk = Mix_LoadWAV(path.c_str());
        if (newChunk) {
            Mix_FreeChunk(soundData.chunk);
            soundData.chunk = newChunk;
        }
    }

    for (auto& [path, musicData] : musics) {
        Mix_Music* newMusic = Mix_LoadMUS(path.c_str());
        if (newMusic) {
            Mix_FreeMusic(musicData.music);
            musicData.music = newMusic;
        }
    }

    for (auto& [path, fontData] : fonts)
    {
        FT_Face newFace;
        if (FT_New_Face(engine->font_em->GetFreeType(), path.c_str(), 0, &newFace)) continue;

        for (auto& [charCode, ch] : fontData.characters) {
            glDeleteTextures(1, &ch.textureID);
        }

        FT_Done_Face(fontData.face);
        fontData.face = newFace;
        fontData.characters.clear();
    }


    engine->scene_manager_em->GetCurrentScene().TraverseHierarchy([&](std::shared_ptr<GameObject> go) {
        if (auto sprite_component = go->GetComponent<Sprite2D>()) sprite_component->ReloadTexture();
        if (auto canvas_component = go->GetComponent<Canvas>())
            for (auto& ui_element : canvas_component->GetUIElements()) ui_element->ReloadTexture();
        });
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

    for (auto it = fonts.begin(); it != fonts.end();) {
        if (it->second.refCount <= 0) {
            FT_Done_Face(it->second.face);
            for (auto& [charCode, character] : it->second.characters) {
                glDeleteTextures(1, &character.textureID);
            }
            it = fonts.erase(it);
        }
        else {
            ++it;
        }
    }
}