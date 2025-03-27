#include "PanelProject.h"

#include "GUI.h"

#include <imgui.h>

PanelProject::PanelProject(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
}

PanelProject::~PanelProject()
{
}

bool PanelProject::Update()
{
	if (bringToFront)
	{
		ImGui::SetNextWindowFocus();
		bringToFront = false;
	}

	bool ret = true;

	if (ImGui::Begin(name.c_str(), &enabled))
	{

	}
	ImGui::End();

	return ret;
}
