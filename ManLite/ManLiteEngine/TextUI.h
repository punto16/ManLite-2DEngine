#ifndef __TEXTUI_H__
#define __TEXTUI_H__
#pragma once

#include "UIElement.h"

#include <string>
#include <memory>
#include "nlohmann/json.hpp"

class TextUI : public UIElement
{
public:
	TextUI(std::weak_ptr<Canvas> container_canvas, std::string name = "TextUI", bool enable = true);
	TextUI(const TextUI& uielement_to_copy, std::shared_ptr<Canvas> container_canvas);
	virtual ~TextUI();

	virtual bool Init();
	virtual bool Update(float dt);

	virtual void Draw();

	virtual bool Pause();
	virtual bool Unpause();

	//serialization
	virtual nlohmann::json SaveUIElement();
	virtual void LoadUIElement(const nlohmann::json& uielementJSON);

protected:
};

#endif // !__TEXTUI_H__