#include "UIElement.h"

#include "GameObject.h"
#include "Canvas.h"

UIElement::UIElement(std::weak_ptr<GameObject> container_go, UIElementType type, std::string name, bool enable) :
	container_go(container_go),
	type(type),
	name(name),
	enabled(enable),
	uielement_id(GameObject::GenerateGameObjectID())
{
}

UIElement::UIElement(const UIElement& uielement_to_copy, std::shared_ptr<GameObject> container_go) :
	container_go(container_go),
	type(uielement_to_copy.type),
	name(uielement_to_copy.name),
	enabled(uielement_to_copy.enabled),
	uielement_id(GameObject::GenerateGameObjectID()),
	position_x(uielement_to_copy.position_x),
	position_y(uielement_to_copy.position_y),
	angle(uielement_to_copy.angle),
	scale_x(uielement_to_copy.scale_x),
	scale_y(uielement_to_copy.scale_y)
{
}
