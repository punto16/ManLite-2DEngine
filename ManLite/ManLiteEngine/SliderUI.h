#ifndef __SLIDERUI_H__
#define __SLIDERUI_H__
#pragma once

#include "UIElement.h"

#include <string>
#include <memory>
#include "nlohmann/json.hpp"

class SliderUI : public UIElement
{
public:
	SliderUI(std::weak_ptr<Canvas> container_canvas, std::string name = "SliderUI", bool enable = true);
	SliderUI(const SliderUI& uielement_to_copy, std::shared_ptr<Canvas> container_canvas);
	virtual ~SliderUI();

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

#endif // !__SLIDERUI_H__