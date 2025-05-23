#include "PanelGame.h"

#include "GUI.h"
#include "Defs.h"
#include "EngineCore.h"
#include "RendererEM.h"
#include "SceneManagerEM.h"
#include "GameObject.h"
#include "Component.h"
#include "Camera.h"

#include <imgui.h>

PanelGame::PanelGame(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
}

PanelGame::~PanelGame()
{
}

bool PanelGame::Update()
{
	if (bringToFront)
	{
		ImGui::SetNextWindowFocus();
		bringToFront = false;
	}

	bool ret = true;

	if (ImGui::Begin(name.c_str(), &enabled))
	{
		DrawTopBarControls();

		engine->renderer_em->UseGameViewCam();

		ImVec2 window_size = ImGui::GetContentRegionAvail();
		float image_width = DEFAULT_CAM_WIDTH;
		float image_height = DEFAULT_CAM_HEIGHT;
		float image_aspect_ratio = image_width / image_height;
		float window_aspect_ratio = window_size.x / window_size.y;
		ImVec2 scaled_size;
		if (window_aspect_ratio > image_aspect_ratio)
		{
			scaled_size.y = window_size.y;
			scaled_size.x = scaled_size.y * image_aspect_ratio;
		}
		else
		{
			scaled_size.x = window_size.x;
			scaled_size.y = scaled_size.x / image_aspect_ratio;
		}
		ImVec2 image_pos = ImGui::GetCursorScreenPos();
		image_pos.x += (window_size.x - scaled_size.x) * 0.5f;
		image_pos.y += (window_size.y - scaled_size.y) * 0.5f;

		//rendering in imgui panel
		ImGui::GetWindowDrawList()->AddImage(
			(ImTextureID)(uintptr_t)engine->renderer_em->renderTexture,
			image_pos,
			ImVec2(image_pos.x + scaled_size.x, image_pos.y + scaled_size.y),
			ImVec2(0, 1),
			ImVec2(1, 0)
		);
	}
	ImGui::End();

	return ret;
}

void PanelGame::DrawTopBarControls()
{
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 5));
    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);

    const ImVec2 button_size_default = ImVec2(100, 0);
    const float combo_width = button_size_default.x * 2.5;

    ImGui::SetNextItemWidth(combo_width);

    GameObject& selected_camera_go_ref = engine->scene_manager_em->GetCurrentScene().GetCurrentCameraGO();
	GameObject* selected_camera_go = &selected_camera_go_ref;
    std::string preview_name = selected_camera_go ? selected_camera_go->GetName() : "Output Camera";

    if (ImGui::BeginCombo("##Settings##ImGuizmoFunctionality", preview_name.c_str()))
    {
        std::vector<std::shared_ptr<GameObject>> cameras;
        Scene& current_scene = engine->scene_manager_em->GetCurrentScene();

        current_scene.TraverseHierarchy([&cameras](std::shared_ptr<GameObject> go) {
            if (go->GetComponent<Camera>() && go->IsEnabled())
                cameras.push_back(go);
            });

        for (const auto& camera_go : cameras) {
            bool is_selected = (selected_camera_go == camera_go.get());
            if (ImGui::Selectable(camera_go->GetName().c_str(), is_selected)) {
                selected_camera_go = camera_go.get();
				current_scene.SetCurrentCameraGO(selected_camera_go->GetSharedPtr());
            }

            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

	ImGui::SameLine();
	ImGui::Checkbox("Rend Lights##LightsFunctionality", &engine->renderer_em->rend_lights);

    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}