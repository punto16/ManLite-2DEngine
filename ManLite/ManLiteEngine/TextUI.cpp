#include "TextUI.h"

#include "ResourceManager.h"
#include "GameObject.h"
#include "Canvas.h"
#include "Transform.h"
#include "EngineCore.h"
#include "RendererEM.h"

#include "mat3f.h"

TextUI::TextUI(std::weak_ptr<GameObject> container_go, std::string fontPath, std::string name, bool enable) :
	UIElement(container_go, UIElementType::Text, name, enable),
	font_path(fontPath),
	text("Lorem Ipsum"),
    color({255, 255, 255, 255})
{
	font = ResourceManager::GetInstance().LoadFont(font_path, 24);
}

TextUI::TextUI(const TextUI& uielement_to_copy, std::shared_ptr<GameObject> container_go) :
	UIElement(uielement_to_copy, container_go),
	font_path(uielement_to_copy.font_path),
	text(uielement_to_copy.text),
    color(uielement_to_copy.color)
{
	font = ResourceManager::GetInstance().LoadFont(font_path, 24);
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

        //calculate resulting mat through -> deformation-less world mat * ui_element local mat
        mat3f localMat = mat3f::CreateTransformMatrix(
            { this->position_x, this->position_y },
            DEGTORAD * this->angle,
            { this->scale_x * 0.02, this->scale_y * 0.02 }
        );
        mat3f finalMat = modelMat * localMat;

        engine->renderer_em->SubmitText(text, font, finalMat, color);
    }
}

void TextUI::SwapFont(std::string new_font)
{
    if (font) ResourceManager::GetInstance().ReleaseFont(font_path);
    font_path = new_font;
    font = ResourceManager::GetInstance().LoadFont(font_path, 24);
}

nlohmann::json TextUI::SaveUIElement()
{
	return nlohmann::json();
}

void TextUI::LoadUIElement(const nlohmann::json& uielementJSON)
{
}