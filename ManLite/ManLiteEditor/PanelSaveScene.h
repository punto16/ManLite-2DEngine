#ifndef __PANEL_SAVE_SCENE_H__
#define __PANEL_SAVE_SCENE_H__
#pragma once

#include "GuiPanel.h"
#include "atomic"
#include "future"
#include "memory"

class Scene;

enum SavePanelAction
{
	NEW_SCENE,
	OPEN_SCENE,
	CLOSE_APP
};

class PanelSaveScene : public Panel
{
public:
	PanelSaveScene(PanelType type, std::string name, bool enabled);
	~PanelSaveScene();

	bool Update();

	SavePanelAction save_panel_action = SavePanelAction::NEW_SCENE;
	std::future<void> loading_task;
	std::atomic<bool> is_loading{ false };
	std::atomic<bool> set_scene{ false };
	std::shared_ptr<Scene> new_scene;

	bool dragged_origin = false;
	std::string dragged_path = "";

private:
	int icon_image_id = -1;
	int w, h;

};

#endif // !__PANEL_SAVE_SCENE_H__