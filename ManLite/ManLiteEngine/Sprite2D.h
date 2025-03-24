#ifndef __SPRITE2D_H__
#define __SPRITE2D_H__
#pragma once

#include "Component.h"
#include "ResourceManager.h"
#include "Transform.h"
#include "memory"

class Sprite2D : public Component {
public:
    Sprite2D(std::weak_ptr<GameObject> container_go, const std::string& texture_path, std::string name = "Sprite2D", bool enable = true);
    Sprite2D(const Sprite2D& component_to_copy, std::shared_ptr<GameObject> container_go);
    ~Sprite2D();

    void Draw() override;

    void SwapTexture(std::string new_path);

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

private:
    std::string texturePath;
    GLuint textureID = 0;
    int tex_width, tex_height;

    int sectionX = 0, sectionY = 0, sectionW = 0, sectionH = 0;

    float u1 = 0.0f, v1 = 0.0f, u2 = 1.0f, v2 = 1.0f;
};

#endif // !__SPRITE2D_H__