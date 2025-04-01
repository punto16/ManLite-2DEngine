#include "ImageUI.h"

ImageUI::ImageUI(std::weak_ptr<Canvas> container_canvas, std::string name, bool enable) :
	UIElement(container_canvas, UIElementType::Image, name, enable)
{
}

ImageUI::ImageUI(const ImageUI& uielement_to_copy, std::shared_ptr<Canvas> container_canvas) :
	UIElement(uielement_to_copy, container_canvas)
{
}

ImageUI::~ImageUI()
{
}
