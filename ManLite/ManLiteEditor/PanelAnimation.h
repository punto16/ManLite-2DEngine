#ifndef __PANEL_ANIMATION_H__
#define __PANEL_ANIMATION_H__
#pragma once

#include "GuiPanel.h"

#include "string"

class Animation;

class PanelAnimation : public Panel
{
public:
	PanelAnimation(PanelType type, std::string name, bool enabled);
	~PanelAnimation();

	bool Update();

	void DrawTopBarControls();
	void DrawImportedSprite();
	void DrawAnimatorControls();


	bool IsAnimationEmpty();
	void SetAnimation(std::string new_animation_PATH);
private:
	std::string sprite_path = "";
	unsigned int sprite = 0;
	int w = 0, h = 0;
	float image_size = 200.0f;

	std::string animation_path = "";
	Animation* animation = nullptr;

	bool animator_panel = false;
	bool animation_frame_panel = false;
	int selected_frame = -1;
};

#endif // !__PANEL_ANIMATION_H__