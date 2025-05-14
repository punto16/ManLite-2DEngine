#include "SliderUI.h"
#include "EngineCore.h"
#include "RendererEM.h"
#include "Transform.h"
#include "GameObject.h"
#include "Canvas.h"
#include "Camera.h"
#include "ResourceManager.h"
#include "SceneManagerEM.h"

void SliderSectionPart::UpdateSections(SliderState state) {
    switch (state) {
    case SliderState::IDLE:
        current_section_true = &section_idle_true;
        current_section_false = &section_idle_false;
        break;
    case SliderState::HOVERED:
        current_section_true = &section_hovered_true;
        current_section_false = &section_hovered_false;
        break;
    case SliderState::DISABLED:
        current_section_true = &section_disabled_true;
        current_section_false = &section_disabled_false;
        break;
    default: break;
    }
}

void SliderSectionManager::UpdateAllSections() {
    regular_part.UpdateSections(slider_state);
    first_part.UpdateSections(slider_state);
    last_part.UpdateSections(slider_state);
}

SliderUI::SliderUI(std::weak_ptr<GameObject> container_go, std::string texturePath, std::string name, bool enable)
    : UIElement(container_go, UIElementType::Slider, name, enable),
    texture_path(texturePath) {

    ResourceManager::GetInstance().LoadTexture("Config\\placeholder.png", tex_width, tex_height);
    textureID = ResourceManager::GetInstance().LoadTexture(texturePath, tex_width, tex_height);
    CalculateDimensions();

    slider_manager.slider_state = SliderState::IDLE;
    slider_manager.slider_style = SliderStyle::ALL_EQUAL;

    //
    slider_manager.first_part.current_section_true = nullptr;
    slider_manager.first_part.current_section_false = nullptr;

    slider_manager.first_part.section_idle_true =           { 0,0,tex_width, tex_height };
    slider_manager.first_part.section_idle_false =          { 0,0,tex_width, tex_height };
    slider_manager.first_part.section_hovered_true =        { 0,0,tex_width, tex_height };
    slider_manager.first_part.section_hovered_false =       { 0,0,tex_width, tex_height };
    slider_manager.first_part.section_disabled_true =       { 0,0,tex_width, tex_height };
    slider_manager.first_part.section_disabled_false =      { 0,0,tex_width, tex_height };

    //
    slider_manager.regular_part.current_section_true = nullptr;
    slider_manager.regular_part.current_section_false = nullptr;

    slider_manager.regular_part.section_idle_true =         { 0,0,tex_width, tex_height };
    slider_manager.regular_part.section_idle_false =        { 0,0,tex_width, tex_height };
    slider_manager.regular_part.section_hovered_true =      { 0,0,tex_width, tex_height };
    slider_manager.regular_part.section_hovered_false =     { 0,0,tex_width, tex_height };
    slider_manager.regular_part.section_disabled_true =     { 0,0,tex_width, tex_height };
    slider_manager.regular_part.section_disabled_false =    { 0,0,tex_width, tex_height };

    //
    slider_manager.last_part.current_section_true = nullptr;
    slider_manager.last_part.current_section_false = nullptr;

    slider_manager.last_part.section_idle_true =            { 0,0,tex_width, tex_height };
    slider_manager.last_part.section_idle_false =           { 0,0,tex_width, tex_height };
    slider_manager.last_part.section_hovered_true =         { 0,0,tex_width, tex_height };
    slider_manager.last_part.section_hovered_false =        { 0,0,tex_width, tex_height };
    slider_manager.last_part.section_disabled_true =        { 0,0,tex_width, tex_height };
    slider_manager.last_part.section_disabled_false =       { 0,0,tex_width, tex_height };
}

