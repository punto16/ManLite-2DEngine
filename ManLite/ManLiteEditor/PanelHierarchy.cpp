#include "PanelHierarchy.h"

#include "GUI.h"
#include "App.h"
#include "PanelTileMap.h"
#include "ManLiteEngine/EngineCore.h"
#include "ManLiteEngine/SceneManagerEM.h"
#include "ManLiteEngine/GameObject.h"
#include "ManLiteEngine/TileMap.h"
#include "ManLiteEngine/Prefab.h"
#include "ManLiteEngine/FileDialog.h"
#include "ManLiteEngine/Light.h"
#include "ManLiteEngine/Sprite2D.h"
#include "ManLiteEngine/AudioSource.h"
#include "ManLiteEngine/ParticleSystem.h"
#include "ManLiteEngine/Collider2D.h"
#include "ManLiteEngine/Camera.h"
#include "ManLiteEngine/Script.h"
#include "ManLiteEngine/Canvas.h"

#include "Defs.h"
#include "algorithm"
#include "filesystem"

#include <imgui.h>

PanelHierarchy::PanelHierarchy(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
}

PanelHierarchy::~PanelHierarchy()
{
}

bool PanelHierarchy::Update()
{
	if (bringToFront)
	{
		ImGui::SetNextWindowFocus();
		bringToFront = false;
	}

	bool ret = true;
	ImGuiWindowFlags settingsFlags = ImGuiWindowFlags_NoFocusOnAppearing;
	uint treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;

	if (ImGui::Begin(name.c_str(), &enabled, settingsFlags))
	{
		ImGui::InputText("##Search", searchTextBuffer, IM_ARRAYSIZE(searchTextBuffer), ImGuiInputTextFlags_None);
		ImGui::SameLine();
		ImGui::Text("Search");

		std::string searchText = std::string(searchTextBuffer);
		std::transform(searchText.begin(), searchText.end(), searchText.begin(), ::tolower);

		if (!searchText.empty())
		{
			auto& scene = engine->scene_manager_em->GetCurrentScene();
			auto& sceneRoot = scene.GetSceneRoot();
			std::vector<std::shared_ptr<GameObject>> allGameObjects;
			for (auto& child : sceneRoot.GetChildren()) {
				CollectAllGameObjects(*child, allGameObjects);
			}

			for (auto& go : allGameObjects) {
				std::string goNameLower = go->GetName();
				std::transform(goNameLower.begin(), goNameLower.end(), goNameLower.begin(), ::tolower);
				if (goNameLower.find(searchText) != std::string::npos) {
					std::string nodeLabel = go->GetName() + "##" + std::to_string(go->GetID());
					uint nodeFlags = ImGuiTreeNodeFlags_Leaf;
					if (IsSelected(go)) nodeFlags |= ImGuiTreeNodeFlags_Selected;

					bool nodeOpen = ImGui::TreeNodeEx(nodeLabel.c_str(), nodeFlags);
					GameObjectSelection(*go);
					DragAndDrop(*go);
					Context(*go);

					if (nodeOpen) ImGui::TreePop();
				}
			}
		}
		else
		{
			BlankContext(engine->scene_manager_em->GetCurrentScene().GetSceneRoot());

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
				IterateTree(engine->scene_manager_em->GetCurrentScene().GetSceneRoot(), true);
			}
			if (engine->GetEngineState() != PLAY)
			{
				ImVec2 window_size = { (float)(ImGui::GetContentRegionMax().x - 2.5), (float)(ImGui::GetContentRegionMax().y - 5) };
				ImGui::SetCursorPos({5,5});
				ImGui::InvisibleButton("##HierarchyDropTarget", window_size);
			}

			if (ImGui::IsItemHovered() &&
				ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
				!ImGui::GetIO().KeyCtrl &&
				!ImGui::GetIO().KeyShift)
			{
				engine->scene_manager_em->GetCurrentScene().SelectGameObject(nullptr, false, true);
			}

			if (engine->GetEngineState() != PLAY)
			{
				if (ImGui::BeginDragDropTarget())
				{
					const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_TILED");
					if (payload)
					{
						const char* payload_path = static_cast<const char*>(payload->Data);
						std::string dragged_path(payload_path);
						if (dragged_path.ends_with(".json")) engine->scene_manager_em->ImportTiledFile(dragged_path);
						else LOG(LogType::LOG_WARNING, "Wrong Tiled Format. Correct format to import Tiled file is .json");
					}
					payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_PREFAB");
					if (payload)
					{
						const char* payload_path = static_cast<const char*>(payload->Data);
						std::string dragged_path(payload_path);
						if (dragged_path.ends_with(".prefab")) Prefab::Instantiate(dragged_path, engine->scene_manager_em->GetCurrentScene().GetSceneRoot().GetSharedPtr());
						else LOG(LogType::LOG_WARNING, "Wrong Prefab Format. Correct format to import Prefab file is .prefab");
					}
					ImGui::EndDragDropTarget();
				}
			}
		}
	}
	ImGui::End();

	return ret;
}

