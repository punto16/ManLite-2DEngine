#include "UIElement.h"

#include "GameObject.h"
#include "Canvas.h"

UIElement::UIElement(std::weak_ptr<Canvas> container_canvas, UIElementType type, std::string name, bool enable) :
	container_canvas(container_canvas),
	type(type),
	name(name),
	enabled(enable),
	uielement_id(GameObject::GenerateGameObjectID())
{
}

UIElement::UIElement(const UIElement& uielement_to_copy, std::shared_ptr<Canvas> container_canvas) : 
	container_canvas(container_canvas),
	type(uielement_to_copy.type),
	name(uielement_to_copy.name),
	enabled(uielement_to_copy.enabled),
	uielement_id(GameObject::GenerateGameObjectID())
{
}
