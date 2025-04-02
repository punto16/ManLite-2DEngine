#include "SliderUI.h"

SliderUI::SliderUI(std::weak_ptr<GameObject> container_go, std::string texturePath, std::string name, bool enable) :
	UIElement(container_go, UIElementType::Slider, name, enable),
	texture_path(texturePath)
{
}

SliderUI::SliderUI(const SliderUI& uielement_to_copy, std::shared_ptr<GameObject> container_go) :
	UIElement(uielement_to_copy, container_go)
{
}

SliderUI::~SliderUI()
{
}

void SliderUI::Draw()
{
}

nlohmann::json SliderUI::SaveUIElement()
{
	return nlohmann::json();
}

void SliderUI::LoadUIElement(const nlohmann::json& uielementJSON)
{
}
