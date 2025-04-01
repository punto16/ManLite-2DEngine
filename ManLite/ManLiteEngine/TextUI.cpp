#include "TextUI.h"

TextUI::TextUI(std::weak_ptr<Canvas> container_canvas, std::string name, bool enable) :
	UIElement(container_canvas, UIElementType::Text, name, enable)
{
}

TextUI::TextUI(const TextUI& uielement_to_copy, std::shared_ptr<Canvas> container_canvas) :
	UIElement(uielement_to_copy, container_canvas)
{
}

TextUI::~TextUI()
{
}
