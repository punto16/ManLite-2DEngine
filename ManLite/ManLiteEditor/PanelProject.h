#ifndef __PANEL_PROJECT_H__
#define __PANEL_PROJECT_H__
#pragma once

#include "GuiPanel.h"

class PanelProject : public Panel
{
public:
	PanelProject(PanelType type, std::string name, bool enabled);
	~PanelProject();

	bool Draw();

private:

};

#endif // !__PANEL_PROJECT_H__