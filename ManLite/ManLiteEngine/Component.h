#ifndef __COMPONENT_H__
#define __COMPONENT_H__
#pragma once

#include <string>
#include <memory>
#include "nlohmann/json.hpp"

class GameObject;

enum class ComponentType
{
	Transform				= 0,
	Camera					= 1,
	Sprite					= 2,
	Animator				= 3,
	Script					= 4,
	Collider2D				= 5,
	Canvas					= 6,
	AudioSource				= 7,
	ParticleSystem			= 8,


	Unkown
};

class Component
{
public:
	Component(std::weak_ptr<GameObject> container_go, ComponentType type = ComponentType::Unkown, std::string name = "Component", bool enable = true);
	Component(const Component& component_to_copy, std::shared_ptr<GameObject> container_go);
	virtual ~Component() {}

	virtual bool Init() { return true; }
	virtual bool Update(float dt) { return true; }
	virtual void Draw() {}

	static std::string ComponentTypeToString(ComponentType type)
	{
		switch (type)
		{
		case ComponentType::Transform:		return "Transform"; break;
		case ComponentType::Camera:			return "Camera"; break;
		case ComponentType::Sprite:			return "Sprite"; break;
		case ComponentType::Script:			return "Script"; break;
		case ComponentType::Collider2D:		return "Collider2D"; break;
		case ComponentType::Canvas:			return "Canvas"; break;
		case ComponentType::AudioSource:	return "AudioSource"; break;
		case ComponentType::ParticleSystem:	return "ParticleSystem"; break;
		case ComponentType::Unkown:			return "Unknown"; break;
		default:							return "Unknown"; break;
		}
	}

	//serialization
	virtual nlohmann::json SaveComponent() = 0;
	virtual void LoadComponent(const nlohmann::json& componentJSON) = 0;

	//getters // setters
	std::shared_ptr<GameObject> GetContainerGO() const { return container_go.lock(); }
	std::string GetName() const { return this->name; }
	void SetName(std::string name) { this->name = name; }
	ComponentType GetType() const { return this->type; }
	void SetType(ComponentType type) { this->type = type; }
	uint32_t GetID() const { return this->component_id; }

	bool IsEnabled() const { return this->enabled; }
	void SetEnabled(bool enable)
	{
		if (this->enabled)
		{
			this->enabled = false;
		}
		else if (!this->enabled)
		{
			this->enabled = true;
		}
	}
	void SwitchEnabled() { SetEnabled(!this->enabled); }

protected:
	std::weak_ptr<GameObject> container_go;
	std::string name;
	ComponentType type;

	uint32_t component_id;
	bool enabled;
};

#endif // !__COMPONENT_H__