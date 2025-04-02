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
}

ButtonImageUI::ButtonImageUI(const ButtonImageUI& uielement_to_copy, std::shared_ptr<GameObject> container_go) :
	UIElement(uielement_to_copy, container_go),
	texture_path(uielement_to_copy.texture_path),
	pixel_art(uielement_to_copy.pixel_art)
{
	textureID = ResourceManager::GetInstance().LoadTexture(texture_path, tex_width, tex_height);
}

ButtonImageUI::~ButtonImageUI()
{
    ResourceManager::GetInstance().ReleaseTexture("Config\\placeholder.png");
    if (texture_path.empty()) return;
    ResourceManager::GetInstance().ReleaseTexture(texture_path);
}

void ButtonImageUI::Draw()
{
    if (!enabled) return;

    if (textureLoading && textureFuture.valid()) {
        if (textureFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            textureID = textureFuture.get();
            textureLoading = false;
            //SetTextureSection(sectionX, sectionY, sectionW, sectionH);
        }
    }

    if (textureID == 0) {
        static GLuint placeholder = ResourceManager::GetInstance().GetTexture("Config\\placeholder.png");
        engine->renderer_em->SubmitSprite(
            placeholder,
            GetContainerGO()->GetComponent<Transform>()->GetWorldMatrix(),
            0, 1, 1, 0,
            pixel_art
        );
        return;
    }

    if (auto transform = GetContainerGO()->GetComponent<Transform>())
    {
        vec2f scale = transform->GetScale();
        bool a_lock = transform->IsAspectRatioLocked();
        transform->SetAspectRatioLock(false);
        //transform->SetScale({ scale.x * sectionW / sectionH, scale.y });
        //engine->renderer_em->SubmitSprite(
        //    textureID,
        //    transform->GetWorldMatrix(),
        //    u1, v1, u2, v2,
        //    pixel_art
        //);
        transform->SetScale(scale);
        transform->SetAspectRatioLock(a_lock);
    }
}

nlohmann::json ButtonImageUI::SaveUIElement()
{
    return nlohmann::json();
}

void ButtonImageUI::LoadUIElement(const nlohmann::json& uielementJSON)
{
}
