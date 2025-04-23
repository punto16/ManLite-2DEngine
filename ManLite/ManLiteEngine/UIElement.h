#ifndef __UIELEMENT_H__
#define __UIELEMENT_H__
#pragma once

#include "ResourceManager.h"

#include "Defs.h"
#include <string>
#include <memory>
#include "nlohmann/json.hpp"

class GameObject;

enum class UIElementType
{
	Image					= 0,
	ButtonImage				= 1,
	Slider					= 2,
	CheckBox				= 3,
	Text					= 4,

	//
	Unkown
};

class UIElement
{
public:
	UIElement(std::weak_ptr<GameObject> container_go, UIElementType type = UIElementType::Unkown, std::string name = "UIElement", bool enable = true);
	UIElement(const UIElement& uielement_to_copy, std::shared_ptr<GameObject> container_go);
	virtual ~UIElement() {}

	virtual bool Init() { return true; }
	virtual bool Update(float dt) { return true; }
	virtual void Draw() {}
	virtual bool Pause() { return true; }
	virtual bool Unpause() { return true; }

	virtual void ReloadTexture() {}

	static std::string UIElementTypeToString(UIElementType type)
	{
		switch (type)
		{
		case UIElementType::Image:			return "Image"; break;
		case UIElementType::ButtonImage:	return "ButtonImage"; break;
		case UIElementType::Slider:			return "Slider"; break;
		case UIElementType::CheckBox:		return "CheckBox"; break;
		case UIElementType::Text:			return "Text"; break;
		case UIElementType::Unkown:			return "Unknown"; break;
		default:							return "Unknown"; break;
		}
	}

	//serialization
	virtual nlohmann::json SaveUIElement() = 0;
	virtual void LoadUIElement(const nlohmann::json& uielementJSON) = 0;

	//getters // setters
	std::shared_ptr<GameObject> GetContainerGO() const { return container_go.lock(); }
	std::string GetName() const { return this->name; }
	void SetName(std::string name) { this->name = name; }
	UIElementType GetType() const { return this->type; }
	void SetType(UIElementType type) { this->type = type; }
	uint32_t GetID() const { return this->uielement_id; }

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

	void SetPosition(vec2f position) { position_x = position.x; position_y = position.y; }
	vec2f GetPosition() { return { position_x, position_y }; }
	void SetAngle(float angle_degree) { angle = angle_degree; }
	float GetAngle() { return angle; }
	void SetScale(vec2f scale) { scale_x = scale.x; scale_y = scale.y; }
	vec2f GetScale() { return { scale_x, scale_y }; }

	bool IsAspectLocked() const { return aspectLocked; }
	void SetAspectLocked(bool state) { aspectLocked = state; }
	float GetLockedAspectRatio() const { return lockedAspectRatio; }
	void SetLockedAspectRatio(float ratio) { lockedAspectRatio = ratio; }

protected:
	std::weak_ptr<GameObject> container_go;
	std::string name;
	UIElementType type;

	bool aspectLocked = false;
	float lockedAspectRatio = 1.0f;

	uint32_t uielement_id;
	bool enabled;

	//transform options
	float position_x = 0, position_y = 0;
	float angle;
	float scale_x = 1, scale_y = 1;
};

#endif // !__UIELEMENT_H__