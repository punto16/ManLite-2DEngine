#include "ButtonImageUI.h"

#include "EngineCore.h"
#include "RendererEM.h"
#include "Transform.h"
#include "GameObject.h"
#include "Canvas.h"

ButtonImageUI::ButtonImageUI(std::weak_ptr<GameObject> container_go, std::string texturePath, std::string name, bool enable) :
	UIElement(container_go, UIElementType::ButtonImage, name, enable),
	texture_path(texturePath),
	pixel_art(false)
{
	ResourceManager::GetInstance().LoadTexture("Config\\placeholder.png", tex_width, tex_height);//load placeholder
	textureID = ResourceManager::GetInstance().LoadTexture(texturePath, tex_width, tex_height);

    button_section_manager.current_section = nullptr;
    button_section_manager.button_state = ButtonState::IDLE;
    button_section_manager.section_idle =               { 0,0,tex_width, tex_height };
    button_section_manager.section_hovered =            { 0,0,tex_width, tex_height };
    button_section_manager.section_selected =           { 0,0,tex_width, tex_height };
    button_section_manager.section_hovered_selected =   { 0,0,tex_width, tex_height };
    button_section_manager.section_disabled =           { 0,0,tex_width, tex_height };
}

ButtonImageUI::ButtonImageUI(const ButtonImageUI& uielement_to_copy, std::shared_ptr<GameObject> container_go) :
	UIElement(uielement_to_copy, container_go),
	texture_path(uielement_to_copy.texture_path),
	pixel_art(uielement_to_copy.pixel_art)
{
	textureID = ResourceManager::GetInstance().LoadTexture(texture_path, tex_width, tex_height);

    button_section_manager.current_section = nullptr;
    button_section_manager.button_state = uielement_to_copy.button_section_manager.button_state;
    button_section_manager.section_idle = uielement_to_copy.button_section_manager.section_idle;
    button_section_manager.section_hovered = uielement_to_copy.button_section_manager.section_hovered;
    button_section_manager.section_selected = uielement_to_copy.button_section_manager.section_selected;
    button_section_manager.section_hovered_selected = uielement_to_copy.button_section_manager.section_hovered_selected;
    button_section_manager.section_disabled = uielement_to_copy.button_section_manager.section_disabled;
}

ButtonImageUI::~ButtonImageUI()
{
    ResourceManager::GetInstance().ReleaseTexture("Config\\placeholder.png");
    if (!texture_path.empty()) ResourceManager::GetInstance().ReleaseTexture(texture_path);
    if (button_section_manager.current_section != nullptr) RELEASE(button_section_manager.current_section)
}

void ButtonImageUI::Draw()
{
    if (!enabled) return;
    UpdateCurrentTexture();

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
            { this->scale_x * button_section_manager.current_section->w / button_section_manager.current_section->h, this->scale_y }
        );
        mat3f finalMat = modelMat * localMat;

        ML_Rect uvs = Canvas::GetUVs(*button_section_manager.current_section, tex_width, tex_height);

        engine->renderer_em->SubmitSprite(
            textureID != 0 ? textureID : ResourceManager::GetInstance().GetTexture("Config\\placeholder.png"),
            finalMat,
            uvs.x, uvs.y, uvs.w, uvs.h,
            pixel_art
        );
    }
}

nlohmann::json ButtonImageUI::SaveUIElement()
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
    uielementJSON["ButtonState"] = button_section_manager.button_state;
    uielementJSON["CurrentSection"] = { 
        button_section_manager.current_section->x, 
        button_section_manager.current_section->y, 
        button_section_manager.current_section->w, 
        button_section_manager.current_section->h };
    uielementJSON["SectionIdle"] = { 
        button_section_manager.section_idle.x, 
        button_section_manager.section_idle.y, 
        button_section_manager.section_idle.w, 
        button_section_manager.section_idle.h };
    uielementJSON["SectionHovered"] = { 
        button_section_manager.section_hovered.x, 
        button_section_manager.section_hovered.y, 
        button_section_manager.section_hovered.w, 
        button_section_manager.section_hovered.h };
    uielementJSON["SectionSelected"] = { 
        button_section_manager.section_selected.x, 
        button_section_manager.section_selected.y, 
        button_section_manager.section_selected.w, 
        button_section_manager.section_selected.h };
    uielementJSON["SectionHoveredSelected"] = { 
        button_section_manager.section_hovered_selected.x, 
        button_section_manager.section_hovered_selected.y, 
        button_section_manager.section_hovered_selected.w, 
        button_section_manager.section_hovered_selected.h };
    uielementJSON["SectionDisabled"] = { 
        button_section_manager.section_disabled.x, 
        button_section_manager.section_disabled.y, 
        button_section_manager.section_disabled.w, 
        button_section_manager.section_disabled.h };

    return uielementJSON;
}

