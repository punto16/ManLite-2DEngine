#ifndef __PANEL_HIERARCHY_H__
#define __PANEL_HIERARCHY_H__
#pragma once

#include "GuiPanel.h"

class PanelHierarchy : public Panel
{
public:
	PanelHierarchy(PanelType type, std::string name, bool enabled);
	~PanelHierarchy();

	bool Update();

private:

};

#endif // !__PANEL_HIERARCHY_H__