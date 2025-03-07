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

Component::Component(std::shared_ptr<Component> component_to_copy) :
	cointainer_go(component_to_copy->GetContainerGO()),
	type(component_to_copy->GetType()),
	name(component_to_copy->GetName()),
	enabled(component_to_copy->IsEnabled()),
	component_id(GameObject::GenerateGameObjectID())
{
}
