#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__
#pragma once

#include "Component.h"
#include "Defs.h"

#include <string>
#include <memory>

class GameObject;

class Transform : public Component
{
public:
	Transform(std::weak_ptr<GameObject> container_go, std::string name = "Transform", bool enable = true);
	Transform(const Transform& component_to_copy, std::shared_ptr<GameObject> container_go);
	~Transform();

	bool Update(float dt);


	//getters //setters

	vec2f GetPosition() const { return this->position; }
	float GetAngle() const { return this->angle_rotation; }
	vec2f GetScale() const { return this->scale; }

	vec2f GetWorldPosition();
	float GetWorldAngle();
	vec2f GetWorldScale();

private:
	vec2f position;
	//angle of rotation in degrees
	float angle_rotation;
	vec2f scale;
};

#endif // !__TRANSFORM_H__