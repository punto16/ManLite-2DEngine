#ifndef __PANEL_ANIMATION_H__
#define __PANEL_ANIMATION_H__
#pragma once

#include "GuiPanel.h"

class PanelAnimation : public Panel
{
public:
	PanelAnimation(PanelType type, std::string name, bool enabled);
	~PanelAnimation();

	bool Update();

private:

};

#endif // !__PANEL_ANIMATION_H__