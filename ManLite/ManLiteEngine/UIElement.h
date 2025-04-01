#ifndef __UIELEMENT_H__
#define __UIELEMENT_H__
#pragma once

#include "ResourceManager.h"

#include "Defs.h"
#include <string>
#include <memory>
#include "nlohmann/json.hpp"

class Canvas;

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
	UIElement(std::weak_ptr<Canvas> container_canvas, UIElementType type = UIElementType::Unkown, std::string name = "UIElement", bool enable = true);
	UIElement(const UIElement& uielement_to_copy, std::shared_ptr<Canvas> container_canvas);
	virtual ~UIElement() {}

	virtual bool Init() { return true; }
	virtual bool Update(float dt) { return true; }
	virtual void Draw() {}
	virtual bool Pause() { return true; }
	virtual bool Unpause() { return true; }

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
	std::shared_ptr<Canvas> GetContainerGO() const { return container_canvas.lock(); }
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

protected:
	std::weak_ptr<Canvas> container_canvas;
	std::string name;
	UIElementType type;

	uint32_t uielement_id;
	bool enabled;

	//trasnform options
	float position_x = 0, position_y = 0;
	float angle;
	float scale_x = 1, scale_y = 1;
};

#endif // !__UIELEMENT_H__