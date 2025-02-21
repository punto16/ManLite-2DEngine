#include "PanelHierarchy.h"

#include "GUI.h"

#include <imgui.h>

PanelHierarchy::PanelHierarchy(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
}

PanelHierarchy::~PanelHierarchy()
{
}

bool PanelHierarchy::Draw()
{
	bool ret = true;

	if (ImGui::Begin(name.c_str(), &enabled))
	{

	}
	ImGui::End();

	return ret;
}
