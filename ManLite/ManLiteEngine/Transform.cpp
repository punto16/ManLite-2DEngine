#include "Transform.h"

Transform::Transform(std::weak_ptr<GameObject> container_go, std::string name, bool enable) :
	Component(container_go, ComponentType::Transform, name, enable),
	position(vec2f(0.0f, 0.0f)),
	angle_rotation(0.0f),
	scale(vec2f(1.0f, 1.0f))
{
}

Transform::Transform(std::shared_ptr<Transform> component_to_copy, std::shared_ptr<GameObject> container_go) :
	Component(component_to_copy, container_go),
	position(component_to_copy->GetPosition()),
	angle_rotation(component_to_copy->GetAngle()),
	scale(component_to_copy->GetScale())
{
}

Transform::~Transform()
{
}

bool Transform::Update(float dt)
{
	bool ret = true;

	return ret;
}
