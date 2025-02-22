#include "PanelScene.h"

#include "GUI.h"

#include <imgui.h>

PanelScene::PanelScene(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
}

PanelScene::~PanelScene()
{
}

bool PanelScene::Draw()
{
	bool ret = true;

	if (ImGui::Begin(name.c_str(), &enabled))
	{
		
	}
	ImGui::End();

	return ret;
}
