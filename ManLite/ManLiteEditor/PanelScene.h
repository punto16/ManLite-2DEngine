#ifndef __PANEL_SCENE_H__
#define __PANEL_SCENE_H__
#pragma once

#include "GuiPanel.h"


class SDL_Texture;

class PanelScene : public Panel
{
public:
	PanelScene(PanelType type, std::string name, bool enabled);
	~PanelScene();

	bool Draw();

private:
};

#endif // !__PANEL_SCENE_H__