#ifndef __PANEL_INSPECTOR_H__
#define __PANEL_INSPECTOR_H__
#pragma once

#include "GuiPanel.h"

class PanelInspector : public Panel
{
public:
	PanelInspector(PanelType type, std::string name, bool enabled);
	~PanelInspector();

	bool Update();

private:

};

#endif // !__PANEL_INSPECTOR_H__