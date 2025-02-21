#include "PanelInspector.h"

#include "GUI.h"

#include <imgui.h>

PanelInspector::PanelInspector(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
}

PanelInspector::~PanelInspector()
{
}

bool PanelInspector::Draw()
{
	bool ret = true;

	if (ImGui::Begin(name.c_str(), &enabled))
	{

	}
	ImGui::End();

	return ret;
}