void PanelHierarchy::IterateTree(GameObject& parent, bool enabled)
{
	auto children = std::vector<std::shared_ptr<GameObject>>(parent.GetChildren());

	DropZone(parent, 0);
    for (size_t i = 0; i < children.size(); ++i) {
        auto& item = children[i];

		if (item->IsPrefabInstance())
		{
			if (item->IsPrefabModified())
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255)); // red
			else
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 191, 255, 255)); // blue
		}

		const bool is_disabled = !item->IsEnabled();
		if (!enabled || is_disabled) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
		uint treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
		bool empty_children = item->GetChildren().empty();
		if (empty_children) treeFlags |= ImGuiTreeNodeFlags_Leaf;

		if (IsSelected(item)) treeFlags |= ImGuiTreeNodeFlags_Selected;

		std::string nodeLabel = std::string(item->GetName() + "##" + std::to_string(item->GetID()));
		bool nodeOpen = ImGui::TreeNodeEx(nodeLabel.c_str(), treeFlags);
		if (!enabled || is_disabled) ImGui::PopStyleColor();
		if (item->IsPrefabInstance()) ImGui::PopStyleColor();

		GameObjectSelection(*item);
		DragAndDrop(*item);

		Context(*item);

		if (nodeOpen)
		{
			if (!empty_children) IterateTree(*item, !(!enabled || is_disabled));
			ImGui::TreePop();
		}

		DropZone(parent, i + 1);
	}
}

