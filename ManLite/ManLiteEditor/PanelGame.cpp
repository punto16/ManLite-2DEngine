#include "PanelGame.h"

#include "GUI.h"

#include <imgui.h>

PanelGame::PanelGame(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
}

PanelGame::~PanelGame()
{
}

bool PanelGame::Draw()
{
	bool ret = true;

	if (ImGui::Begin(name.c_str(), &enabled))
	{

	}
	ImGui::End();

	return ret;
}
