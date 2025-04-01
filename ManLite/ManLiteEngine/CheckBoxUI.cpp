#include "CheckBoxUI.h"

CheckBoxUI::CheckBoxUI(std::weak_ptr<Canvas> container_canvas, std::string name, bool enable) :
	UIElement(container_canvas, UIElementType::CheckBox, name, enable)
{
}

CheckBoxUI::CheckBoxUI(const CheckBoxUI& uielement_to_copy, std::shared_ptr<Canvas> container_canvas) :
	UIElement(uielement_to_copy, container_canvas)
{
}

CheckBoxUI::~CheckBoxUI()
{
}
