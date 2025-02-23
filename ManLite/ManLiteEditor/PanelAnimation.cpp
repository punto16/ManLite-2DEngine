#include "PanelAnimation.h"

#include "GUI.h"

#include <imgui.h>

PanelAnimation::PanelAnimation(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
}

PanelAnimation::~PanelAnimation()
{
}

bool PanelAnimation::Update()
{
	bool ret = true;

	if (ImGui::Begin(name.c_str(), &enabled))
	{

	}
	ImGui::End();

	return ret;
}
