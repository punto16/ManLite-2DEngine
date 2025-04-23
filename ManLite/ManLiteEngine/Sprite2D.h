#ifndef __SPRITE2D_H__
#define __SPRITE2D_H__
#pragma once

#include "Component.h"
#include "ResourceManager.h"
#include "Transform.h"
#include "memory"
#include <future>
#include <atomic>

class Sprite2D : public Component {
public:
    Sprite2D(std::weak_ptr<GameObject> container_go, const std::string& texture_path, std::string name = "Sprite2D", bool enable = true);
    Sprite2D(const Sprite2D& component_to_copy, std::shared_ptr<GameObject> container_go);
    ~Sprite2D();

    void Draw() override;

    void SwapTexture(std::string new_path);
    void ReloadTexture();

    //serialization
    nlohmann::json SaveComponent() override;
    void LoadComponent(const nlohmann::json& componentJSON) override;

    //getters // setters
    GLuint GetTextureID() const { return textureID; }
    const std::string& GetTexturePath() const { return texturePath; }
    void GetTextureSize(int& tex_width, int& tex_height)
    {
        tex_width = this->tex_width;
        tex_height = this->tex_height;
    }

    void SetTextureSection(int x, int y, int w, int h);
    ML_Rect GetTextureSection() const { return ML_Rect(sectionX, sectionY, sectionW, sectionH); }
    void SetPixelArtRender(bool pixel_art) { this->pixel_art = pixel_art; }
    bool IsPixelArt() { return pixel_art; }

    void SetOffset(vec2f offset) { this->offset = offset; }
    vec2f GetOffset() { return offset; }

private:
    std::string texturePath;
    GLuint textureID = 0;
    int tex_width, tex_height;
    bool pixel_art;

    vec2f offset = { 0.0f, 0.0f };
    int sectionX = 0, sectionY = 0, sectionW = 0, sectionH = 0;

    float u1 = 0.0f, v1 = 0.0f, u2 = 1.0f, v2 = 1.0f;


    std::future<GLuint> textureFuture;
    std::atomic<bool> textureLoading{ false };
};

#endif // !__SPRITE2D_H__