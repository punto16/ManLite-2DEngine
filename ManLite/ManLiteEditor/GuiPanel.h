#ifndef __GUIPANEL_H__
#define __GUIPANEL_H__
#pragma once

#include <string>

enum class PanelType
{
	ABOUT,

	CONSOLE,
	HIERARCHY,
	INSPECTOR,
	PROJECT,
	SCENE,
	GAME,
	ANIMATION,
	RENDERER,
	SETTINGS,
	BUILD,
	LAYER,
	SAVE_SCENE,
	LOADING,

	UNKNOWN
};

class Panel
{
public:
	Panel(PanelType  type, std::string name, bool enabled) : type(type), name(name), enabled(enabled) {}
	virtual ~Panel() {}

	virtual void Start() {}
	virtual bool Update() = 0;
	virtual bool CleanUp() { return true; }

	virtual void OnSceneChange() {}

	virtual PanelType GetType() { return type; }
	virtual std::string GetName() { return name; }

	virtual bool GetState() { return enabled; }
	virtual void SetState(bool state) { this->enabled = state; }
	virtual void SwitchState() { enabled = !enabled; }

	void RequestFocus() { bringToFront = true; enabled = true; }
protected:

	PanelType type = PanelType::UNKNOWN;
	std::string name;
	bool enabled = false;

	bool bringToFront = false;
};

#endif // !__PANEL_H__