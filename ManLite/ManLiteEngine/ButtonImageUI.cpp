#include "ButtonImageUI.h"

ButtonImageUI::ButtonImageUI(std::weak_ptr<Canvas> container_canvas, std::string name, bool enable) :
	UIElement(container_canvas, UIElementType::ButtonImage, name, enable)
{
}

ButtonImageUI::ButtonImageUI(const ButtonImageUI& uielement_to_copy, std::shared_ptr<Canvas> container_canvas) :
	UIElement(uielement_to_copy, container_canvas)
{
}

ButtonImageUI::~ButtonImageUI()
{
}
