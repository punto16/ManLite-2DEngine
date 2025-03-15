#ifndef __PANEL_LAYER_H__
#define __PANEL_LAYER_H__
#pragma once

#include "GuiPanel.h"

#include "unordered_map"

class Layer;

class PanelLayer : public Panel
{
public:
	PanelLayer(PanelType type, std::string name, bool enabled);
	~PanelLayer();

	bool Update();

	void IntroPart();
	void IterateChildren(Layer& go, bool visible);
	void BlankContext();

private:

	std::unordered_map<uint32_t, bool> collapsed_layers;

	bool request_uncollapse_all = false;
	bool request_collapse_all = false;
};

#endif // !__PANEL_LAYER_H__