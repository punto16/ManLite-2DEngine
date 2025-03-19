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

    GLuint GetTextureID() const { return textureID; }
    const std::string& GetTexturePath() const { return texturePath; }

private:
    std::string texturePath;
    GLuint textureID = 0;
};

#endif // !__SPRITE2D_H__