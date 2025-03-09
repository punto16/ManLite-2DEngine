#include "Transform.h"

Transform::Transform(std::weak_ptr<GameObject> container_go, std::string name, bool enable) :
	Component(container_go, ComponentType::Transform, name, enable),
	position(vec2f(0.0f, 0.0f)),
	angle_rotation(0.0f),
	scale(vec2f(1.0f, 1.0f))
{
}

Transform::Transform(const Transform& component_to_copy, std::shared_ptr<GameObject> container_go)
	: Component(component_to_copy, container_go),
	position(component_to_copy.position),
	angle_rotation(component_to_copy.angle_rotation),
	scale(component_to_copy.scale)
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