void PanelHierarchy::BlankContext(GameObject& parent)
{
	if (ImGui::BeginPopupContextWindow())
	{
		// Create Empty
		if (ImGui::MenuItem("Create Empty"))
		{
			auto newGO = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(parent);
			engine->scene_manager_em->GetCurrentScene().SelectGameObject(newGO);
		}

		ImGui::Separator();

		// Camera
		if (ImGui::MenuItem("Create Camera"))
		{
			auto newGO = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(parent);
			newGO->SetName("Camera");
			newGO->AddComponent<Camera>();
			engine->scene_manager_em->GetCurrentScene().SelectGameObject(newGO);
		}

		// Collider 2D
		if (ImGui::MenuItem("Create Collider2D"))
		{
			auto newGO = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(parent);
			newGO->SetName("Collider2D");
			newGO->AddComponent<Collider2D>();
			engine->scene_manager_em->GetCurrentScene().SelectGameObject(newGO);
		}

		// Script
		if (ImGui::MenuItem("Create Script"))
		{
			std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open ManLite Script file (*.cs)\0*.cs\0", "Assets\\Scripts")).string();
			if (!filePath.empty() && filePath.ends_with(".cs"))
			{
				auto newGO = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(parent);
				newGO->SetName("Script");
				std::filesystem::path fullPath(filePath);
				std::string script_name = fullPath.stem().string();
				newGO->AddComponent<Script>(script_name);
				engine->scene_manager_em->GetCurrentScene().SelectGameObject(newGO);
			}
		}

		ImGui::Separator();

		// Canvas
		if (ImGui::MenuItem("Create Canvas"))
		{
			auto newGO = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(parent);
			newGO->SetName("Canvas");
			newGO->AddComponent<Canvas>();
			engine->scene_manager_em->GetCurrentScene().SelectGameObject(newGO);
		}

		ImGui::Separator();

		// Particle System
		if (ImGui::MenuItem("Create Particle System"))
		{
			auto newGO = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(parent);
			newGO->SetName("ParticleSystem");
			newGO->AddComponent<ParticleSystem>();
			engine->scene_manager_em->GetCurrentScene().SelectGameObject(newGO);
		}

		ImGui::Separator();

		// Audio Source
		if (ImGui::MenuItem("Create Audio Source"))
		{
			auto newGO = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(parent);
			newGO->SetName("AudioSource");
			newGO->AddComponent<AudioSource>();
			engine->scene_manager_em->GetCurrentScene().SelectGameObject(newGO);
		}

		ImGui::Separator();

		// Sprite
		if (ImGui::MenuItem("Create Sprite"))
		{
			std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Sprite file (*.png)\0*.png\0", "Assets\\Textures")).string();
			if (!filePath.empty() && filePath.ends_with(".png"))
			{
				auto newGO = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(parent);
				newGO->SetName("Sprite");
				newGO->AddComponent<Sprite2D>(filePath);
				engine->scene_manager_em->GetCurrentScene().SelectGameObject(newGO);
			}
		}

		// TileMap
		if (ImGui::MenuItem("Create TileMap"))
		{
			std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open TileSet (*.png)\0*.png\0", "Assets\\TileMaps")).string();
			if (!filePath.empty() && filePath.ends_with(".png"))
			{
				auto newGO = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(parent);
				newGO->SetName("TileMap");
				newGO->AddComponent<TileMap>();
				newGO->GetComponent<TileMap>()->SwapTexture(filePath);
				engine->scene_manager_em->GetCurrentScene().SelectGameObject(newGO);
			}
		}

		ImGui::Separator();

		// Lights
		if (ImGui::MenuItem("Ambient Light"))
		{
			auto newGO = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(parent);
			newGO->SetName("AmbientLight");
			newGO->AddComponent<Light>();
			newGO->GetComponent<Light>()->SetType(LightType::AREA_LIGHT);
			engine->scene_manager_em->GetCurrentScene().SelectGameObject(newGO);
		}
		if (ImGui::MenuItem("Point Light"))
		{
			auto newGO = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(parent);
			newGO->SetName("PointLight");
			newGO->AddComponent<Light>();
			newGO->GetComponent<Light>()->SetType(LightType::POINT_LIGHT);
			engine->scene_manager_em->GetCurrentScene().SelectGameObject(newGO);
		}
		if (ImGui::MenuItem("Spot Light"))
		{
			auto newGO = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(parent);
			newGO->SetName("SpotLight");
			newGO->AddComponent<Light>();
			newGO->GetComponent<Light>()->SetType(LightType::RAY_LIGHT);
			engine->scene_manager_em->GetCurrentScene().SelectGameObject(newGO);
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

		if (selectedCount > 1 && IsSelected(parent.GetSharedPtr()))
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
					if (auto map = weakGo.lock()->GetComponent<TileMap>())
					{
						if (map->GetID() == app->gui->tile_map_panel->GetMap()->GetID())
							app->gui->tile_map_panel->SetMap(nullptr);
					}

					if (auto go = weakGo.lock()) go->Delete();
				}
				scene.UnselectAll();
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
				if (auto map = parent.GetComponent<TileMap>())
				{
					if (app->gui->tile_map_panel->GetMap() && map->GetID() == app->gui->tile_map_panel->GetMap()->GetID())
						app->gui->tile_map_panel->SetMap(nullptr);
				}
				parent.Delete();
				scene.UnselectAll();
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
			ImGui::Separator();

			if (parent.IsPrefabInstance())
			{
				if (ImGui::MenuItem("Revert to Prefab"))
				{
					parent.LoadGameObject(parent.GetPrefabOriginalData());
					parent.SetPrefabModified(false);
				}

				if (ImGui::MenuItem("Overwrite Prefab"))
				{
					nlohmann::json prefab_data;
					Prefab::SaveAsPrefab(parent.GetSharedPtr(), parent.GetPrefabPath(), prefab_data);
					parent.GetPrefabOriginalData() = prefab_data;
					parent.SetPrefabModified(false);
				}

				if (ImGui::MenuItem("Unlink Prefab"))
				{
					std::string t = "";
					parent.SetPrefabPath(t);
					parent.SetPrefabModified(false);
				}
			}
			else
			{
				if (ImGui::MenuItem("Convert to Prefab"))
				{
					std::string filePath = std::filesystem::relative(FileDialog::SaveFile("Save Prefab file (*.prefab)\0*.prefab\0", "Assets\\Prefabs")).string();
					if (!filePath.empty())
					{
						std::string stem_name = std::filesystem::path(filePath).stem().string();

						nlohmann::json prefab_data;
						if (!filePath.ends_with(".prefab")) filePath += ".prefab";
						if (Prefab::SaveAsPrefab(parent.GetSharedPtr(), filePath, prefab_data))
						{
							parent.SetPrefabPath(filePath);
							if (!parent.GetPrefabPath().empty())
							{
								parent.GetPrefabOriginalData() = prefab_data;
								LOG(LogType::LOG_OK, "Prefab file saved to: %s", filePath.c_str());
							}
						}
					}
				}
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
		else
		{
			std::vector<std::shared_ptr<GameObject>> draggedItems;
			draggedItems.push_back(parent.GetSharedPtr());
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
	if (ImGui::IsItemHovered(ImGuiMouseButton_Left) &&
		ImGui::IsMouseReleased(ImGuiMouseButton_Left)
		&& !ImGui::IsMouseDragging(ImGuiMouseButton_Left))
	{
		auto& scene = engine->scene_manager_em->GetCurrentScene();
		auto itemShared = go.GetSharedPtr();

		if (ImGui::GetIO().KeyCtrl)
		{
			if (IsSelected(itemShared))
				scene.DeselectGameObject(itemShared);
			else
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

void PanelHierarchy::CollectAllGameObjects(GameObject& parent, std::vector<std::shared_ptr<GameObject>>& list)
{
	list.push_back(parent.GetSharedPtr());
	for (auto& child : parent.GetChildren()) {
		CollectAllGameObjects(*child, list);
	}
}

