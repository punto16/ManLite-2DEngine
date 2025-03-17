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
        auto layer_list = std::vector<std::shared_ptr<Layer>>(engine->scene_manager_em->GetCurrentScene().GetSceneLayers());

        for (const auto& layer : layer_list)
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

            ImGui::PushID(layer_id);
            bool collapsing_header_layer = ImGui::CollapsingHeader(layer_header_label.c_str(), treeFlags);
            ImGui::PopID();
            HandleLayerDragAndDrop(*layer);

            HandleLayerHeaderDrop(*layer);

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
    if (children.empty()) return;

    bool is_collapsed = collapsed_layers[layer.GetLayerID()];
    if (!is_collapsed) GameObjectDropZone(layer, 0);
  
    for (size_t i = 0; i < children.size(); ++i)
    {
        auto& go = children[i];
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

        ImGui::PushID(go->GetID());
        HandleGameObjectDragAndDrop(*go, layer);
        ImGui::PopID();

        if (!is_collapsed) GameObjectDropZone(layer, i + 1);
	}
	if (!children.empty() && !is_collapsed) ImGui::Separator();
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

void PanelLayer::HandleLayerDragAndDrop(Layer& layer)
{
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        dragged_layer_index = engine->scene_manager_em->GetCurrentScene().GetLayerIndex(layer.GetLayerID());
        ImGui::SetDragDropPayload("DND_LAYER", &dragged_layer_index, sizeof(int));

        request_collapse_all = true;
        dragging_layer = true;

        ImGui::Text("Moving Layer: %s", layer.GetLayerName().c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_LAYER"))
        {
            int payload_index = *(const int*)payload->Data;
            engine->scene_manager_em->GetCurrentScene().ReorderLayer(payload_index, layer.GetLayerID());
        }
        ImGui::EndDragDropTarget();
    }
}

void PanelLayer::HandleGameObjectDragAndDrop(GameObject& go, Layer& target_layer)
{
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        dragged_gameobject = go.GetSharedPtr();
        ImGui::SetDragDropPayload("DND_GAMEOBJECT", &go, sizeof(GameObject*));

        ImGui::Text("Moving %s", go.GetName().c_str());
        ImGui::EndDragDropSource();
    }
}

void PanelLayer::GameObjectDropZone(Layer& target_layer, int position)
{
    if (!collapsed_layers[target_layer.GetLayerID()]) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

        std::string nodeLabel = "##DropZoneLayer" +
            std::to_string(target_layer.GetLayerID()) +
            std::to_string(position);

        ImGui::InvisibleButton(nodeLabel.c_str(), ImVec2(-1, 6));

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_GAMEOBJECT"))
            
                if (GameObject* dragged_go = *(GameObject**)payload->Data)
                    engine->scene_manager_em->GetCurrentScene().ReparentToLayer(
                        dragged_go->GetSharedPtr(),
                        target_layer.GetLayerID(),
                        position
                    );
               
            
            ImGui::EndDragDropTarget();
        }

        ImGui::PopStyleVar();
    }
}

void PanelLayer::HandleLayerHeaderDrop(Layer& target_layer)
{
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_GAMEOBJECT"))
        
            if (GameObject* dragged_go = *(GameObject**)payload->Data) 
                engine->scene_manager_em->GetCurrentScene().ReparentToLayer(
                    dragged_go->GetSharedPtr(),
                    target_layer.GetLayerID(),
                    target_layer.GetChildren().size()
                );
            
        
        ImGui::EndDragDropTarget();
    }
}
