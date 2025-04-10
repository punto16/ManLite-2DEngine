#ifndef __PANEL_ANIMATION_H__
#define __PANEL_ANIMATION_H__
#pragma once

#include "GuiPanel.h"
#include "Defs.h"
#include "string"

class Animation;

class PanelAnimation : public Panel
{
public:
	PanelAnimation(PanelType type, std::string name, bool enabled);
	~PanelAnimation();

	bool Update();

	void DrawSpriteSection();
	void DrawStatusBar();
	void DrawFrameProperties();

	bool IsAnimationEmpty();
	void SetAnimation(std::string new_animation_PATH);

	static ML_Rect GetUVs(ML_Rect section, int w, int h);
	void SetSprite(std::string filePath);

private:
	std::string sprite_path = "";
	unsigned int sprite = 0;
	int w = 0, h = 0;

	std::string animation_path = "";
	Animation* animation = nullptr;
	float currentFrame;

	int selected_frame = -1;
};

#endif // !__PANEL_ANIMATION_H__