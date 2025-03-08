#include "PanelLayer.h"

#include "GUI.h"

#include <imgui.h>

PanelLayer::PanelLayer(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
}

PanelLayer::~PanelLayer()
{
}

bool PanelLayer::Update()
{
	bool ret = true;

	if (ImGui::Begin(name.c_str(), &enabled))
	{

	}
	ImGui::End();

	return ret;
}