void ButtonImageUI::LoadUIElement(const nlohmann::json& uielementJSON)
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
    if (uielementJSON.contains("ButtonState")) button_section_manager.button_state = (ButtonState)uielementJSON["ButtonState"];
    if (uielementJSON.contains("CurrentSection"))
    {
        button_section_manager.current_section->x = uielementJSON["CurrentSection"][0];
        button_section_manager.current_section->y = uielementJSON["CurrentSection"][1];
        button_section_manager.current_section->w = uielementJSON["CurrentSection"][2];
        button_section_manager.current_section->h = uielementJSON["CurrentSection"][3];
    }
    if (uielementJSON.contains("SectionIdle"))
    {
        button_section_manager.section_idle.x = uielementJSON["SectionIdle"][0];
        button_section_manager.section_idle.y = uielementJSON["SectionIdle"][1];
        button_section_manager.section_idle.w = uielementJSON["SectionIdle"][2];
        button_section_manager.section_idle.h = uielementJSON["SectionIdle"][3];
    }
    if (uielementJSON.contains("SectionHovered"))
    {
        button_section_manager.section_hovered.x = uielementJSON["SectionHovered"][0];
        button_section_manager.section_hovered.y = uielementJSON["SectionHovered"][1];
        button_section_manager.section_hovered.w = uielementJSON["SectionHovered"][2];
        button_section_manager.section_hovered.h = uielementJSON["SectionHovered"][3];
    }
    if (uielementJSON.contains("SectionSelected"))
    {
        button_section_manager.section_selected.x = uielementJSON["SectionSelected"][0];
        button_section_manager.section_selected.y = uielementJSON["SectionSelected"][1];
        button_section_manager.section_selected.w = uielementJSON["SectionSelected"][2];
        button_section_manager.section_selected.h = uielementJSON["SectionSelected"][3];
    }
    if (uielementJSON.contains("SectionHoveredSelected"))
    {
        button_section_manager.section_hovered_selected.x = uielementJSON["SectionHoveredSelected"][0];
        button_section_manager.section_hovered_selected.y = uielementJSON["SectionHoveredSelected"][1];
        button_section_manager.section_hovered_selected.w = uielementJSON["SectionHoveredSelected"][2];
        button_section_manager.section_hovered_selected.h = uielementJSON["SectionHoveredSelected"][3];
    }
    if (uielementJSON.contains("SectionDisabled"))
    {
        button_section_manager.section_disabled.x = uielementJSON["SectionDisabled"][0];
        button_section_manager.section_disabled.y = uielementJSON["SectionDisabled"][1];
        button_section_manager.section_disabled.w = uielementJSON["SectionDisabled"][2];
        button_section_manager.section_disabled.h = uielementJSON["SectionDisabled"][3];
    }
}

void ButtonImageUI::SwapTexture(std::string new_path)
{
    if (!texture_path.empty()) ResourceManager::GetInstance().ReleaseTexture(texture_path);
    texture_path = new_path;
    textureID = ResourceManager::GetInstance().LoadTexture(texture_path, tex_width, tex_height);
}

void ButtonImageUI::UpdateCurrentTexture()
{
    switch (button_section_manager.button_state)
    {
    case ButtonState::IDLE:                 button_section_manager.current_section = &button_section_manager.section_idle;              break;
    case ButtonState::HOVERED:              button_section_manager.current_section = &button_section_manager.section_hovered;           break;
    case ButtonState::SELECTED:             button_section_manager.current_section = &button_section_manager.section_selected;          break;
    case ButtonState::HOVEREDSELECTED:      button_section_manager.current_section = &button_section_manager.section_hovered_selected;  break;
    case ButtonState::DISABLED:             button_section_manager.current_section = &button_section_manager.section_disabled;          break;
    case ButtonState::UNKNOWN:
        break;
    default:
        break;
    }
}
