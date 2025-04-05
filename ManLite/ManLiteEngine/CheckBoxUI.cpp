#include "CheckBoxUI.h"
#include "EngineCore.h"
#include "RendererEM.h"
#include "Transform.h"
#include "GameObject.h"
#include "Canvas.h"
#include "Camera.h"

CheckBoxUI::CheckBoxUI(std::weak_ptr<GameObject> container_go, std::string texturePath, std::string name, bool enable)
    : UIElement(container_go, UIElementType::CheckBox, name, enable),
    texture_path(texturePath),
    value(false)
{

    ResourceManager::GetInstance().LoadTexture("Config\\placeholder.png", tex_width, tex_height);
    textureID = ResourceManager::GetInstance().LoadTexture(texturePath, tex_width, tex_height);

    // Initialize all sections
    section_manager.current_section = nullptr;
    section_manager.checkbox_state = CheckBoxState::IDLE;

    section_manager.section_idle_true =                 { 0,0,tex_width, tex_height };
    section_manager.section_hovered_true =              { 0,0,tex_width, tex_height };
    section_manager.section_selected_true =             { 0,0,tex_width, tex_height };
    section_manager.section_hovered_selected_true =     { 0,0,tex_width, tex_height };
    section_manager.section_disabled_true =             { 0,0,tex_width, tex_height };

    section_manager.section_idle_false =                { 0,0,tex_width, tex_height };
    section_manager.section_hovered_false =             { 0,0,tex_width, tex_height };
    section_manager.section_selected_false =            { 0,0,tex_width, tex_height };
    section_manager.section_hovered_selected_false =    { 0,0,tex_width, tex_height };
    section_manager.section_disabled_false =            { 0,0,tex_width, tex_height };
}

CheckBoxUI::CheckBoxUI(const CheckBoxUI& uielement_to_copy, std::shared_ptr<GameObject> container_go)
    : UIElement(uielement_to_copy, container_go),
    texture_path(uielement_to_copy.texture_path),
    pixel_art(uielement_to_copy.pixel_art),
    value(uielement_to_copy.value)
{

    textureID = ResourceManager::GetInstance().LoadTexture(texture_path, tex_width, tex_height);
    
    // Initialize all sections
    section_manager.current_section = nullptr;
    section_manager.checkbox_state = CheckBoxState::IDLE;

    section_manager.section_idle_true =                 uielement_to_copy.section_manager.section_idle_true;
    section_manager.section_hovered_true =              uielement_to_copy.section_manager.section_hovered_true;
    section_manager.section_selected_true =             uielement_to_copy.section_manager.section_selected_true;
    section_manager.section_hovered_selected_true =     uielement_to_copy.section_manager.section_hovered_selected_true;
    section_manager.section_disabled_true =             uielement_to_copy.section_manager.section_disabled_true;

    section_manager.section_idle_false =                uielement_to_copy.section_manager.section_idle_false;
    section_manager.section_hovered_false =             uielement_to_copy.section_manager.section_hovered_false;
    section_manager.section_selected_false =            uielement_to_copy.section_manager.section_selected_false;
    section_manager.section_hovered_selected_false =    uielement_to_copy.section_manager.section_hovered_selected_false;
    section_manager.section_disabled_false =            uielement_to_copy.section_manager.section_disabled_false;
}

CheckBoxUI::~CheckBoxUI() {
    ResourceManager::GetInstance().ReleaseTexture("Config\\placeholder.png");
    if (!texture_path.empty()) ResourceManager::GetInstance().ReleaseTexture(texture_path);
    if (section_manager.current_section != nullptr) RELEASE(section_manager.current_section);
}

void CheckBoxUI::Draw() {
    if (!enabled) return;
    UpdateCurrentSection();

    if (textureLoading && textureFuture.valid()) {
        if (textureFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            textureID = textureFuture.get();
            textureLoading = false;
        }
    }

    if (auto transform = GetContainerGO()->GetComponent<Transform>()) {
        vec2f scale = transform->GetScale();
        bool a_lock = transform->IsAspectRatioLocked();
        transform->SetAspectRatioLock(false);
        transform->SetScale({ 1, 1 });

        mat3f modelMat = transform->GetWorldMatrix();
        transform->SetScale(scale);
        transform->SetAspectRatioLock(a_lock);

        //fixed scale to addapt text to scene size
        vec2f o_scale_modification = { 1, 1 };
        vec2f scale_modification = o_scale_modification;

        //if go has camera, it will addapt to it
        if (auto cam = GetContainerGO()->GetComponent<Camera>())
        {
            int viewport_x, viewport_y, zoom;
            cam->GetViewportSize(viewport_x, viewport_y);
            zoom = cam->GetZoom();
            scale_modification = { scale_modification.x * 0.04705882352 * viewport_x / zoom, scale_modification.y * 0.08888888888 * viewport_y / zoom };
        }

        mat3f localMat = mat3f::CreateTransformMatrix(
            { this->position_x * (scale_modification.x / o_scale_modification.x), this->position_y * (scale_modification.y / o_scale_modification.y) },
            DEGTORAD * this->angle,
            { this->scale_x * section_manager.current_section->w / section_manager.current_section->h * scale_modification.x, this->scale_y * scale_modification.y }
        );
        mat3f finalMat = modelMat * localMat;

        ML_Rect uvs = Canvas::GetUVs(*section_manager.current_section, tex_width, tex_height);

        engine->renderer_em->SubmitSprite(
            textureID != 0 ? textureID : ResourceManager::GetInstance().GetTexture("Config\\placeholder.png"),
            finalMat,
            uvs.x, uvs.y, uvs.w, uvs.h,
            pixel_art
        );
    }
}

