#ifndef __PANEL_LOADING_H__
#define __PANEL_LOADING_H__
#pragma once

#include "GuiPanel.h"
#include "string"

class PanelLoading : public Panel
{
public:
	PanelLoading(PanelType type, std::string name, bool enabled);
	~PanelLoading();

	bool Update();

	//setters //getters
	void SetText(std::string t) { this->text_to_display = t; }
	std::string GetText() { return text_to_display; }
	void SetDescription(std::string t) { this->description = t; }
	std::string GetDescription() { return description; }

private:
	std::string text_to_display = "Loading...";
	std::string description = "Loading Scene...";
};

#endif // !__PANEL_LOADING_H__