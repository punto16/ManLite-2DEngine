#ifndef __GUI_H__
#define __GUI_H__
#pragma once

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)

#include "Module.h"
#include "string"
#include <list>
#include "vector"
#include "unordered_map"

class Panel;
class PanelHierarchy;
class PanelProject;
class PanelInspector;
class PanelScene;
class PanelGame;
class PanelConsole;
class PanelAnimation;
class PanelAbout;
class PanelLayer;
class PanelSaveScene;
class PanelLoading;
class PanelTileMap;

enum class Aspect
{
	A_FREE,
	A_16x9,
	A_21x9
};

class Gui : public Module
{
public:
	Gui(App* app);
	~Gui();

	bool Awake();
	bool Start();
	bool PreUpdate();
	bool Update(double dt);
	bool PostUpdate();
	bool CleanUp();

	//dockspace
	void MainWindowDockspace();

	//top main bar menus
	void MainMenuBar();
	void FileMenu();
	void EditMenu();
	void AssetsMenu();
	void GameObjectMenu();
	void ComponentMenu();
	void WindowMenu();
	void HelpMenu();

	void BuildPanel();
	void AutoSaveScenePanel();
	void AutoSaveSceneBg(float dt);
	void RestartAutoSaveTimer();

	//shortcut handler
	void HandleShortcut();

	//input control
	void HandleInput();
	void ProcessEvent();

	bool IsInitialized(Panel* panel);
	std::list<Panel*> GetPanels() { return panels; }

	static void HelpMarker(std::string text);

private:


public:
	//panels
	PanelHierarchy* hierarchy_panel = nullptr;
	PanelProject* project_panel = nullptr;
	PanelInspector* inspector_panel = nullptr;
	PanelScene* scene_panel = nullptr;
	PanelGame* game_panel = nullptr;
	PanelConsole* console_panel = nullptr;
	PanelAnimation* animation_panel = nullptr;
	PanelAbout* about_panel = nullptr;
	PanelLayer* layer_panel = nullptr;
	PanelSaveScene* save_scene_panel = nullptr;
	PanelLoading* loading_panel = nullptr;
	PanelTileMap* tile_map_panel = nullptr;

private:
	std::list<Panel*> panels;

	//
	bool show_demo_panel = false;

	// Auto-save
	float auto_save_timer = 0.0f;
	float countdown_timer = 0.0f;
	bool show_auto_save_panel = false;

	//build settings
	bool showBuildPanel = false;
	unsigned int icon_texture = 0;
	std::string icon_path = "";
	char app_name[128] = "";
	std::vector<std::string> scenes;
	std::unordered_map<std::string, bool> sceneInclusionMap;
	std::vector<std::string> includedScenesNames;
	std::string selectedMainScene = "";
	bool fullscreen = false;
	bool vsync = false;
};

#endif // !__GUI_H__