#include "PanelHierarchy.h"

#include "GUI.h"
#include "ManLiteEngine/EngineCore.h"
#include "ManLiteEngine/SceneManagerEM.h"
#include "ManLiteEngine/GameObject.h"

#include "Defs.h"

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

		if (ImGui::CollapsingHeader(std::string(engine->scene_manager_em->GetCurrentScene().GetSceneName() +
			"##" +
			std::to_string(engine->scene_manager_em->GetCurrentScene().GetSceneRoot().GetID())).c_str(), treeFlags | ImGuiTreeNodeFlags_Leaf))
		{
			if (ImGui::BeginPopupContextItem())
			{
				static char newNameBuffer[32];
				strcpy(newNameBuffer, engine->scene_manager_em->GetCurrentScene().GetSceneName().c_str());
				if (ImGui::InputText("Change Scene Name", newNameBuffer, sizeof(newNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
				{
					std::string newSceneName(newNameBuffer);
					LOG(LogType::LOG_INFO, "Scene %s has been renamed to %s", engine->scene_manager_em->GetCurrentScene().GetSceneName().c_str(), newSceneName.c_str());
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
	for (const auto& item : parent.GetChildren())
	{
		uint treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
		bool empty_children = item->GetChildren().empty();
		if (empty_children) treeFlags |= ImGuiTreeNodeFlags_Leaf;
		if (ImGui::TreeNodeEx(std::string(item->GetName() +
		"##" +
		std::to_string(item->GetID())).c_str(), treeFlags))
		{
			Context(*item);
			if (!empty_children) IterateTree(*item);
			ImGui::TreePop();
		}
	}
}

void PanelHierarchy::BlankContext(GameObject& parent)
{
	if (ImGui::BeginPopupContextWindow())
	{
		if (ImGui::MenuItem("Create Empty"))
		{
			engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(parent);
		}
		ImGui::EndPopup();
	}
}

void PanelHierarchy::Context(GameObject& parent)
{
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem("Create Empty"))
		{
			engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(parent);
		}
		if (ImGui::MenuItem("Create Empty Parent"))
		{
			std::shared_ptr<GameObject> new_parent = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(parent);
			parent.Reparent(new_parent);
			new_parent->SetName(GameObject::GenerateUniqueName(new_parent->GetName(), new_parent.get()));
		}
		ImGui::EndPopup();
	}
}
