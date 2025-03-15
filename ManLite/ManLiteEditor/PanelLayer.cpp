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
        uint treeFlags = ImGuiTreeNodeFlags_OpenOnArrow;
        IntroPart();

        const bool had_global_request = request_collapse_all || request_uncollapse_all;

        if (had_global_request)
        {
            const bool collapse_state = request_collapse_all;
            for (const auto& layer : engine->scene_manager_em->GetCurrentScene().GetSceneLayers())
            {
                collapsed_layers[layer->GetLayerID()] = collapse_state;
            }
            request_collapse_all = request_uncollapse_all = false;
        }

        for (const auto& layer : engine->scene_manager_em->GetCurrentScene().GetSceneLayers())
        {
            const uint32_t layer_id = layer->GetLayerID();
            bool& is_collapsed = collapsed_layers[layer_id];

            std::string layer_visible_label = std::string("##IsVisibleLayer" + std::to_string(layer_id));
            bool layer_visible = layer->IsVisible();
            ImGui::Checkbox(layer_visible_label.c_str(), &layer_visible);
            layer->SetVisible(layer_visible);

            ImGui::SameLine();

            ImGui::SetNextItemOpen(!is_collapsed, had_global_request ? ImGuiCond_Always : ImGuiCond_Appearing);

            std::string layer_header_label = std::string(layer->GetLayerName() + "  <id: " + std::to_string(layer_id) + ">");
            if (!layer_visible) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);

            bool collapsing_header_layer = ImGui::CollapsingHeader(layer_header_label.c_str(), treeFlags);

            if (!layer_visible) ImGui::PopStyleColor();

            is_collapsed = !collapsing_header_layer;

            if (collapsing_header_layer)
            {
                IterateChildren(*layer, layer_visible);
            }
        }
        BlankContext();
    }
    ImGui::End();

    return ret;
}

void PanelLayer::IntroPart()
{
	if (ImGui::Button("Collapse All")) request_collapse_all = true;
	ImGui::SameLine();
	if (ImGui::Button("Uncollapse All")) request_uncollapse_all = true;
	ImGui::Separator();
}

void PanelLayer::IterateChildren(Layer& layer, bool visible)
{
	uint treeFlags = ImGuiTreeNodeFlags_Leaf;
	auto children = std::vector<std::shared_ptr<GameObject>>(layer.GetChildren());

	for (const auto& go : children)
	{
		ImGui::Indent(23);
		bool is_visible = go->IsVisible();
		std::string checkbox_go_label = std::string("##is_visible_go_in_layer" + std::to_string(go->GetID()));
		ImGui::Checkbox(checkbox_go_label.c_str(), &is_visible);
		ImGui::Unindent(23);
		go->SetVisible(is_visible);
		ImGui::SameLine();
		if (!visible || !is_visible) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
		std::string tree_go_label = std::string(go->GetName() + "##in_layer" + std::to_string(go->GetID()));
		ImGui::Indent(23);
		ImGui::TreeNodeEx(tree_go_label.c_str(), treeFlags);
		ImGui::Unindent(23);
		if (!visible || !is_visible) ImGui::PopStyleColor();
		ImGui::TreePop();
	}
	if (!children.empty()) ImGui::Separator();
}

void PanelLayer::BlankContext()
{
	if (ImGui::BeginPopupContextWindow())
	{
		if (ImGui::MenuItem("Create Layer"))
		{
			engine->scene_manager_em->GetCurrentScene().CreateEmptyLayer();
		}
		ImGui::EndPopup();
	}
}
