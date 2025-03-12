#ifndef __PANEL_INSPECTOR_H__
#define __PANEL_INSPECTOR_H__
#pragma once

#include "GuiPanel.h"

class GameObject;

class PanelInspector : public Panel
{
public:
	PanelInspector(PanelType type, std::string name, bool enabled);
	~PanelInspector();

	bool Update();

	void GeneralOptions(GameObject& go);
	void PrefabOptions(GameObject& go);
	//components inpsector
	void TransformOptions(GameObject& go);


private:
	bool keep_transform_scale_proportions = true;
};

#endif // !__PANEL_INSPECTOR_H__