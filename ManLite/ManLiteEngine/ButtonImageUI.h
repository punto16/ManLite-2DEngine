#ifndef __BUTTONIMAGEUI_H__
#define __BUTTONIMAGEUI_H__
#pragma once

#include "UIElement.h"

#include <string>
#include <memory>
#include "nlohmann/json.hpp"

class ButtonImageUI : public UIElement
{
public:
	ButtonImageUI(std::weak_ptr<Canvas> container_canvas, std::string name = "ButtonImageUI", bool enable = true);
	ButtonImageUI(const ButtonImageUI& uielement_to_copy, std::shared_ptr<Canvas> container_canvas);
	virtual ~ButtonImageUI();

	virtual bool Init();
	virtual bool Update(float dt);

	virtual void Draw();

	virtual bool Pause();
	virtual bool Unpause();

	//serialization
	virtual nlohmann::json SaveUIElement();
	virtual void LoadUIElement(const nlohmann::json& uielementJSON);

private:

	ML_Rect section_idle;
	ML_Rect section_hovered;
	ML_Rect section_selected;
	ML_Rect section_hovered_selected;
	ML_Rect section_disabled;
};

#endif // !__BUTTONIMAGEUI_H__