SliderUI::SliderUI(const SliderUI& uielement_to_copy, std::shared_ptr<GameObject> container_go)
    : UIElement(uielement_to_copy, container_go),
    texture_path(uielement_to_copy.texture_path),
    pixel_art(uielement_to_copy.pixel_art),
    min_value(uielement_to_copy.min_value),
    current_value(uielement_to_copy.current_value),
    max_value(uielement_to_copy.max_value),
    offset(uielement_to_copy.offset),
    offset_first(uielement_to_copy.offset_first),
    offset_last(uielement_to_copy.offset_last) {

    ResourceManager::GetInstance().LoadTexture("Config\\placeholder.png", tex_width, tex_height);//load placeholder
    textureID = ResourceManager::GetInstance().LoadTexture(texture_path, tex_width, tex_height);
    CalculateDimensions();

    slider_manager.slider_state = uielement_to_copy.slider_manager.slider_state;
    slider_manager.slider_style = uielement_to_copy.slider_manager.slider_style;

    //
    slider_manager.first_part.current_section_true = nullptr;
    slider_manager.first_part.current_section_false = nullptr;

    slider_manager.first_part.section_idle_true =           uielement_to_copy.slider_manager.first_part.section_idle_true;
    slider_manager.first_part.section_idle_false =          uielement_to_copy.slider_manager.first_part.section_idle_false;
    slider_manager.first_part.section_hovered_true =        uielement_to_copy.slider_manager.first_part.section_hovered_true;
    slider_manager.first_part.section_hovered_false =       uielement_to_copy.slider_manager.first_part.section_hovered_false;
    slider_manager.first_part.section_disabled_true =       uielement_to_copy.slider_manager.first_part.section_disabled_true;
    slider_manager.first_part.section_disabled_false =      uielement_to_copy.slider_manager.first_part.section_disabled_false;

    //
    slider_manager.regular_part.current_section_true = nullptr;
    slider_manager.regular_part.current_section_false = nullptr;

    slider_manager.regular_part.section_idle_true =         uielement_to_copy.slider_manager.regular_part.section_idle_true;
    slider_manager.regular_part.section_idle_false =        uielement_to_copy.slider_manager.regular_part.section_idle_false;
    slider_manager.regular_part.section_hovered_true =      uielement_to_copy.slider_manager.regular_part.section_hovered_true;
    slider_manager.regular_part.section_hovered_false =     uielement_to_copy.slider_manager.regular_part.section_hovered_false;
    slider_manager.regular_part.section_disabled_true =     uielement_to_copy.slider_manager.regular_part.section_disabled_true;
    slider_manager.regular_part.section_disabled_false =    uielement_to_copy.slider_manager.regular_part.section_disabled_false;

    //
    slider_manager.last_part.current_section_true = nullptr;
    slider_manager.last_part.current_section_false = nullptr;

    slider_manager.last_part.section_idle_true =            uielement_to_copy.slider_manager.last_part.section_idle_true;
    slider_manager.last_part.section_idle_false =           uielement_to_copy.slider_manager.last_part.section_idle_false;
    slider_manager.last_part.section_hovered_true =         uielement_to_copy.slider_manager.last_part.section_hovered_true;
    slider_manager.last_part.section_hovered_false =        uielement_to_copy.slider_manager.last_part.section_hovered_false;
    slider_manager.last_part.section_disabled_true =        uielement_to_copy.slider_manager.last_part.section_disabled_true;
    slider_manager.last_part.section_disabled_false =       uielement_to_copy.slider_manager.last_part.section_disabled_false;
}

SliderUI::~SliderUI() {
    ResourceManager::GetInstance().ReleaseTexture("Config\\placeholder.png");
    if (!texture_path.empty()) ResourceManager::GetInstance().ReleaseTexture(texture_path);
}

void SliderUI::Draw() {
    if (!enabled || max_value <= 0) return;

    if (textureLoading && textureFuture.valid()) {
        if (textureFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            textureID = textureFuture.get();
            textureLoading = false;
        }
    }

    slider_manager.UpdateAllSections();

    if (auto transform = GetContainerGO()->GetComponent<Transform>())
    {
        //transform getter
        vec2f baseScale = transform->GetScale();
        bool aspectLock = transform->IsAspectRatioLocked();
        transform->SetAspectRatioLock(false);
        transform->SetScale({ 1, 1 });
        mat3f modelMat = transform->GetWorldMatrix();
        transform->SetScale(baseScale);
        transform->SetAspectRatioLock(aspectLock);

        //alineation
        float total_width = 0.0f;

        if (slider_manager.slider_style == SliderStyle::FIRST_AND_LAST_DIFFERENT) {
            total_width = offset_first + (max_value - 2) * (element_width + offset) + offset_last + element_width;
        }
        else {
            total_width = (element_width + offset) * (max_value - 1) + element_width;
        }

        float currentX = position_x;

        switch (alignment) {
        case SliderAlignment::CENTER:
            currentX -= total_width * 0.5f;
            break;
        case SliderAlignment::RIGHT:
            currentX -= total_width;
            break;
        case SliderAlignment::LEFT:
        default:
            break;
        }

        //actual drawing
        for (int i = 0; i < max_value; ++i) {
            const SliderSectionPart* part = &slider_manager.regular_part;

            if (slider_manager.slider_style == SliderStyle::FIRST_AND_LAST_DIFFERENT) {
                if (i == 0) part = &slider_manager.first_part;
                else if (i == max_value - 1) part = &slider_manager.last_part;
            }

            const ML_Rect* section = (i < current_value) ?
                part->current_section_true :
                part->current_section_false;

            if (!section) continue;

            if (i == max_value - 1 && slider_manager.slider_style == SliderStyle::FIRST_AND_LAST_DIFFERENT) {
                currentX += offset_last;
            }
            if (i > 0) {
                currentX += (i == 1 && slider_manager.slider_style == SliderStyle::FIRST_AND_LAST_DIFFERENT) ?
                    (offset + offset_first) : offset;
            }

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
                { currentX * (scale_modification.x / o_scale_modification.x), position_y * (scale_modification.y / o_scale_modification.y) },
                DEGTORAD * angle,
                { scale_x * section->w / section->h * scale_modification.x, scale_y * scale_modification.y }
            );

            ML_Rect uvs = Canvas::GetUVs(*section, tex_width, tex_height);
            engine->renderer_em->SubmitSprite(
                textureID != 0 ? textureID : ResourceManager::GetInstance().GetTexture("Config\\placeholder.png"),
                modelMat * localMat,
                uvs.x, uvs.y, uvs.w, uvs.h,
                pixel_art,
                engine->scene_manager_em->GetCurrentScene().GetGOOrderInLayer(container_go.lock()),
                container_go.lock()->GetComponent<Canvas>()->GetUIElementOrderPosition(this)
            );

            currentX += element_width;
        }
    }
}

