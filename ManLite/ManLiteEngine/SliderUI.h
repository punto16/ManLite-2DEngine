#ifndef __SLIDERUI_H__
#define __SLIDERUI_H__
#pragma once

#include "UIElement.h"

#include <string>
#include <memory>
#include "nlohmann/json.hpp"
#include <future>
#include <atomic>

enum class SliderAlignment
{
    LEFT                        = 0,
    CENTER                      = 1,
    RIGHT                       = 2
};

enum class SliderStyle
{
	ALL_EQUAL					= 0,
	FIRST_AND_LAST_DIFFERENT	= 1,

	//
	UNKOWN
};

enum class SliderState
{
	IDLE						= 0,
	HOVERED						= 1,
	DISABLED					= 2,

	//
	UNKNOWN
};

class SliderSectionPart
{
public:
    ML_Rect* current_section_true = nullptr;
    ML_Rect* current_section_false = nullptr;

    ML_Rect section_idle_true;
    ML_Rect section_hovered_true;
    ML_Rect section_disabled_true;

    ML_Rect section_idle_false;
    ML_Rect section_hovered_false;
    ML_Rect section_disabled_false;

    void UpdateSections(SliderState state);
};

class SliderSectionManager
{
public:
    SliderStyle slider_style = SliderStyle::ALL_EQUAL;
    SliderState slider_state = SliderState::IDLE;

    SliderSectionPart regular_part;
    SliderSectionPart first_part;
    SliderSectionPart last_part;

    void UpdateAllSections();
};

class SliderUI : public UIElement {
public:
    SliderUI(std::weak_ptr<GameObject> container_go, std::string texturePath = "", std::string name = "SliderUI", bool enable = true);
    SliderUI(const SliderUI& uielement_to_copy, std::shared_ptr<GameObject> container_go);
    virtual ~SliderUI();

    virtual void Draw() override;

    void ReloadTexture();

    // Serialization
    virtual nlohmann::json SaveUIElement() override;
    virtual void LoadUIElement(const nlohmann::json& uielementJSON) override;

    // Getters/Setters
    void SetValue(int value);
    int GetValue() const { return current_value; }
    void SetRange(int min, int max);
    void SetOffsets(float regular, float first, float last);

    void SetAlignment(SliderAlignment new_alignment);
    SliderAlignment GetAlignment() const { return alignment; }


    SliderSectionPart& GetRegularPart() { return slider_manager.regular_part; }
    SliderSectionPart& GetFirstPart() { return slider_manager.first_part; }
    SliderSectionPart& GetLastPart() { return slider_manager.last_part; }

    SliderStyle GetSliderStyle() const { return slider_manager.slider_style; }
    void SetSliderStyle(SliderStyle style) { slider_manager.slider_style = style; }

    SliderState GetSliderState() const { return slider_manager.slider_state; }
    void SetSliderState(SliderState state) { slider_manager.slider_state = state; }

    void GetOffsets(float& main, float& first, float& last) const {
        main = offset;
        first = offset_first;
        last = offset_last;
    }

    void SwapTexture(const std::string& new_path);


    std::string GetTexturePath() { return texture_path; }

    bool IsPixelArt() { return pixel_art; }
    void SetIsPixelArt(bool b) { pixel_art = b; }

    int GetMinValue() { return min_value; }
    int GetMaxValue() { return max_value; }

    vec2 GetTextureSize() { return { tex_width, tex_height}; }
private:
    void CalculateDimensions();

    SliderSectionManager slider_manager;

    std::string texture_path;
    GLuint textureID = 0;
    int tex_width = 0;
    int tex_height = 0;
    bool pixel_art = false;

    SliderAlignment alignment = SliderAlignment::LEFT;

    std::future<GLuint> textureFuture;
    std::atomic<bool> textureLoading{ false };

    int min_value = 0;
    int current_value = 10;
    int max_value = 10;

    float element_width = 0.0f;
    float element_height = 0.0f;
    float offset = 0.0f;
    float offset_first = 0.0f;
    float offset_last = 0.0f;
};

#endif // !__SLIDERUI_H__