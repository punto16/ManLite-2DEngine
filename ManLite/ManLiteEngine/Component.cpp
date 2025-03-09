#include "Component.h"
#include "GameObject.h"

Component::Component(std::weak_ptr<GameObject> cointainer_go, ComponentType type, std::string name, bool enable) :
	cointainer_go(cointainer_go),
	type(type),
	name(name),
	enabled(enable),
	component_id(GameObject::GenerateGameObjectID())
{
}

Component::Component(const Component& component_to_copy, std::shared_ptr<GameObject> container_go)
	: cointainer_go(container_go),
	type(component_to_copy.type),
	name(component_to_copy.name),
	enabled(component_to_copy.enabled),
	component_id(GameObject::GenerateGameObjectID())
{
}