nlohmann::json CheckBoxUI::SaveUIElement() {
    nlohmann::json uielementJSON;

    // General UIElement properties
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

    // CheckBox specific properties
    uielementJSON["TexturePath"] = texture_path;
    uielementJSON["PixelArtRender"] = pixel_art;
    uielementJSON["TextureSize"] = { tex_width, tex_height };
    uielementJSON["CheckBoxState"] = section_manager.checkbox_state;

    // Sections
    auto AddSection = [&](const std::string& name, const ML_Rect& section) {
        uielementJSON[name] = { section.x, section.y, section.w, section.h };
        };

    AddSection("SectionIdleTrue", section_manager.section_idle_true);
    AddSection("SectionHoveredTrue", section_manager.section_hovered_true);
    AddSection("SectionSelectedTrue", section_manager.section_selected_true);
    AddSection("SectionHoveredSelectedTrue", section_manager.section_hovered_selected_true);
    AddSection("SectionDisabledTrue", section_manager.section_disabled_true);

    AddSection("SectionIdleFalse", section_manager.section_idle_false);
    AddSection("SectionHoveredFalse", section_manager.section_hovered_false);
    AddSection("SectionSelectedFalse", section_manager.section_selected_false);
    AddSection("SectionHoveredSelectedFalse", section_manager.section_hovered_selected_false);
    AddSection("SectionDisabledFalse", section_manager.section_disabled_false);

    return uielementJSON;
}

void CheckBoxUI::LoadUIElement(const nlohmann::json& uielementJSON) {
    // Load base properties
    if (uielementJSON.contains("UIElementID")) uielement_id = uielementJSON["UIElementID"];
    if (uielementJSON.contains("UIElementName")) name = uielementJSON["UIElementName"];
    if (uielementJSON.contains("UIElementType")) type = (UIElementType)uielementJSON["UIElementType"];
    if (uielementJSON.contains("UIElementEnabled")) enabled = uielementJSON["UIElementEnabled"];
    if (uielementJSON.contains("UIElementPosition")) {
        position_x = uielementJSON["UIElementPosition"][0];
        position_y = uielementJSON["UIElementPosition"][1];
    }
    if (uielementJSON.contains("UIElementAngle")) angle = uielementJSON["UIElementAngle"];
    if (uielementJSON.contains("UIElementScale")) {
        scale_x = uielementJSON["UIElementScale"][0];
        scale_y = uielementJSON["UIElementScale"][1];
    }
    if (uielementJSON.contains("UIElementAspectLocked")) aspectLocked = uielementJSON["UIElementAspectLocked"];
    if (uielementJSON.contains("UIElementAspectLockedRatio")) lockedAspectRatio = uielementJSON["UIElementAspectLockedRatio"];

    // Load CheckBox specific properties
    if (uielementJSON.contains("TexturePath")) {
        texture_path = uielementJSON["TexturePath"];
        textureLoading = true;
        textureFuture = ResourceManager::GetInstance().LoadTextureAsync(texture_path, tex_width, tex_height);
    }
    if (uielementJSON.contains("PixelArtRender")) pixel_art = uielementJSON["PixelArtRender"];
    if (uielementJSON.contains("TextureSize")) {
        tex_width = uielementJSON["TextureSize"][0];
        tex_height = uielementJSON["TextureSize"][1];
    }
    if (uielementJSON.contains("CheckBoxState")) section_manager.checkbox_state = (CheckBoxState)uielementJSON["CheckBoxState"];

    // Load sections
    auto LoadSection = [&](const std::string& name, ML_Rect& section) {
        if (uielementJSON.contains(name)) {
            section.x = uielementJSON[name][0];
            section.y = uielementJSON[name][1];
            section.w = uielementJSON[name][2];
            section.h = uielementJSON[name][3];
        }
        };

    LoadSection("SectionIdleTrue", section_manager.section_idle_true);
    LoadSection("SectionHoveredTrue", section_manager.section_hovered_true);
    LoadSection("SectionSelectedTrue", section_manager.section_selected_true);
    LoadSection("SectionHoveredSelectedTrue", section_manager.section_hovered_selected_true);
    LoadSection("SectionDisabledTrue", section_manager.section_disabled_true);

    LoadSection("SectionIdleFalse", section_manager.section_idle_false);
    LoadSection("SectionHoveredFalse", section_manager.section_hovered_false);
    LoadSection("SectionSelectedFalse", section_manager.section_selected_false);
    LoadSection("SectionHoveredSelectedFalse", section_manager.section_hovered_selected_false);
    LoadSection("SectionDisabledFalse", section_manager.section_disabled_false);
}

void CheckBoxUI::SwapTexture(std::string new_path) {
    if (!texture_path.empty()) ResourceManager::GetInstance().ReleaseTexture(texture_path);
    texture_path = new_path;
    textureID = ResourceManager::GetInstance().LoadTexture(texture_path, tex_width, tex_height);
}

void CheckBoxUI::UpdateCurrentSection() {
    switch (section_manager.checkbox_state) {
    case CheckBoxState::IDLE:                   section_manager.current_section = value ? &section_manager.section_idle_true : &section_manager.section_idle_false; break;
    case CheckBoxState::HOVERED:                section_manager.current_section = value ? &section_manager.section_hovered_true : &section_manager.section_hovered_false; break;
    case CheckBoxState::SELECTED:               section_manager.current_section = value ? &section_manager.section_selected_true : &section_manager.section_selected_false; break;
    case CheckBoxState::HOVEREDSELECTED:        section_manager.current_section = value ? &section_manager.section_hovered_selected_true : &section_manager.section_hovered_selected_false; break;
    case CheckBoxState::DISABLED:               section_manager.current_section = value ? &section_manager.section_disabled_true : &section_manager.section_disabled_false; break;
    default:
        section_manager.current_section = &section_manager.section_idle_false;
        break;
    }
}