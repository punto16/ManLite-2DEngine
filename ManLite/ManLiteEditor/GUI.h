#ifndef __GUI_H__
#define __GUI_H__
#pragma once

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)

#include "Module.h"

#include <list>

class Panel;
class PanelHierarchy;
class PanelProject;
class PanelInspector;
class PanelScene;
class PanelGame;
class PanelConsole;

enum class Aspect
{
	A_FREE,
	A_16x9,
	A_21x9
};

//struct EditColor
//{
//	std::string id;
//	ImVec4 color;
//	ImVec4 previousColor;
//};

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

	//bool IsInitialized(Panel* panel);
	//std::list<Panel*> GetPanels();

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

	void HandleInput();
	void ProcessEvent();

	bool IsInitialized(Panel* panel);
	std::list<Panel*> GetPanels() { return panels; }

private:


public:
	//panels
	PanelHierarchy* hierarchy_panel = nullptr;
	PanelProject* project_panel = nullptr;
	PanelInspector* inspector_panel = nullptr;
	PanelScene* scene_panel = nullptr;
	PanelGame* game_panel = nullptr;
	PanelConsole* console_panel = nullptr;

private:
	std::list<Panel*> panels;
};

#endif // !__GUI_H__