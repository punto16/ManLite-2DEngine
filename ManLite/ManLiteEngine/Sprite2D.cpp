#include "Sprite2D.h"

#include "ResourceManager.h"
#include "RendererEM.h"
#include "EngineCore.h"
#include "GameObject.h"

#include "Transform.h"
#include "mat3f.h"

Sprite2D::Sprite2D(std::weak_ptr<GameObject> container_go, const std::string& texture_path, std::string name, bool enable)
    : Component(container_go, ComponentType::Sprite, name, enable),
    texturePath(texture_path)
{
    textureID = ResourceManager::GetInstance().LoadTexture(texturePath, tex_width, tex_height);
    SetTextureSection(0, 0, tex_width, tex_height);
}

Sprite2D::Sprite2D(const Sprite2D& component_to_copy, std::shared_ptr<GameObject> container_go)
    : Component(component_to_copy, container_go),
    texturePath(component_to_copy.texturePath)
{
    textureID = ResourceManager::GetInstance().LoadTexture(texturePath, tex_width, tex_height);
    this->tex_width = component_to_copy.tex_width;
    this->tex_height = component_to_copy.tex_height;
    SetTextureSection(component_to_copy.sectionX, component_to_copy.sectionY, component_to_copy.sectionW, component_to_copy.sectionH);
}

Sprite2D::~Sprite2D()
{
    if (texturePath.empty()) return;
    ResourceManager::GetInstance().ReleaseTexture(texturePath);
}

void Sprite2D::Draw()
{
    if (!enabled || textureID == 0) return;

    if (auto transform = GetContainerGO()->GetComponent<Transform>()) {
        engine->renderer_em->SubmitSprite(
            textureID,
            transform->GetWorldMatrix(),
            u1, v1, u2, v2
        );
    }
}

void Sprite2D::SwapTexture(std::string new_path)
{
    if (!texturePath.empty()) ResourceManager::GetInstance().ReleaseTexture(texturePath);
    texturePath = new_path;
    textureID = ResourceManager::GetInstance().LoadTexture(texturePath, tex_width, tex_height);
    SetTextureSection(0, 0, tex_width, tex_height);
}

json Sprite2D::SaveComponent()
{
    json componentJSON;
    //component generic
    componentJSON["ContainerGOID"] = this->container_go.lock()->GetID();
    componentJSON["ComponentID"] = component_id;
    componentJSON["ComponentName"] = name;
    componentJSON["ComponentType"] = (int)type;
    componentJSON["Enabled"] = enabled;
    
    //component spcecific
    
    componentJSON["TexturePath"] = texturePath;
    componentJSON["TextureSize"] = { tex_width, tex_height };
    componentJSON["TextureSection"] = { sectionX, sectionY,sectionW, sectionH };
    componentJSON["TextureUVs"] = { u1, v1, u2, v2 };

    return componentJSON;
}

void Sprite2D::LoadComponent(const json& componentJSON)
{
    if (componentJSON.contains("ComponentID")) component_id = componentJSON["ComponentID"];
    if (componentJSON.contains("ComponentName")) name = componentJSON["ComponentName"];
    if (componentJSON.contains("ComponentType")) type = (ComponentType)componentJSON["ComponentType"];
    if (componentJSON.contains("Enabled")) enabled = componentJSON["Enabled"];
    
    if (componentJSON.contains("TexturePath")) texturePath = componentJSON["TexturePath"];
    SwapTexture(texturePath);
    if (componentJSON.contains("TextureSize")) tex_width = componentJSON["TextureSize"][0];
    if (componentJSON.contains("TextureSize")) tex_height = componentJSON["TextureSize"][1];
    if (componentJSON.contains("TextureSection")) sectionX = componentJSON["TextureSection"][0];
    if (componentJSON.contains("TextureSection")) sectionY = componentJSON["TextureSection"][1];
    if (componentJSON.contains("TextureSection")) sectionW = componentJSON["TextureSection"][2];
    if (componentJSON.contains("TextureSection")) sectionH = componentJSON["TextureSection"][3];
    if (componentJSON.contains("TextureUVs")) u1 = componentJSON["TextureUVs"][0];
    if (componentJSON.contains("TextureUVs")) v1 = componentJSON["TextureUVs"][1];
    if (componentJSON.contains("TextureUVs")) u2 = componentJSON["TextureUVs"][2];
    if (componentJSON.contains("TextureUVs")) v2 = componentJSON["TextureUVs"][3];


}

void Sprite2D::SetTextureSection(int x, int y, int w, int h) {
    sectionX = x;
    sectionY = y;
    sectionW = w;
    sectionH = h;

    if (tex_width > 0 && tex_height > 0)
    {
        u1 = static_cast<float>(x) / tex_width;
        u2 = static_cast<float>(x + w) / tex_width;
        v1 = static_cast<float>(tex_height - (y + h)) / tex_height;
        v2 = static_cast<float>(tex_height - y) / tex_height;
    }
}