void SliderUI::ReloadTexture()
{
    ResourceManager::GetInstance().GetTexture(texture_path, tex_width, tex_height);
}

nlohmann::json SliderUI::SaveUIElement() {
    nlohmann::json uielementJSON;

    // General UIElement properties
    uielementJSON["UIElementID"] = uielement_id;
    uielementJSON["UIElementName"] = name;
    uielementJSON["ContainerCanvasID"] = GetContainerGO()->GetComponent<Canvas>()->GetID();
    uielementJSON["UIElementType"] = type;
    uielementJSON["UIElementEnabled"] = enabled;
    uielementJSON["UIElementPosition"] = { position_x, position_y };
    uielementJSON["UIElementAngle"] = angle;
    uielementJSON["UIElementScale"] = { scale_x, scale_y };
    uielementJSON["UIElementAspectLocked"] = aspectLocked;
    uielementJSON["UIElementAspectLockedRatio"] = lockedAspectRatio;

    // slider specific properties
    uielementJSON["TexturePath"] = texture_path;
    uielementJSON["PixelArt"] = pixel_art;
    uielementJSON["CurrentValue"] = current_value;
    uielementJSON["MinValue"] = min_value;
    uielementJSON["MaxValue"] = max_value;
    uielementJSON["Offset"] = offset;
    uielementJSON["OffsetFirst"] = offset_first;
    uielementJSON["OffsetLast"] = offset_last;
    uielementJSON["Alignment"] = alignment;
    uielementJSON["SliderStyle"] = slider_manager.slider_style;
    uielementJSON["SliderState"] = slider_manager.slider_state;

    auto AddSection = [&](const std::string& name, const ML_Rect& section) {
        uielementJSON[name] = { section.x, section.y, section.w, section.h };
        };

    AddSection("FirstPartIdleTrueSection",              slider_manager.first_part.section_idle_true);
    AddSection("FirstPartIdleFalseSection",             slider_manager.first_part.section_idle_false);
    AddSection("FirstPartHoveredTrueSection",           slider_manager.first_part.section_hovered_true);
    AddSection("FirstPartHoveredFalseSection",          slider_manager.first_part.section_hovered_false);
    AddSection("FirstPartDisabledTrueSection",          slider_manager.first_part.section_disabled_true);
    AddSection("FirstPartDisabledFalseSection",         slider_manager.first_part.section_disabled_false);

    AddSection("RegularPartIdleTrueSection",            slider_manager.regular_part.section_idle_true);
    AddSection("RegularPartIdleFalseSection",           slider_manager.regular_part.section_idle_false);
    AddSection("RegularPartHoveredTrueSection",         slider_manager.regular_part.section_hovered_true);
    AddSection("RegularPartHoveredFalseSection",        slider_manager.regular_part.section_hovered_false);
    AddSection("RegularPartDisabledTrueSection",        slider_manager.regular_part.section_disabled_true);
    AddSection("RegularPartDisabledFalseSection",       slider_manager.regular_part.section_disabled_false);

    AddSection("LastPartIdleTrueSection",               slider_manager.last_part.section_idle_true);
    AddSection("LastPartIdleFalseSection",              slider_manager.last_part.section_idle_false);
    AddSection("LastPartHoveredTrueSection",            slider_manager.last_part.section_hovered_true);
    AddSection("LastPartHoveredFalseSection",           slider_manager.last_part.section_hovered_false);
    AddSection("LastPartDisabledTrueSection",           slider_manager.last_part.section_disabled_true);
    AddSection("LastPartDisabledFalseSection",          slider_manager.last_part.section_disabled_false);

    return uielementJSON;
}

