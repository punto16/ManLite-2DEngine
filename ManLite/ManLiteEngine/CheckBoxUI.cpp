#include "CheckBoxUI.h"

CheckBoxUI::CheckBoxUI(std::weak_ptr<GameObject> container_go, std::string texturePath, std::string name, bool enable) :
	UIElement(container_go, UIElementType::CheckBox, name, enable),
	texture_path(texturePath)
{
}

CheckBoxUI::CheckBoxUI(const CheckBoxUI& uielement_to_copy, std::shared_ptr<GameObject> container_go) :
	UIElement(uielement_to_copy, container_go)
{
}

CheckBoxUI::~CheckBoxUI()
{
}

void CheckBoxUI::Draw()
{
}

nlohmann::json CheckBoxUI::SaveUIElement()
{
	return nlohmann::json();
}

void CheckBoxUI::LoadUIElement(const nlohmann::json& uielementJSON)
{
}
