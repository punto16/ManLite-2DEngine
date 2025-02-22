#ifndef __PANEL_CONSOLE_H__
#define __PANEL_CONSOLE_H__
#pragma once

#include "GuiPanel.h"

class PanelConsole : public Panel
{
public:
	PanelConsole(PanelType type, std::string name, bool enabled);
	~PanelConsole();

	bool Draw();

private:

};

#endif // !__PANEL_CONSOLE_H__