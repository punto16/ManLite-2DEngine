#ifndef __IMAGEUI_H__
#define __IMAGEUI_H__
#pragma once

#include "UIElement.h"

#include <string>
#include <memory>
#include "nlohmann/json.hpp"

class ImageUI : public UIElement
{
public:
	ImageUI(std::weak_ptr<Canvas> container_canvas, std::string name = "ImageUI", bool enable = true);
	ImageUI(const ImageUI& uielement_to_copy, std::shared_ptr<Canvas> container_canvas);
	virtual ~ImageUI();

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
};

#endif // !__IMAGEUI_H__