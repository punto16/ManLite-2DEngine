#ifndef __PANEL_SCENE_H__
#define __PANEL_SCENE_H__
#pragma once

#include "GuiPanel.h"

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

class Grid;
class ImVec2;

class PanelScene : public Panel
{
public:
	PanelScene(PanelType type, std::string name, bool enabled);
	~PanelScene();

	void Start();
	bool Update();
	bool CleanUp();

	void InputToCamMovement();
	bool MouseHasTPed(int& current_mouse_x, int& current_mouse_y, int& prev_mouse_x, int& prev_mouse_y);

	void SetCamSpeed(float newspeed) { this->cam_speed = newspeed; }
	float GetCamSpeed() const { return this->cam_speed; }

	void ImGuizmoFunctionality(ImVec2 position, ImVec2 scale);
	void DrawTopBarControls();
private:

	GLuint openglTextureID = 0;
	float cam_speed;

	bool gizmoMode = 1;
	bool snapEnabled = true;
	float snapValue = 1.0f;
	int op;
};

#endif // !__PANEL_SCENE_H__