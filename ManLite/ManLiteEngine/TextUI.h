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
	TextUI(std::weak_ptr<GameObject> container_go, std::string fontPath = "", std::string name = "TextUI", bool enable = true);
	TextUI(const TextUI& uielement_to_copy, std::shared_ptr<GameObject> container_go);
	virtual ~TextUI();

	virtual void Draw();

	//serialization
	virtual nlohmann::json SaveUIElement();
	virtual void LoadUIElement(const nlohmann::json& uielementJSON);

private:

	std::string text = "Lorem Ipsum";

	std::string font_path;

};

#endif // !__TEXTUI_H__