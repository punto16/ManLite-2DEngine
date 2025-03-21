#include "Sprite2D.h"

#include "ResourceManagerEM.h"
#include "RendererEM.h"
#include "EngineCore.h"
#include "GameObject.h"

#include "Transform.h"
#include "mat3f.h"

Sprite2D::Sprite2D(std::weak_ptr<GameObject> container_go, const std::string& texture_path, std::string name, bool enable)
    : Component(container_go, ComponentType::Sprite, name, enable),
    texturePath(texture_path)
{
    textureID = ResourceManagerEM::GetInstance().LoadTexture(texturePath, tex_width, tex_height);
}

Sprite2D::Sprite2D(const Sprite2D& component_to_copy, std::shared_ptr<GameObject> container_go)
    : Component(component_to_copy, container_go),
    texturePath(component_to_copy.texturePath)
{
    textureID = ResourceManagerEM::GetInstance().LoadTexture(texturePath, tex_width, tex_height);
    this->tex_width = component_to_copy.tex_width;
    this->tex_height = component_to_copy.tex_height;
}

Sprite2D::~Sprite2D()
{
    if (texturePath.empty()) return;
    ResourceManagerEM::GetInstance().ReleaseTexture(texturePath);
}

void Sprite2D::Draw() {
    if (!enabled || textureID == 0) return;

    if (auto transform = GetContainerGO()->GetComponent<Transform>())
    {
        engine->renderer_em->SubmitSprite(textureID, transform->GetWorldMatrix());
    }
}

void Sprite2D::SwapTexture(std::string new_path)
{
    if (!texturePath.empty()) ResourceManagerEM::GetInstance().ReleaseTexture(texturePath);
    texturePath = new_path;
    textureID = ResourceManagerEM::GetInstance().LoadTexture(texturePath, tex_width, tex_height);
}