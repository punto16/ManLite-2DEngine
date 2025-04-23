#ifndef __TEXTUI_H__
#define __TEXTUI_H__
#pragma once

#include "UIElement.h"
#include "RendererEM.h"

#include <string>
#include <memory>
#include "nlohmann/json.hpp"

class FontData;

class TextUI : public UIElement
{
public:
	TextUI(std::weak_ptr<GameObject> container_go, std::string fontPath = "", std::string name = "TextUI", bool enable = true);
	TextUI(const TextUI& uielement_to_copy, std::shared_ptr<GameObject> container_go);
	virtual ~TextUI();

	virtual void Draw();

	void SwapFont(std::string new_font);
	void ReloadTexture();

	//serialization
	virtual nlohmann::json SaveUIElement();
	virtual void LoadUIElement(const nlohmann::json& uielementJSON);

	//getters // setters
	std::string GetText() { return text; }
	void SetText(std::string t) { text = t; }

	std::string GetFontPath() { return font_path; }

	ML_Color GetColor() { return color; }
	void SetColor(ML_Color c) { color = c; }

	TextAlignment GetTextAlignment() { return text_alignment; }
	void SetTextAlignment(TextAlignment ta) { text_alignment = ta; }
	

private:

	std::string text = "Lorem Ipsum";
	std::string font_path;

	TextAlignment text_alignment;
	FontData* font;
	ML_Color color;
};

#endif // !__TEXTUI_H__