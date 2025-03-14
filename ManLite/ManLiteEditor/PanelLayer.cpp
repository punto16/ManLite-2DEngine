#include "PanelLayer.h"

#include "GUI.h"
#include "EngineCore.h"
#include "SceneManagerEM.h"
#include "GameObject.h"
#include "Layer.h"
#include "Defs.h"

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
		for (const auto& layer : engine->scene_manager_em->GetCurrentScene().GetSceneLayers())
		{
			std::string layer_visible_label = std::string("##IsVisibleLayer" + std::to_string(layer->GetLayerID()));
			bool layer_visible = layer->IsVisible();
			ImGui::Checkbox(layer_visible_label.c_str(), &layer_visible);
			layer->SetVisible(layer_visible);
			ImGui::SameLine();
			std::string layer_header_label = std::string(layer->GetLayerName() + "  <id: " + std::to_string(layer->GetLayerID()) + ">");
			if (!layer_visible) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
			bool collapsing_header_layer = ImGui::CollapsingHeader(layer_header_label.c_str());
			if (!layer_visible) ImGui::PopStyleColor();
			if (collapsing_header_layer)
			{
				IterateChildren(*layer);
			}
		}
	}
	ImGui::End();

	return ret;
}

void PanelLayer::IterateChildren(Layer& layer)
{
	uint treeFlags = ImGuiTreeNodeFlags_Leaf;
	for (const auto& go : layer.GetChildren())
	{
		const bool is_disabled = !go->IsEnabled();
		if (is_disabled) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
		std::string tree_go_label = std::string(go->GetName() + "##in_layer" + std::to_string(go->GetID()));
		ImGui::TreeNodeEx(tree_go_label.c_str(), treeFlags);
		ImGui::TreePop();
		if (is_disabled) ImGui::PopStyleColor();

	}
}
