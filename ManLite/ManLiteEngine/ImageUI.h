#ifndef __IMAGEUI_H__
#define __IMAGEUI_H__
#pragma once

#include "UIElement.h"

#include <string>
#include <memory>
#include "nlohmann/json.hpp"
#include <future>
#include <atomic>

class ImageUI : public UIElement
{
public:
	ImageUI(std::weak_ptr<GameObject> container_go, std::string texturePath = "", std::string name = "ImageUI", bool enable = true);
	ImageUI(const ImageUI& uielement_to_copy, std::shared_ptr<GameObject> container_go);
	virtual ~ImageUI();

	virtual void Draw();

	void ReloadTexture();

	//serialization
	virtual nlohmann::json SaveUIElement();
	virtual void LoadUIElement(const nlohmann::json& uielementJSON);

	void SwapTexture(std::string new_path);

	//getters // setters
	ML_Rect GetSection() { return section_idle; }
	void SetSection(ML_Rect s) { section_idle = s; }
	
	std::string GetTexturePath() { return texture_path; }

	vec2f GetTextureSize() { return { tex_width, tex_height }; }
	bool IsPixelArt() { return pixel_art; }
	void SetIsPixelArt(bool pa) { pixel_art = pa; }

private:

	ML_Rect section_idle;

	std::string texture_path;
	GLuint textureID = 0;
	int tex_width, tex_height;
	bool pixel_art;

	std::future<GLuint> textureFuture;
	std::atomic<bool> textureLoading{ false };
};

#endif // !__IMAGEUI_H__