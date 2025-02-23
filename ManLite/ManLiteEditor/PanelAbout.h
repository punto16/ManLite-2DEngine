#ifndef __PANEL_ABOUT_H__
#define __PANEL_ABOUT_H__
#pragma once

#include "GuiPanel.h"

class PanelAbout : public Panel
{
public:
	PanelAbout(PanelType type, std::string name, bool enabled);
	~PanelAbout();

	bool Update();

private:

};

#endif // !__PANEL_ABOUT_H__