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
	IDLE						= 0,
	HOVERED						= 1,
	SELECTED					= 2,
	HOVEREDSELECTED				= 3,
	DISABLED					= 4,

	//
	UNKNOWN
};
class CheckBoxSectionManager
{
public:
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

	virtual void Draw();

	void ReloadTexture();

	//serialization
	virtual nlohmann::json SaveUIElement();
	virtual void LoadUIElement(const nlohmann::json& uielementJSON);

	// Getters/Setters
	std::string GetTexturePath() { return texture_path; }
	vec2f GetTextureSize() { return { tex_width, tex_height }; }
	bool IsPixelArt() { return pixel_art; }
	void SetIsPixelArt(bool pa) { pixel_art = pa; }

	CheckBoxSectionManager& GetSectionManager() { return section_manager; }

	void SwapTexture(std::string new_path);
	void UpdateCurrentSection();

	void SetValue(bool b) { value = b; }
	bool GetValue() { return value; }

private:
	CheckBoxSectionManager section_manager;

	std::string texture_path;
	GLuint textureID = 0;
	int tex_width, tex_height;
	bool pixel_art;

	std::future<GLuint> textureFuture;
	std::atomic<bool> textureLoading{ false };

	bool value = false;
};

#endif // !__CHECKBOXUI_H__