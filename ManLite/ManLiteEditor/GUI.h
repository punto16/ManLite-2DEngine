#ifndef __GUI_H__
#define __GUI_H__
#pragma once

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)

#include "Module.h"
#include "string"
#include <list>

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

private:
	std::list<Panel*> panels;
};

#endif // !__GUI_H__