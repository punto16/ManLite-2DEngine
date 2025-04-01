#ifndef __CHECKBOXUI_H__
#define __CHECKBOXUI_H__
#pragma once

#include "UIElement.h"

#include <string>
#include <memory>
#include "nlohmann/json.hpp"

class CheckBoxUI : public UIElement
{
public:
	CheckBoxUI(std::weak_ptr<Canvas> container_canvas, std::string name = "CheckBoxUI", bool enable = true);
	CheckBoxUI(const CheckBoxUI& uielement_to_copy, std::shared_ptr<Canvas> container_canvas);
	virtual ~CheckBoxUI();

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

#endif // !__CHECKBOXUI_H__