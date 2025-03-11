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

vec2f Transform::GetWorldPosition()
{
	//make it world
	return position;
}

void Transform::SetWorldPosition(vec2f pos)
{
}

float Transform::GetWorldAngle()
{
	//make it world
	return angle_rotation;
}

void Transform::SetWorldAngle(float angle)
{
}

vec2f Transform::GetWorldScale()
{
	//make it world
	return scale;
}

void Transform::SetWorldScale(vec2f scale)
{
}
