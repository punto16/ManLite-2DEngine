#include "PanelSaveScene.h"

#include "GUI.h"
#include "App.h"
#include "EngineCore.h"
#include "ScriptingEM.h"
#include "WindowEM.h"
#include "ResourceManager.h"
#include "SceneManagerEM.h"
#include "FileDialog.h"
#include "PanelLoading.h"
#include "PanelTileMap.h"

#include <imgui.h>
#include "filesystem"

PanelSaveScene::PanelSaveScene(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
}

PanelSaveScene::~PanelSaveScene()
{
}

bool PanelSaveScene::Update()
{
    if (bringToFront)
    {
        ImGui::SetNextWindowFocus();
		icon_image_id = ResourceManager::GetInstance().LoadTexture("Config\\Icons\\icon.png", w, h);
        bringToFront = false;
    }

    bool ret = true;

	if (enabled)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, { 0.5, 0.5 });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 10, 10 });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5);
		ImGui::OpenPopup("Close Scene without Save?");
		ImGui::SetNextWindowSize({460, 170});
		if (ImGui::BeginPopupModal("Close Scene without Save?", nullptr, ImGuiWindowFlags_NoResize))
		{
			ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingFixedFit;
			if (ImGui::BeginTable("##TableWarning", 2, tableFlags))
			{
				ImGui::TableSetupColumn("Image", ImGuiTableColumnFlags_NoHide);
				ImGui::TableSetupColumn("Dialog", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide);
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::Image((ImTextureID)icon_image_id, { 64, 64 }, { 0, 1 }, { 1, 0 });
				ImGui::SameLine();
				ImGui::Dummy({ 8, 0 });

				ImGui::TableSetColumnIndex(1);
				ImGui::TextWrapped("Do you want to save the changes you made in the scenes:");

				std::string path = engine->scene_manager_em->GetCurrentScene().GetScenePath();
				if (path.empty() || path == "") path = engine->scene_manager_em->GetCurrentScene().GetSceneName();

				ImGui::TextWrapped(path.c_str());

				ImGui::Dummy({ 0, 8 });
				ImGui::TextWrapped("Your changes will be lost if you don't save them.");
				ImGui::Dummy({ 0, 8 });

				if (ImGui::Button("Save", { 80, 20 }))
				{
					//
					std::string filePath = engine->scene_manager_em->GetCurrentScene().GetScenePath();
					if (filePath.empty() || filePath == "")
						filePath = "Assets\\Scenes\\" + engine->scene_manager_em->GetCurrentScene().GetSceneName() + ".mlscene";
					if (!filePath.empty() && filePath != "")
					{
						std::filesystem::path fullPath(filePath);
						std::string sceneName = fullPath.stem().string();
						std::string directory = fullPath.parent_path().string();

						if (!directory.empty() && directory.back() != std::filesystem::path::preferred_separator)
							directory += std::filesystem::path::preferred_separator;

						engine->scene_manager_em->SaveScene(directory, sceneName);
						app->gui->RestartAutoSaveTimer();
					}
					//
					engine->SetEngineState(EngineState::STOP);
					//
					if (save_panel_action == SavePanelAction::NEW_SCENE)
					{
						app->gui->tile_map_panel->SetMap(nullptr);
						engine->scene_manager_em->CreateEmptyScene();
						app->gui->RestartAutoSaveTimer();
					}
					else if (save_panel_action == SavePanelAction::CLOSE_APP)
						return false;
					else
					{
						std::string filePath = dragged_path;
						if (!dragged_origin) filePath = std::filesystem::relative(FileDialog::OpenFile("Open ManLite Scene file (*.mlscene)\0*.mlscene\0", "Assets\\Scenes")).string();
						dragged_origin = false;
						dragged_path = "";
						if (!filePath.empty() && filePath.ends_with(".mlscene"))
						{
							std::string sceneName = std::filesystem::path(filePath).stem().string();

							is_loading = true;
							app->gui->loading_panel->RequestFocus();
							app->gui->loading_panel->SetText("Loading Scene...");
							std::string description = "Scene   < " + sceneName + " >   is being loaded from\nScenePath:  < " + filePath + " >\n\nJust a moment! :D";
							app->gui->loading_panel->SetDescription(description);
							new_scene = std::make_shared<Scene>();
							auto scene_copy = new_scene;
							int w = 0, h = 0;
							app->gui->tile_map_panel->SetMap(nullptr);
							ResourceManager::GetInstance().LoadTexture("Config\\placeholder.png", w, h);//load placeholder
							engine->scripting_em->stop_process_instantiate_queue = true;
							loading_task = std::async(std::launch::async, [this, filePath, scene_copy]() {
								engine->scene_manager_em->LoadSceneToScene(filePath, *scene_copy);
								is_loading = false;
								set_scene = true;
								});
						}
					}
					//
					ImGui::CloseCurrentPopup();
					ResourceManager::GetInstance().ReleaseTexture("Config\\Icons\\icon.png");
					enabled = false;
				}
				ImGui::SameLine();
				if (ImGui::Button("Save As...", { 80, 20 }))
				{
					//
					engine->SetEngineState(EngineState::STOP);
					//
					std::string filePath = std::filesystem::relative(FileDialog::SaveFile("Save ManLite Scene file (*.mlscene)\0*.mlscene\0", "Assets\\Scenes")).string();
					if (!filePath.empty())
					{
						std::filesystem::path fullPath(filePath);
						std::string sceneName = fullPath.stem().string();
						std::string directory = fullPath.parent_path().string();

						if (!directory.empty() && directory.back() != std::filesystem::path::preferred_separator)
							directory += std::filesystem::path::preferred_separator;

						engine->scene_manager_em->SaveScene(directory, sceneName);
						app->gui->RestartAutoSaveTimer();
						//
						ImGui::CloseCurrentPopup();
						ResourceManager::GetInstance().ReleaseTexture("Config\\Icons\\icon.png");
						enabled = false;
						//
						if (save_panel_action == SavePanelAction::NEW_SCENE)
						{
							app->gui->tile_map_panel->SetMap(nullptr);
							engine->scene_manager_em->CreateEmptyScene();
							app->gui->RestartAutoSaveTimer();
						}
						else if (save_panel_action == SavePanelAction::CLOSE_APP)
							return false;
						else
						{
							std::string filePath = dragged_path;
							if (!dragged_origin) filePath = std::filesystem::relative(FileDialog::OpenFile("Open ManLite Scene file (*.mlscene)\0*.mlscene\0", "Assets\\Scenes")).string();
							dragged_origin = false;
							dragged_path = "";
							if (!filePath.empty() && filePath.ends_with(".mlscene"))
							{
								std::string sceneName = std::filesystem::path(filePath).stem().string();

								is_loading = true;
								app->gui->loading_panel->RequestFocus();
								app->gui->loading_panel->SetText("Loading Scene...");
								std::string description = "Scene   < " + sceneName + " >   is being loaded from\nScenePath:  < " + filePath + " >\n\nJust a moment! :D";
								app->gui->loading_panel->SetDescription(description);
								new_scene = std::make_shared<Scene>();
								auto scene_copy = new_scene;
								int w = 0, h = 0;
								ResourceManager::GetInstance().LoadTexture("Config\\placeholder.png", w, h);//load placeholder
								app->gui->tile_map_panel->SetMap(nullptr);
								engine->scripting_em->stop_process_instantiate_queue = true;
								loading_task = std::async(std::launch::async, [this, filePath, scene_copy]() {
									engine->scene_manager_em->LoadSceneToScene(filePath, *scene_copy);
									is_loading = false;
									set_scene = true;
									});
							}
						}
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Don't Save", { 80, 20 }))
				{
					engine->SetEngineState(EngineState::STOP);
					//
					if (save_panel_action == SavePanelAction::NEW_SCENE)
					{
						app->gui->tile_map_panel->SetMap(nullptr);
						engine->scene_manager_em->CreateEmptyScene();
						app->gui->RestartAutoSaveTimer();
					}
					else if (save_panel_action == SavePanelAction::CLOSE_APP)
						return false;
					else
					{
						std::string filePath = dragged_path;
						if (!dragged_origin) filePath = std::filesystem::relative(FileDialog::OpenFile("Open ManLite Scene file (*.mlscene)\0*.mlscene\0", "Assets\\Scenes")).string();
						dragged_origin = false;
						dragged_path = "";
						if (!filePath.empty() && filePath.ends_with(".mlscene"))
						{
							std::string sceneName = std::filesystem::path(filePath).stem().string();

							is_loading = true;
							app->gui->loading_panel->RequestFocus();
							app->gui->loading_panel->SetText("Loading Scene...");
							std::string description = "Scene   < " + sceneName + " >   is being loaded from\nScenePath:  < " + filePath + " >\n\nJust a moment! :D";
							app->gui->loading_panel->SetDescription(description);
							new_scene = std::make_shared<Scene>();
							auto scene_copy = new_scene;
							int w = 0, h = 0;
							ResourceManager::GetInstance().LoadTexture("Config\\placeholder.png", w, h);//load placeholder
							app->gui->tile_map_panel->SetMap(nullptr);
							engine->scripting_em->stop_process_instantiate_queue = true;
							loading_task = std::async(std::launch::async, [this, filePath, scene_copy]() {
								engine->scene_manager_em->LoadSceneToScene(filePath, *scene_copy);
								is_loading = false;
								set_scene = true;
								});
						}
					}
					//
					ImGui::CloseCurrentPopup();
					ResourceManager::GetInstance().ReleaseTexture("Config\\Icons\\icon.png");
					enabled = false;
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel", { 80, 20 }) || ImGui::IsKeyDown(ImGuiKey_Escape))
				{
					engine->SetEngineState(EngineState::STOP);
					//
					ImGui::CloseCurrentPopup();
					ResourceManager::GetInstance().ReleaseTexture("Config\\Icons\\icon.png");
					enabled = false;
				}

				ImGui::EndTable();
			}
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar(2);
	}
    return ret;
}