void SliderUI::LoadUIElement(const nlohmann::json& uielementJSON)
{
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

    // Load slider specific properties
    if (uielementJSON.contains("TexturePath")) {
        texture_path = uielementJSON["TexturePath"];
        textureLoading = true;
        textureFuture = ResourceManager::GetInstance().LoadTextureAsync(texture_path, tex_width, tex_height);
    }

    if (uielementJSON.contains("PixelArt")) pixel_art = uielementJSON["PixelArt"];
    if (uielementJSON.contains("CurrentValue")) current_value = uielementJSON["CurrentValue"];
    if (uielementJSON.contains("MinValue")) min_value = uielementJSON["MinValue"];
    if (uielementJSON.contains("MaxValue")) max_value = uielementJSON["MaxValue"];
    if (uielementJSON.contains("Offset")) offset = uielementJSON["Offset"];
    if (uielementJSON.contains("OffsetFirst")) offset_first = uielementJSON["OffsetFirst"];
    if (uielementJSON.contains("OffsetLast")) offset_last = uielementJSON["OffsetLast"];

    if (uielementJSON.contains("Alignment"))
        alignment = static_cast<SliderAlignment>(uielementJSON["Alignment"]);

    if (uielementJSON.contains("SliderStyle"))
        slider_manager.slider_style = static_cast<SliderStyle>(uielementJSON["SliderStyle"]);

    if (uielementJSON.contains("SliderState"))
        slider_manager.slider_state = static_cast<SliderState>(uielementJSON["SliderState"]);

    auto LoadSection = [&](const std::string& name, ML_Rect& section) {
        if (uielementJSON.contains(name)) {
            section.x = uielementJSON[name][0];
            section.y = uielementJSON[name][1];
            section.w = uielementJSON[name][2];
            section.h = uielementJSON[name][3];
        }
        };

    LoadSection("FirstPartIdleTrueSection",              slider_manager.first_part.section_idle_true);
    LoadSection("FirstPartIdleFalseSection",             slider_manager.first_part.section_idle_false);
    LoadSection("FirstPartHoveredTrueSection",           slider_manager.first_part.section_hovered_true);
    LoadSection("FirstPartHoveredFalseSection",          slider_manager.first_part.section_hovered_false);
    LoadSection("FirstPartDisabledTrueSection",          slider_manager.first_part.section_disabled_true);
    LoadSection("FirstPartDisabledFalseSection",         slider_manager.first_part.section_disabled_false);

    LoadSection("RegularPartIdleTrueSection",            slider_manager.regular_part.section_idle_true);
    LoadSection("RegularPartIdleFalseSection",           slider_manager.regular_part.section_idle_false);
    LoadSection("RegularPartHoveredTrueSection",         slider_manager.regular_part.section_hovered_true);
    LoadSection("RegularPartHoveredFalseSection",        slider_manager.regular_part.section_hovered_false);
    LoadSection("RegularPartDisabledTrueSection",        slider_manager.regular_part.section_disabled_true);
    LoadSection("RegularPartDisabledFalseSection",       slider_manager.regular_part.section_disabled_false);

    LoadSection("LastPartIdleTrueSection",               slider_manager.last_part.section_idle_true);
    LoadSection("LastPartIdleFalseSection",              slider_manager.last_part.section_idle_false);
    LoadSection("LastPartHoveredTrueSection",            slider_manager.last_part.section_hovered_true);
    LoadSection("LastPartHoveredFalseSection",           slider_manager.last_part.section_hovered_false);
    LoadSection("LastPartDisabledTrueSection",           slider_manager.last_part.section_disabled_true);
    LoadSection("LastPartDisabledFalseSection",          slider_manager.last_part.section_disabled_false);
}

void SliderUI::SetValue(int value) {
    current_value = std::clamp(value, min_value, max_value);
}

void SliderUI::SetRange(int min, int max) {
    min_value = min;
    max_value = max;
    current_value = std::clamp(current_value, min_value, max_value);
    CalculateDimensions();
}

void SliderUI::SetOffsets(float regular, float first, float last) {
    offset = regular;
    offset_first = first;
    offset_last = last;
}

void SliderUI::SetAlignment(SliderAlignment new_alignment) {
    alignment = new_alignment;
}

void SliderUI::CalculateDimensions() {
    if (max_value > 0) {
        element_width = (slider_manager.regular_part.section_idle_true.w / tex_width) * scale_x;
        element_height = (slider_manager.regular_part.section_idle_true.h / tex_height) * scale_y;
    }
}

void SliderUI::SwapTexture(const std::string& new_path) {
    if (!texture_path.empty()) ResourceManager::GetInstance().ReleaseTexture(texture_path);
    texture_path = new_path;
    textureID = ResourceManager::GetInstance().LoadTexture(texture_path, tex_width, tex_height);
    CalculateDimensions();
}