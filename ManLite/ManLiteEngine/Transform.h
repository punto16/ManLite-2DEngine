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
	void SetPosition(vec2f pos) { this->position = pos; }
	float GetAngle() const { return this->angle_rotation; }
	void SetAngle(float angle) { this->angle_rotation = angle; }
	vec2f GetScale() const { return this->scale; }
	void SetScale(vec2f scale) { this->scale = scale; }

	vec2f GetWorldPosition();
	void SetWorldPosition(vec2f pos);
	float GetWorldAngle();
	void SetWorldAngle(float angle);
	vec2f GetWorldScale();
	void SetWorldScale(vec2f scale);

private:
	vec2f position;
	//angle of rotation in degrees
	float angle_rotation;
	vec2f scale;
};

#endif // !__TRANSFORM_H__