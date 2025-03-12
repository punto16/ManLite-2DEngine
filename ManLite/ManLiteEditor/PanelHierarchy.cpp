#include "PanelHierarchy.h"

#include "GUI.h"
#include "ManLiteEngine/EngineCore.h"
#include "ManLiteEngine/SceneManagerEM.h"
#include "ManLiteEngine/GameObject.h"

#include "Defs.h"
#include "algorithm"

#include <imgui.h>

PanelHierarchy::PanelHierarchy(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
}

PanelHierarchy::~PanelHierarchy()
{
}

bool PanelHierarchy::Update()
{
	bool ret = true;
	ImGuiWindowFlags settingsFlags = ImGuiWindowFlags_NoFocusOnAppearing;
	uint treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;

	if (ImGui::Begin(name.c_str(), &enabled, settingsFlags))
	{
		BlankContext(engine->scene_manager_em->GetCurrentScene().GetSceneRoot());
		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered())
			engine->scene_manager_em->GetCurrentScene().SelectGameObject(nullptr, false, true);

		if (ImGui::CollapsingHeader(std::string(engine->scene_manager_em->GetCurrentScene().GetSceneName() +
			"##" +
			std::to_string(engine->scene_manager_em->GetCurrentScene().GetSceneRoot().GetID())).c_str(), treeFlags | ImGuiTreeNodeFlags_Leaf))
		{
			if (ImGui::BeginPopupContextItem())
			{
				static char newNameBuffer[32];
				strcpy(newNameBuffer, engine->scene_manager_em->GetCurrentScene().GetSceneName().c_str());
				uint input_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsNoBlank;
				if (ImGui::InputText("Change Scene Name", newNameBuffer, sizeof(newNameBuffer), input_flags))
				{
					std::string newSceneName(newNameBuffer);
					LOG(LogType::LOG_INFO, "Scene <%s> has been renamed to <%s>", engine->scene_manager_em->GetCurrentScene().GetSceneName().c_str(), newSceneName.c_str());
					engine->scene_manager_em->GetCurrentScene().SetSceneName(newSceneName);
					newNameBuffer[0] = '\0';
				}
				ImGui::EndPopup();
			}
			IterateTree(engine->scene_manager_em->GetCurrentScene().GetSceneRoot());
		}
	}
	ImGui::End();

	return ret;
}

void PanelHierarchy::IterateTree(GameObject& parent)
{
	auto children = std::vector<std::shared_ptr<GameObject>>(parent.GetChildren());

	DropZone(parent, 0);
    for (size_t i = 0; i < children.size(); ++i) {
        auto& item = children[i];

		const bool is_disabled = !item->IsEnabled();
		if (is_disabled) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);

		uint treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
		bool empty_children = item->GetChildren().empty();
		if (empty_children) treeFlags |= ImGuiTreeNodeFlags_Leaf;

		if (IsSelected(item)) treeFlags |= ImGuiTreeNodeFlags_Selected;

		std::string nodeLabel = std::string(item->GetName() + "##" + std::to_string(item->GetID()));
		bool nodeOpen = ImGui::TreeNodeEx(nodeLabel.c_str(), treeFlags);
		if (is_disabled) ImGui::PopStyleColor();

		GameObjectSelection(*item);
		DragAndDrop(*item);

		if (nodeOpen)
		{
			Context(*item);
			if (!empty_children) IterateTree(*item);
			ImGui::TreePop();
		}

		DropZone(parent, i + 1);
	}
}

void PanelHierarchy::BlankContext(GameObject& parent)
{
	if (ImGui::BeginPopupContextWindow())
	{
		if (ImGui::MenuItem("Create Empty"))
		{
			engine->scene_manager_em->GetCurrentScene().SelectGameObject(engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(parent));
		}
		ImGui::EndPopup();
	}
}

