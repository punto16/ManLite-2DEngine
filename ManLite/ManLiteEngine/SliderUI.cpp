#include "SliderUI.h"

SliderUI::SliderUI(std::weak_ptr<Canvas> container_canvas, std::string name, bool enable) :
	UIElement(container_canvas, UIElementType::Slider, name, enable)
{
}

SliderUI::SliderUI(const SliderUI& uielement_to_copy, std::shared_ptr<Canvas> container_canvas) :
	UIElement(uielement_to_copy, container_canvas)
{
}

SliderUI::~SliderUI()
{
}
