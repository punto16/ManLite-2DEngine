#ifndef __SPRITE2D_H__
#define __SPRITE2D_H__
#pragma once

#include "Component.h"
#include "ResourceManagerEM.h"
#include "Transform.h"

class Sprite2D : public Component {
public:
    Sprite2D(std::weak_ptr<GameObject> container_go, const std::string& texture_path, std::string name = "Sprite2D", bool enable = true);
    Sprite2D(const Sprite2D& component_to_copy, std::shared_ptr<GameObject> container_go);
    ~Sprite2D();

    void Draw() override;

    void SwapTexture(std::string new_path);

    GLuint GetTextureID() const { return textureID; }
    const std::string& GetTexturePath() const { return texturePath; }
    void GetTextureSize(int& tex_width, int& tex_height)
    {
        tex_width = this->tex_width;
        tex_height = this->tex_height;
    }

private:
    std::string texturePath;
    GLuint textureID = 0;
    int tex_width, tex_height;
};

#endif // !__SPRITE2D_H__