void PanelHierarchy::Context(GameObject& parent)
{
	if (ImGui::BeginPopupContextItem())
	{
		auto& scene = engine->scene_manager_em->GetCurrentScene();
		const auto& selected = scene.GetSelectedGOs();
		int selectedCount = selected.size();

		if (!IsSelected(parent.GetSharedPtr())) scene.SelectGameObject(parent.GetSharedPtr());
		if (selectedCount > 1)
		{
			if (ImGui::MenuItem(("Duplicate (" + std::to_string(selectedCount) + ")").c_str()))
			{
				std::vector<std::shared_ptr<GameObject>> new_selected;
				for (const auto& weakGo : selected)
				{
					if (auto go = weakGo.lock())
					{
						new_selected.push_back(scene.DuplicateGO(*go));
					}
				}
				scene.SelectGameObject(nullptr, false, true);
				for (const auto& go : new_selected)
				{
					scene.SelectGameObject(go, true);
				}
			}

			if (ImGui::MenuItem(("Delete (" + std::to_string(selectedCount) + ")").c_str()))
			{
				for (const auto& weakGo : selected) {
					if (auto go = weakGo.lock()) go->Delete();
				}
				scene.SelectGameObject(nullptr, false, true);
			}

			if (ImGui::MenuItem("Create Parent for Selected"))
			{
				auto new_parent = scene.CreateEmptyGO(parent);
				new_parent->SetName("Group");
				for (const auto& weakGo : selected)
					if (auto go = weakGo.lock()) go->Reparent(new_parent, true);
				scene.SelectGameObject(new_parent);
			}
		}
		else
		{
			if (ImGui::MenuItem("Duplicate"))
			{
				scene.SelectGameObject(engine->scene_manager_em->GetCurrentScene().DuplicateGO(parent));
			}
			if (ImGui::MenuItem("Delete"))
			{
				parent.Delete();
			}
			if (ImGui::MenuItem("Create Empty"))
			{
				scene.SelectGameObject(engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(parent));
			}
			if (ImGui::MenuItem("Create Empty Parent"))
			{
				std::shared_ptr<GameObject> new_parent = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(parent);
				parent.Reparent(new_parent, true);
				new_parent->SetName(GameObject::GenerateUniqueName(new_parent->GetName(), new_parent.get()));
				scene.SelectGameObject(new_parent);
			}
		}
		ImGui::EndPopup();
	}
}

void PanelHierarchy::DragAndDrop(GameObject& parent)
{
	auto& scene = engine->scene_manager_em->GetCurrentScene();

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	{
		const auto& selected = scene.GetSelectedGOs();

		if (IsSelected(parent.GetSharedPtr()))
		{
			std::vector<std::shared_ptr<GameObject>> draggedItems;
			for (const auto& weakGo : selected)
				if (auto go = weakGo.lock()) draggedItems.push_back(go);

			ImGui::SetDragDropPayload("DND_MULTI_NODE", draggedItems.data(), draggedItems.size() * sizeof(std::shared_ptr<GameObject>));
			if (draggedItems.size() <= 1) ImGui::Text("Moving <%s>", parent.GetName().c_str());
			else ImGui::Text("Moving %d Objects", draggedItems.size());
		}
		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_MULTI_NODE")) {
			auto draggedItems = reinterpret_cast<std::shared_ptr<GameObject>*>(payload->Data);
			size_t count = payload->DataSize / sizeof(std::shared_ptr<GameObject>);

			for (size_t i = 0; i < count; ++i)
				draggedItems[i]->Reparent(parent.GetSharedPtr());
		}
		ImGui::EndDragDropTarget();
	}
}

void PanelHierarchy::DropZone(GameObject& parent, int position)
{
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	std::string nodeLabel = "##DropZone" + std::to_string(parent.GetID()) + std::to_string(position);
	ImGui::InvisibleButton(nodeLabel.c_str(), ImVec2(-1, 4));
	ImGui::PopStyleVar();

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_MULTI_NODE"))
		{
			auto draggedItems = reinterpret_cast<std::shared_ptr<GameObject>*>(payload->Data);
			size_t count = payload->DataSize / sizeof(std::shared_ptr<GameObject>);

			for (size_t i = 0; i < count; ++i)
			{
				draggedItems[i]->Reparent(parent.GetSharedPtr());
				draggedItems[i]->MoveInVector(position);
			}
		}
		else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_NODE"))
		{
			GameObject* draggedItem = *(GameObject**)payload->Data;
			draggedItem->Reparent(parent.GetSharedPtr());
			draggedItem->MoveInVector(position);
		}
		ImGui::EndDragDropTarget();
	}
}

void PanelHierarchy::GameObjectSelection(GameObject& go)
{
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsMouseDragging(ImGuiMouseButton_Left))
	{
		auto& scene = engine->scene_manager_em->GetCurrentScene();
		auto itemShared = go.GetSharedPtr();

		if (ImGui::GetIO().KeyCtrl)
		{
			scene.SelectGameObject(itemShared, true);
		}
		else if (ImGui::GetIO().KeyShift)
		{
			scene.SelectRange(itemShared);
		}
		else
		{
			if (!IsSelected(itemShared)) scene.SelectGameObject(go.GetSharedPtr());
		}
	}
	else if (ImGui::IsItemHovered() &&
		ImGui::IsMouseReleased(ImGuiMouseButton_Left) &&
		!ImGui::GetIO().KeyShift &&
		!ImGui::GetIO().KeyCtrl)
	{
		auto& scene = engine->scene_manager_em->GetCurrentScene();
		scene.SelectGameObject(go.GetSharedPtr());
	}
}

bool PanelHierarchy::IsSelected(const std::shared_ptr<GameObject>& go)
{
	const auto& selected = engine->scene_manager_em->GetCurrentScene().GetSelectedGOs();
	return std::find_if(selected.begin(), selected.end(),
		[&](const auto& weakGo) { return weakGo.lock() == go; }) != selected.end();
}

