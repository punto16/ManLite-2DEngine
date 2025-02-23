#include "PanelConsole.h"

#include "GUI.h"

#include <imgui.h>

PanelConsole::PanelConsole(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
}

PanelConsole::~PanelConsole()
{
}

bool PanelConsole::Update()
{
	bool ret = true;

	if (ImGui::Begin(name.c_str(), &enabled))
	{

	}
	ImGui::End();

	return ret;
}
