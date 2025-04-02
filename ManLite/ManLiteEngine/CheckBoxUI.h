#ifndef __CHECKBOXUI_H__
#define __CHECKBOXUI_H__
#pragma once

#include "UIElement.h"

#include <string>
#include <memory>
#include "nlohmann/json.hpp"
#include <future>
#include <atomic>

enum class CheckBoxState {
	IDLE_TRUE					= 0,
	HOVERED_TRUE				= 1,
	SELECTED_TRUE				= 2,
	HOVEREDSELECTED_TRUE		= 3,
	DISABLED_TRUE				= 4,

	IDLE_FALSE					= 5,
	HOVERED_FALSE				= 6,
	SELECTED_FALSE				= 7,
	HOVEREDSELECTED_FALSE		= 8,
	DISABLED_FALSE				= 9,

	//
	UNKNOWN
};
class CheckBoxSectionManager
{
	ML_Rect* current_section;
	CheckBoxState checkbox_state;

	ML_Rect section_idle_true;
	ML_Rect section_hovered_true;
	ML_Rect section_selected_true;
	ML_Rect section_hovered_selected_true;
	ML_Rect section_disabled_true;

	ML_Rect section_idle_false;
	ML_Rect section_hovered_false;
	ML_Rect section_selected_false;
	ML_Rect section_hovered_selected_false;
	ML_Rect section_disabled_false;
};

class CheckBoxUI : public UIElement
{
public:
	CheckBoxUI(std::weak_ptr<GameObject> container_go, std::string texturePath = "", std::string name = "CheckBoxUI", bool enable = true);
	CheckBoxUI(const CheckBoxUI& uielement_to_copy, std::shared_ptr<GameObject> container_go);
	virtual ~CheckBoxUI();

	//virtual bool Init();
	//virtual bool Update(float dt);

	virtual void Draw();

	//virtual bool Pause();
	//virtual bool Unpause();

	//serialization
	virtual nlohmann::json SaveUIElement();
	virtual void LoadUIElement(const nlohmann::json& uielementJSON);

private:
	CheckBoxSectionManager section_manager;

	std::string texture_path;
	GLuint textureID = 0;
	int tex_width, tex_height;
	bool pixel_art;

	std::future<GLuint> textureFuture;
	std::atomic<bool> textureLoading{ false };
};

#endif // !__CHECKBOXUI_H__