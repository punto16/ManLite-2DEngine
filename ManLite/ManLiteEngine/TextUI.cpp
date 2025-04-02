#include "TextUI.h"

TextUI::TextUI(std::weak_ptr<GameObject> container_go, std::string fontPath, std::string name, bool enable) :
	UIElement(container_go, UIElementType::Text, name, enable),
	font_path(fontPath)
{
}

TextUI::TextUI(const TextUI& uielement_to_copy, std::shared_ptr<GameObject> container_go) :
	UIElement(uielement_to_copy, container_go)
{
}

TextUI::~TextUI()
{
}

void TextUI::Draw()
{
}

nlohmann::json TextUI::SaveUIElement()
{
	return nlohmann::json();
}

void TextUI::LoadUIElement(const nlohmann::json& uielementJSON)
{
}
