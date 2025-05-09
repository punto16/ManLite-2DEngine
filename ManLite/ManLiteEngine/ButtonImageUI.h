#ifndef __BUTTONIMAGEUI_H__
#define __BUTTONIMAGEUI_H__
#pragma once

#include "UIElement.h"

#include <string>
#include <memory>
#include "nlohmann/json.hpp"
#include <future>
#include <atomic>

enum class ButtonState {
	IDLE					= 0,
	HOVERED					= 1,
	SELECTED				= 2,
	HOVEREDSELECTED			= 3,
	DISABLED				= 4,

	//
	UNKNOWN
};

class ButtonSectionManager
{
public:
	ML_Rect* current_section;
	ButtonState button_state;

	ML_Rect section_idle;
	ML_Rect section_hovered;
	ML_Rect section_selected;
	ML_Rect section_hovered_selected;
	ML_Rect section_disabled;
};

class ButtonImageUI : public UIElement
{
public:
	ButtonImageUI(std::weak_ptr<GameObject> container_go, std::string texturePath = "", std::string name = "ButtonImageUI", bool enable = true);
	ButtonImageUI(const ButtonImageUI& uielement_to_copy, std::shared_ptr<GameObject> container_go);
	virtual ~ButtonImageUI();

	virtual void Draw();

	void ReloadTexture();

	//serialization
	virtual nlohmann::json SaveUIElement();
	virtual void LoadUIElement(const nlohmann::json& uielementJSON);

	//getters // setters
	ButtonSectionManager& GetButtonSectionManager() { return button_section_manager; }

	std::string GetTexturePath() { return texture_path; }

	vec2f GetTextureSize() { return { tex_width, tex_height }; }
	bool IsPixelArt() { return pixel_art; }
	void SetIsPixelArt(bool pa) { pixel_art = pa; }

	void SwapTexture(std::string new_path);
	void UpdateCurrentTexture();

private:

	ButtonSectionManager button_section_manager;

	std::string texture_path;
	GLuint textureID = 0;
	int tex_width, tex_height;
	bool pixel_art;

	std::future<GLuint> textureFuture;
	std::atomic<bool> textureLoading{ false };
};

#endif // !__BUTTONIMAGEUI_H__