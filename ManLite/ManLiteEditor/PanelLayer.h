#ifndef __PANEL_LAYER_H__
#define __PANEL_LAYER_H__
#pragma once

#include "GuiPanel.h"

class PanelLayer : public Panel
{
public:
	PanelLayer(PanelType type, std::string name, bool enabled);
	~PanelLayer();

	bool Update();

private:

};

#endif // !__PANEL_LAYER_H__