#include "PanelSaveScene.h"

#include "GUI.h"
#include "App.h"
#include "EngineCore.h"
#include "WindowEM.h"
#include "ResourceManager.h"
#include "SceneManagerEM.h"
#include "FileDialog.h"
#include "PanelLoading.h"

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
					if (!filePath.empty() && filePath != "")
					{
						std::filesystem::path fullPath(filePath);
						std::string sceneName = fullPath.stem().string();
						std::string directory = fullPath.parent_path().string();

						if (!directory.empty() && directory.back() != std::filesystem::path::preferred_separator)
							directory += std::filesystem::path::preferred_separator;

						engine->scene_manager_em->SaveScene(directory, sceneName);
					}
					//
					if (new_scene_or_open_scene)
						engine->scene_manager_em->CreateEmptyScene();
					else
					{
						std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open ManLite Scene file (*.mlscene)\0*.mlscene\0")).string();
						if (!filePath.empty() && filePath.ends_with(".mlscene"))
						{
							std::string sceneName = std::filesystem::path(filePath).stem().string();

							is_loading = true;
							app->gui->loading_panel->RequestFocus();
							app->gui->loading_panel->SetText("Loading Scene...");
							std::string description = "Scene < " + sceneName + " > is being loaded from < " + filePath + " >\n\nJust a moment! :D";
							app->gui->loading_panel->SetDescription(description);
							new_scene = std::make_shared<Scene>();
							auto scene_copy = new_scene;
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
					std::string filePath = std::filesystem::relative(FileDialog::SaveFile("Save ManLite Scene file (*.mlscene)\0*.mlscene\0")).string();
					if (!filePath.empty())
					{
						std::filesystem::path fullPath(filePath);
						std::string sceneName = fullPath.stem().string();
						std::string directory = fullPath.parent_path().string();

						if (!directory.empty() && directory.back() != std::filesystem::path::preferred_separator)
							directory += std::filesystem::path::preferred_separator;

						engine->scene_manager_em->SaveScene(directory, sceneName);
						//
						ImGui::CloseCurrentPopup();
						ResourceManager::GetInstance().ReleaseTexture("Config\\Icons\\icon.png");
						enabled = false;
						//
						if (new_scene_or_open_scene)
							engine->scene_manager_em->CreateEmptyScene();
						else
						{
							std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open ManLite Scene file (*.mlscene)\0*.mlscene\0")).string();
							if (!filePath.empty() && filePath.ends_with(".mlscene"))
							{
								std::string sceneName = std::filesystem::path(filePath).stem().string();

								is_loading = true;
								app->gui->loading_panel->RequestFocus();
								app->gui->loading_panel->SetText("Loading Scene...");
								std::string description = "Scene < " + sceneName + " > is being loaded from < " + filePath + " >\n\nJust a moment! :D";
								app->gui->loading_panel->SetDescription(description);
								new_scene = std::make_shared<Scene>();
								auto scene_copy = new_scene;
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
					if (new_scene_or_open_scene)
						engine->scene_manager_em->CreateEmptyScene();
					else
					{
						std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open ManLite Scene file (*.mlscene)\0*.mlscene\0")).string();
						if (!filePath.empty() && filePath.ends_with(".mlscene"))
						{
							std::string sceneName = std::filesystem::path(filePath).stem().string();

							is_loading = true;
							app->gui->loading_panel->RequestFocus();
							app->gui->loading_panel->SetText("Loading Scene...");
							std::string description = "Scene < " + sceneName + " > is being loaded from < " + filePath + " >\n\nJust a moment! :D";
							app->gui->loading_panel->SetDescription(description);
							new_scene = std::make_shared<Scene>();
							auto scene_copy = new_scene;
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
				if (ImGui::Button("Cancel", { 80, 20 }))
				{
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
