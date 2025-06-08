#include "Sprite2D.h"

#include "ResourceManager.h"
#include "RendererEM.h"
#include "EngineCore.h"
#include "GameObject.h"
#include "SceneManagerEM.h"

#include "Transform.h"
#include "mat3f.h"
#include "string"

Sprite2D::Sprite2D(std::weak_ptr<GameObject> container_go, const std::string& texture_path, std::string name, bool enable)
    : Component(container_go, ComponentType::Sprite, name, enable),
    texturePath(texture_path), pixel_art(false), offset({0.0f, 0.0f}),
    default_flip_horizontal(false),
    default_flip_vertical(false),
    flip_horizontal(false),
    flip_vertical(false)
{
    ResourceManager::GetInstance().LoadTexture("Config\\placeholder.png", tex_width, tex_height);//load placeholder
    if (std::this_thread::get_id() == engine->main_thread_id)
        textureID = ResourceManager::GetInstance().LoadTexture(texturePath, tex_width, tex_height);
    else
    {
        textureLoading = true;
        textureFuture = ResourceManager::GetInstance().LoadTextureAsync(texturePath, tex_width, tex_height);
    }
    SetTextureSection(0, 0, tex_width, tex_height);
}

Sprite2D::Sprite2D(const Sprite2D& component_to_copy, std::shared_ptr<GameObject> container_go)
    : Component(component_to_copy, container_go),
    texturePath(component_to_copy.texturePath),
    pixel_art(component_to_copy.pixel_art),
    offset(component_to_copy.offset),
    default_flip_horizontal(component_to_copy.default_flip_horizontal),
    default_flip_vertical(component_to_copy.default_flip_vertical),
    flip_horizontal(component_to_copy.flip_horizontal),
    flip_vertical(component_to_copy.flip_vertical)
{
    ResourceManager::GetInstance().LoadTexture("Config\\placeholder.png", tex_width, tex_height);//load placeholder
    if (std::this_thread::get_id() == engine->main_thread_id)
        textureID = ResourceManager::GetInstance().LoadTexture(texturePath, tex_width, tex_height);
    else
    {
        textureLoading = true;
        textureFuture = ResourceManager::GetInstance().LoadTextureAsync(texturePath, tex_width, tex_height);
    }

    this->tex_width = component_to_copy.tex_width;
    this->tex_height = component_to_copy.tex_height;
    SetTextureSection(component_to_copy.sectionX, component_to_copy.sectionY, component_to_copy.sectionW, component_to_copy.sectionH);
}

Sprite2D::~Sprite2D()
{
    ResourceManager::GetInstance().ReleaseTexture("Config\\placeholder.png");
    if (texturePath.empty()) return;
    ResourceManager::GetInstance().ReleaseTexture(texturePath);
}

void Sprite2D::Draw()
{
    if (!enabled) return;

    if (textureLoading && textureFuture.valid()) {
        if (textureFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            textureID = textureFuture.get();
            textureLoading = false;
            SetTextureSection(sectionX, sectionY, sectionW, sectionH);
        }
    }

    if (textureID == 0) {
        static GLuint placeholder = ResourceManager::GetInstance().GetTexture("Config\\placeholder.png");
        engine->renderer_em->SubmitSprite(
            placeholder,
            GetContainerGO()->GetComponent<Transform>()->GetWorldMatrix(),
            0, 1, 1, 0,
            pixel_art,
            engine->scene_manager_em->GetCurrentScene().GetGOOrderInLayer(container_go.lock()),
            0
        );
        return;
    }

    if (auto transform = GetContainerGO()->GetComponent<Transform>())
    {
        vec2f scale = transform->GetScale();
        bool a_lock = transform->IsAspectRatioLocked();

        transform->SetAspectRatioLock(false);
        transform->SetScale({ scale.x * sectionW / sectionH, scale.y });

        mat3f model_mat = transform->GetWorldMatrix();

        transform->SetScale(scale);
        transform->SetAspectRatioLock(a_lock);

        mat3f local_mat = mat3f::CreateTransformMatrix(
            offset,
            0,
            {1.0f, 1.0f}
        );

        model_mat = model_mat * local_mat;
        engine->renderer_em->SubmitSprite(
            textureID,
            model_mat,
            flip_horizontal     ? u2 : u1,
            flip_vertical       ? v2 : v1,
            flip_horizontal     ? u1 : u2,
            flip_vertical       ? v1 : v2,
            pixel_art,
            engine->scene_manager_em->GetCurrentScene().GetGOOrderInLayer(container_go.lock()),
            0.0f
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

void Sprite2D::ReloadTexture()
{
    bool resize_section = (sectionW == tex_width && sectionH == tex_height);
    ResourceManager::GetInstance().GetTexture(texturePath, tex_width, tex_height);
    if (resize_section) SetTextureSection(sectionX, sectionY, tex_width, tex_height);
    else SetTextureSection(sectionX, sectionY, sectionW, sectionH);
}

nlohmann::json Sprite2D::SaveComponent()
{
    nlohmann::json componentJSON;
    //component generic
    componentJSON["ContainerGOID"] = this->container_go.lock()->GetID();
    componentJSON["ComponentID"] = component_id;
    componentJSON["ComponentName"] = name;
    componentJSON["ComponentType"] = (int)type;
    componentJSON["Enabled"] = enabled;
    
    //component spcecific
    componentJSON["TexturePath"] = texturePath;
    componentJSON["PixelArtRender"] = pixel_art;
    componentJSON["TextureSize"] = { tex_width, tex_height };
    componentJSON["TextureSection"] = { sectionX, sectionY,sectionW, sectionH };
    componentJSON["TextureUVs"] = { u1, v1, u2, v2 };
    componentJSON["Offset"] = { offset.x, offset.y };
    componentJSON["FlipVertical"] = default_flip_vertical;
    componentJSON["FlipHorizontal"] = default_flip_horizontal;

    return componentJSON;
}

void Sprite2D::LoadComponent(const nlohmann::json& componentJSON)
{
    if (componentJSON.contains("ComponentID")) component_id = componentJSON["ComponentID"];
    if (componentJSON.contains("ComponentName")) name = componentJSON["ComponentName"];
    if (componentJSON.contains("ComponentType")) type = (ComponentType)componentJSON["ComponentType"];
    if (componentJSON.contains("Enabled")) enabled = componentJSON["Enabled"];
    
    if (componentJSON.contains("TexturePath"))
    {
        texturePath = componentJSON["TexturePath"];
        textureLoading = true;
        textureFuture = ResourceManager::GetInstance().LoadTextureAsync(texturePath, tex_width, tex_height);
    }
    if (componentJSON.contains("PixelArtRender")) pixel_art = componentJSON["PixelArtRender"];
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
    if (componentJSON.contains("Offset")) offset.x = componentJSON["Offset"][0];
    if (componentJSON.contains("Offset")) offset.y = componentJSON["Offset"][1];
    if (componentJSON.contains("FlipVertical")) default_flip_vertical = componentJSON["FlipVertical"];
    if (componentJSON.contains("FlipHorizontal")) default_flip_horizontal = componentJSON["FlipHorizontal"];

    flip_horizontal = default_flip_horizontal;
    flip_vertical = default_flip_vertical;
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
