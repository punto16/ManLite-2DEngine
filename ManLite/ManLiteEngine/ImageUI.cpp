#include "ImageUI.h"

#include "EngineCore.h"
#include "RendererEM.h"
#include "Transform.h"
#include "GameObject.h"
#include "Canvas.h"

ImageUI::ImageUI(std::weak_ptr<GameObject> container_go, std::string texturePath, std::string name, bool enable) :
	UIElement(container_go, UIElementType::Image, name, enable),
	texture_path(texturePath),
    pixel_art(false)
{
	ResourceManager::GetInstance().LoadTexture("Config\\placeholder.png", tex_width, tex_height);//load placeholder
	textureID = ResourceManager::GetInstance().LoadTexture(texturePath, tex_width, tex_height);
    section_idle = {0, 0, tex_width, tex_height};
}

ImageUI::ImageUI(const ImageUI& uielement_to_copy, std::shared_ptr<GameObject> container_go) :
	UIElement(uielement_to_copy, container_go),
    texture_path(uielement_to_copy.texture_path),
    pixel_art(uielement_to_copy.pixel_art),
    section_idle(uielement_to_copy.section_idle)
{
	textureID = ResourceManager::GetInstance().LoadTexture(texture_path, tex_width, tex_height);
}

ImageUI::~ImageUI()
{
	ResourceManager::GetInstance().ReleaseTexture("Config\\placeholder.png");
	if (!texture_path.empty()) ResourceManager::GetInstance().ReleaseTexture(texture_path);
}

void ImageUI::Draw()
{
    if (!enabled) return;

    if (textureLoading && textureFuture.valid()) {
        if (textureFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            textureID = textureFuture.get();
            textureLoading = false;
        }
    }

    if (auto transform = GetContainerGO()->GetComponent<Transform>())
    {
        //get world mat without deformations
        vec2f scale = transform->GetScale();
        bool a_lock = transform->IsAspectRatioLocked();
        transform->SetAspectRatioLock(false);
        transform->SetScale({ 1, 1 });

        mat3f modelMat = transform->GetWorldMatrix();

        transform->SetScale(scale);
        transform->SetAspectRatioLock(a_lock);

        //calculate resulting mat through -> deformation-less world mat * ui_element local mat
        mat3f localMat = mat3f::CreateTransformMatrix(
            { this->position_x, this->position_y },
            DEGTORAD * this->angle,
            { this->scale_x * section_idle.w / section_idle.h, this->scale_y }
        );
        mat3f finalMat = modelMat * localMat;

        ML_Rect uvs = Canvas::GetUVs(section_idle, tex_width, tex_height);

        engine->renderer_em->SubmitSprite(
            textureID != 0 ? textureID : ResourceManager::GetInstance().GetTexture("Config\\placeholder.png"),
            finalMat,
            uvs.x, uvs.y, uvs.w, uvs.h,
            pixel_art
        );
    }
}

nlohmann::json ImageUI::SaveUIElement()
{
    nlohmann::json uielementJSON;

    //general uielement
    uielementJSON["UIElementID"] = uielement_id;
    uielementJSON["UIElementName"] = name;
    uielementJSON["CointainerCanvasID"] = GetContainerGO()->GetComponent<Canvas>()->GetID();
    uielementJSON["UIElementType"] = type;
    uielementJSON["UIElementEnabled"] = enabled;
    uielementJSON["UIElementPosition"] = { position_x, position_y };
    uielementJSON["UIElementAngle"] = angle;
    uielementJSON["UIElementScale"] = { scale_x, scale_y };
    uielementJSON["UIElementAspectLocked"] = aspectLocked;
    uielementJSON["UIElementAspectLockedRatio"] = lockedAspectRatio;

    //specific uielement
    uielementJSON["TexturePath"] = texture_path;
    uielementJSON["PixelArtRender"] = pixel_art;
    uielementJSON["TextureSize"] = { tex_width, tex_height };
    uielementJSON["SectionIdle"] = { section_idle.x, section_idle.y, section_idle.w, section_idle.h };

    return uielementJSON;
}

void ImageUI::LoadUIElement(const nlohmann::json& uielementJSON)
{
    if (uielementJSON.contains("UIElementID")) uielement_id = uielementJSON["UIElementID"];
    if (uielementJSON.contains("UIElementName")) name = uielementJSON["UIElementName"];
    if (uielementJSON.contains("UIElementType")) type = (UIElementType)uielementJSON["UIElementType"];
    if (uielementJSON.contains("UIElementEnabled")) enabled = uielementJSON["UIElementEnabled"];
    if (uielementJSON.contains("UIElementPosition")) position_x = uielementJSON["UIElementPosition"][0];
    if (uielementJSON.contains("UIElementPosition")) position_y = uielementJSON["UIElementPosition"][1];
    if (uielementJSON.contains("UIElementAngle")) angle = uielementJSON["UIElementAngle"];
    if (uielementJSON.contains("UIElementScale")) scale_x = uielementJSON["UIElementScale"][0];
    if (uielementJSON.contains("UIElementScale")) scale_y = uielementJSON["UIElementScale"][1];
    if (uielementJSON.contains("UIElementAspectLocked")) aspectLocked = uielementJSON["UIElementAspectLocked"];
    if (uielementJSON.contains("UIElementAspectLockedRatio")) lockedAspectRatio = uielementJSON["UIElementAspectLockedRatio"];

    //
    if (uielementJSON.contains("TexturePath"))
    {
        texture_path = uielementJSON["TexturePath"];
        textureLoading = true;
        textureFuture = ResourceManager::GetInstance().LoadTextureAsync(texture_path, tex_width, tex_height);
    }
    if (uielementJSON.contains("PixelArtRender")) pixel_art = uielementJSON["PixelArtRender"];
    if (uielementJSON.contains("TextureSize"))
    {
        tex_width = uielementJSON["TextureSize"][0];
        tex_height = uielementJSON["TextureSize"][1];
    }
    if (uielementJSON.contains("SectionIdle"))
    {
        section_idle.x = uielementJSON["SectionIdle"][0];
        section_idle.y = uielementJSON["SectionIdle"][1];
        section_idle.w = uielementJSON["SectionIdle"][2];
        section_idle.h = uielementJSON["SectionIdle"][3];
    }
}

void ImageUI::SwapTexture(std::string new_path)
{
    if (!texture_path.empty()) ResourceManager::GetInstance().ReleaseTexture(texture_path);
    texture_path = new_path;
    textureID = ResourceManager::GetInstance().LoadTexture(texture_path, tex_width, tex_height);
}
