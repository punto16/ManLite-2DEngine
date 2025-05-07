#include "TextUI.h"

#include "ResourceManager.h"
#include "GameObject.h"
#include "Canvas.h"
#include "Transform.h"
#include "EngineCore.h"
#include "RendererEM.h"
#include "Camera.h"

#include "mat3f.h"

TextUI::TextUI(std::weak_ptr<GameObject> container_go, std::string fontPath, std::string name, bool enable) :
	UIElement(container_go, UIElementType::Text, name, enable),
	font_path(fontPath),
	text("Lorem Ipsum"),
    color({255, 255, 255, 255}),
    text_alignment(TextAlignment::TEXT_ALIGN_LEFT)
{
	font = ResourceManager::GetInstance().LoadFont(font_path, 512);
}

TextUI::TextUI(const TextUI& uielement_to_copy, std::shared_ptr<GameObject> container_go) :
	UIElement(uielement_to_copy, container_go),
	font_path(uielement_to_copy.font_path),
	text(uielement_to_copy.text),
    color(uielement_to_copy.color),
    text_alignment(uielement_to_copy.text_alignment)
{
	font = ResourceManager::GetInstance().LoadFont(font_path, 512);
}

TextUI::~TextUI()
{
	if (font) ResourceManager::GetInstance().ReleaseFont(font_path);
}

void TextUI::Draw()
{
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
        
        //fixed scale to addapt text to scene size
        vec2f o_scale_modification = { 0.005, 0.005 };
        vec2f scale_modification = o_scale_modification;

        //if go has camera, it will addapt to it
        if (auto cam = GetContainerGO()->GetComponent<Camera>())
        {
            int viewport_x, viewport_y, zoom;
            cam->GetViewportSize(viewport_x, viewport_y);
            zoom = cam->GetZoom();
            scale_modification = { scale_modification.x * 0.04705882352 * viewport_x / zoom, scale_modification.y * 0.08888888888 * viewport_y / zoom };
        }

        //calculate resulting mat through -> deformation-less world mat * ui_element local mat
        mat3f localMat = mat3f::CreateTransformMatrix(
            { this->position_x * (scale_modification.x / o_scale_modification.x), this->position_y * (scale_modification.y / o_scale_modification.y) },
            DEGTORAD * this->angle,
            { this->scale_x * scale_modification.x, this->scale_y * scale_modification.y }
        );

        mat3f finalMat = modelMat * localMat;

        engine->renderer_em->SubmitText(text, font, finalMat, color, text_alignment);
    }
}

void TextUI::SwapFont(std::string new_font)
{
    if (font) ResourceManager::GetInstance().ReleaseFont(font_path);
    font_path = new_font;
    font = ResourceManager::GetInstance().LoadFont(font_path, 512);
}

void TextUI::ReloadTexture()
{
    SwapFont(font_path);
}

nlohmann::json TextUI::SaveUIElement()
{
    nlohmann::json uielementJSON;

    //general uielement
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

    //specific uielement
    uielementJSON["Text"] = text;
    uielementJSON["FontPath"] = font_path;
    uielementJSON["Color"] = { color.r, color.g, color.b, color.a };
    uielementJSON["TextAlignment"] = text_alignment;

    return uielementJSON;
}

void TextUI::LoadUIElement(const nlohmann::json& uielementJSON)
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
    if (uielementJSON.contains("Text")) text = uielementJSON["Text"];
    if (uielementJSON.contains("FontPath"))
    {
        font_path = uielementJSON["FontPath"];
        font = ResourceManager::GetInstance().LoadFont(font_path, 512);
    }
    if (uielementJSON.contains("Color"))
    {
        color.r = uielementJSON["Color"][0];
        color.g = uielementJSON["Color"][1];
        color.b = uielementJSON["Color"][2];
        color.a = uielementJSON["Color"][3];
    }
    if (uielementJSON.contains("TextAlignment")) text_alignment = uielementJSON["TextAlignment"];
}