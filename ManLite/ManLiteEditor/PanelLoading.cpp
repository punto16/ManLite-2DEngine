#include "PanelLoading.h"

#include "App.h"
#include "GUI.h"
#include "PanelSaveScene.h"

#include <imgui.h>

PanelLoading::PanelLoading(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
}

PanelLoading::~PanelLoading()
{
}

bool PanelLoading::Update()
{
	if (bringToFront)
	{
		ImGui::SetNextWindowFocus();
		bringToFront = false;
	}

	bool ret = true;

	if (enabled && app->gui->save_scene_panel->is_loading)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, { 0.5, 0.5 });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 10, 10 });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5);
		ImGui::SetNextWindowSize({600, 200});
		ImGui::OpenPopup(text_to_display.c_str());
		if (ImGui::BeginPopupModal(text_to_display.c_str(), nullptr, ImGuiWindowFlags_NoResize))
		{
			ImGui::Text(description.c_str());
			ImGui::Dummy({0, 30});
			ImGui::SetNextItemWidth(580);
			ImGui::ProgressBar(-1.0f * (float)ImGui::GetTime(), ImVec2(0.0f, 0.0f), text_to_display.c_str());


			ImGui::EndPopup();
		}
		ImGui::PopStyleVar(2);
	}

	return ret;
}
