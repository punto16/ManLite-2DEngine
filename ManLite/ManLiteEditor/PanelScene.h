#ifndef __PANEL_SCENE_H__
#define __PANEL_SCENE_H__
#pragma once

#include "GuiPanel.h"

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

class SDL_Texture;

class PanelScene : public Panel
{
public:
	PanelScene(PanelType type, std::string name, bool enabled);
	~PanelScene();

	void Start();
	bool Update();
	bool CleanUp();

	void SetCamSpeed(float newspeed) { this->cam_speed = newspeed; }
	float GetCamSpeed() const { return this->cam_speed; }

private:

	GLuint openglTextureID = 0;

	float cam_speed;
};

#endif // !__PANEL_SCENE_H__