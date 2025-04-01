#ifndef __SLIDERUI_H__
#define __SLIDERUI_H__
#pragma once

#include "UIElement.h"

#include <string>
#include <memory>
#include "nlohmann/json.hpp"
#include <future>
#include <atomic>

enum class SliderStyle
{
	ALL_EQUAL					= 0,
	FIRST_AND_LAST_DIFFERENT	= 1,

	//
	UNKOWN
};

enum class SliderState
{
	IDLE_TRUE					= 0,
	HOVERED_TRUE				= 1,
	DISABLED_TRUE				= 2,

	IDLE_FALSE					= 3,
	HOVERED_FALSE				= 4,
	DISABLED_FALSE				= 5,

	//
	UNKNOWN
};

class SliderSectionPart
{
	ML_Rect* current_section;

	ML_Rect section_idle_true;
	ML_Rect section_hovered_true;
	ML_Rect section_disabled_true;

	ML_Rect section_idle_false;
	ML_Rect section_hovered_false;
	ML_Rect section_disabled_false;
};

class SliderSectionManager
{
	SliderStyle slider_style;
	SliderState slider_state;

	SliderSectionPart regular_section_part;
	SliderSectionPart first_section_part;
	SliderSectionPart last_section_part;
};

class SliderUI : public UIElement
{
public:
	SliderUI(std::weak_ptr<Canvas> container_canvas, std::string texturePath = "", std::string name = "SliderUI", bool enable = true);
	SliderUI(const SliderUI& uielement_to_copy, std::shared_ptr<Canvas> container_canvas);
	virtual ~SliderUI();

	//virtual bool Init();
	//virtual bool Update(float dt);

	virtual void Draw();

	//virtual bool Pause();
	//virtual bool Unpause();

	////serialization
	//virtual nlohmann::json SaveUIElement();
	//virtual void LoadUIElement(const nlohmann::json& uielementJSON);

private:

	SliderSectionManager slider_section_manager;

	std::string texture_path;
	GLuint textureID = 0;
	int tex_width, tex_height;
	bool pixel_art;

	std::future<GLuint> textureFuture;
	std::atomic<bool> textureLoading{ false };

	int min_value = 0;
	int current_value = 3;
	int max_value = 3;

	float offset = 0;
	float offset_first = 0;
	float offset_last = 0;
};

#endif // !__SLIDERUI_H__