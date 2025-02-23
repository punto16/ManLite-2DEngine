#ifndef __PANEL_GAME_H__
#define __PANEL_GAME_H__
#pragma once

#include "GuiPanel.h"

class PanelGame : public Panel
{
public:
	PanelGame(PanelType type, std::string name, bool enabled);
	~PanelGame();

	bool Update();

private:

};

#endif // !__PANEL_GAME_H__