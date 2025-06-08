#include "PanelScene.h"

#include "App.h"
#include "GUI.h"
#include "PanelSaveScene.h"
#include "EngineCore.h"
#include "RendererEM.h"
#include "InputEM.h"
#include "Prefab.h"
#include "Defs.h"
#include "WindowEM.h"
#include "SceneManagerEM.h"
#include "GameObject.h"
#include "Transform.h"
#include "mat3f.h"

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL.h>
#include <gl/GL.h>
#include <imgui.h>
#include <vector>
#include "ImGuizmo.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

PanelScene::PanelScene(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled), cam_speed(5.0f)
{
}

PanelScene::~PanelScene()
{
}

void PanelScene::Start()
{
	openglTextureID = engine->renderer_em->renderTexture;
	grid = new Grid(METERS_TO_PIXELS(1), 32);
}

bool PanelScene::Update()
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
		
		engine->renderer_em->UseSceneViewCam();

		if (render_grid) grid->Draw(engine->renderer_em->GetSceneCamera().GetViewProjMatrix());
		
		//movement of scene camera
		InputToCamMovement();

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

		if (engine->GetEngineState() != EngineState::PLAY)
			ImGuizmoFunctionality(image_pos, scaled_size);


		//drag and drop
		if (engine->input_em->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT || engine->input_em->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP)
		{
			ImGui::SetCursorScreenPos(image_pos);
			ImGui::InvisibleButton("##SceneImageDragTarget", scaled_size);

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
				payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_SCENE");
				if (payload)
				{
					const char* payload_path = static_cast<const char*>(payload->Data);
					std::string dragged_path(payload_path);
					if (dragged_path.ends_with(".mlscene"))
					{
						app->gui->save_scene_panel->RequestFocus();
						app->gui->save_scene_panel->dragged_origin = true;
						app->gui->save_scene_panel->dragged_path = dragged_path;
						app->gui->save_scene_panel->save_panel_action = SavePanelAction::OPEN_SCENE;
					}
					else LOG(LogType::LOG_WARNING, "Wrong Scene Format. Correct format to import Scene file is .mlscene");
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

	ImGui::End();

	return ret;
}

bool PanelScene::CleanUp()
{
	bool ret = true;

	RELEASE(grid)

	return ret;
}

void PanelScene::InputToCamMovement()
{
	static bool middle_drag = false;
	static int prev_mouse_x, prev_mouse_y;

	if (engine->input_em->GetMouseButtonDown(SDL_BUTTON_MIDDLE) == KEY_REPEAT)
	{
		int current_mouse_x, current_mouse_y;
		engine->input_em->GetMousePosition(current_mouse_x, current_mouse_y);

		if (!middle_drag)
		{
			if (!ImGui::IsWindowHovered()) return;
			SDL_CaptureMouse(SDL_TRUE);
			prev_mouse_x = current_mouse_x;
			prev_mouse_y = current_mouse_y;
			middle_drag = true;
		}
		else
		{
			bool wrapped = MouseHasTPed(current_mouse_x, current_mouse_y, prev_mouse_x, prev_mouse_y);

			float delta_x = 0.0f;
			float delta_y = 0.0f;
			if (!wrapped)
			{
				delta_x = static_cast<float>(current_mouse_x - prev_mouse_x);
				delta_y = static_cast<float>(current_mouse_y - prev_mouse_y);
			}

			ImVec2 panel_size = ImGui::GetContentRegionAvail();
			Camera2D& camera = engine->renderer_em->GetSceneCamera();
			ImVec2 panelSize = ImGui::GetContentRegionAvail();
			float panelWidth = panelSize.x;
			float panelHeight = panelSize.y;

			float scale_factor = 1.0f / camera.GetZoom();

			float aspect_ratio = panelWidth / panelHeight;

			float world_delta_x = -delta_x * scale_factor * (2.0f / panelWidth);
			float world_delta_y = delta_y * scale_factor * (2.0f / panelHeight) * 1 / aspect_ratio;

			float drag_sensitivity = 850.0f;
			world_delta_x *= drag_sensitivity;
			world_delta_y *= drag_sensitivity;

			camera.Move(glm::vec2(world_delta_x, world_delta_y));

			prev_mouse_x = current_mouse_x;
			prev_mouse_y = current_mouse_y;
		}
	}
	else
	{
		if (middle_drag)
		{
			SDL_CaptureMouse(SDL_FALSE);
			middle_drag = false;
		}
	}

	if (!ImGui::IsWindowHovered()) return;

	cam_speed = 5.0f / engine->renderer_em->GetSceneCamera().GetZoom();
	if (engine->input_em->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_REPEAT)
	{
		if (engine->input_em->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) cam_speed *= 2;
		if (engine->input_em->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT) cam_speed *= 0.5;
		if (engine->input_em->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) engine->renderer_em->GetSceneCamera().Move(vec2f(0, cam_speed));
		if (engine->input_em->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) engine->renderer_em->GetSceneCamera().Move(vec2f(0, -cam_speed));
		if (engine->input_em->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) engine->renderer_em->GetSceneCamera().Move(vec2f(-cam_speed, 0));
		if (engine->input_em->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) engine->renderer_em->GetSceneCamera().Move(vec2f(cam_speed, 0));
	}
	float zoom_speed = 0.1f;
	int mouse_wheel = engine->input_em->GetMouseWheelMotion();
	if (mouse_wheel != 0)
	{
		if (engine->input_em->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) zoom_speed *= 2;
		if (engine->input_em->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT) zoom_speed *= 0.5;
		engine->renderer_em->GetSceneCamera().Zoom(1.0f + (mouse_wheel * zoom_speed));
	}
}


bool PanelScene::MouseHasTPed(int& current_mouse_x, int& current_mouse_y, int& prev_mouse_x, int& prev_mouse_y)
{
	int window_width, window_height;
	SDL_GetWindowSize(engine->window_em->GetSDLWindow(), &window_width, &window_height);

	bool wrapped = false;
	int new_x = current_mouse_x;
	int new_y = current_mouse_y;

	if (current_mouse_x < 0)
	{
		new_x = window_width + current_mouse_x;
		wrapped = true;
	}
	else if (current_mouse_x >= window_width)
	{
		new_x = current_mouse_x - window_width;
		wrapped = true;
	}

	if (current_mouse_y < 0)
	{
		new_y = window_height + current_mouse_y;
		wrapped = true;
	}
	else if (current_mouse_y >= window_height)
	{
		new_y = current_mouse_y - window_height;
		wrapped = true;
	}

	if (wrapped)
	{
		SDL_WarpMouseInWindow(engine->window_em->GetSDLWindow(), new_x, new_y);
		current_mouse_x = new_x;
		current_mouse_y = new_y;
		prev_mouse_x = new_x;
		prev_mouse_y = new_y;
	}

	return wrapped;
}

void PanelScene::ImGuizmoFunctionality(ImVec2 image_pos, ImVec2 scaled_size)
{
	auto& scene = engine->scene_manager_em->GetCurrentScene();
	const auto& selected_gos = scene.GetSelectedGOs();
	if (!selected_gos.empty()) {
		auto selected_go = selected_gos[0].lock();
		if (selected_go) {
			auto transform = selected_go->GetComponent<Transform>();
			if (transform)
			{
				mat3f worldMat = transform->GetWorldMatrix();
				float matrix[16];

				matrix[0] = worldMat.m[0];
				matrix[1] = worldMat.m[1];
				matrix[2] = 0;
				matrix[3] = 0;
				matrix[4] = worldMat.m[3];
				matrix[5] = worldMat.m[4];
				matrix[6] = 0;
				matrix[7] = 0;
				matrix[8] = 0;
				matrix[9] = 0;
				matrix[10] = 1;
				matrix[11] = 0;
				matrix[12] = worldMat.m[6];
				matrix[13] = worldMat.m[7];
				matrix[14] = 0;
				matrix[15] = 1;

				Camera2D& camera = engine->renderer_em->GetSceneCamera();
				ImGuizmo::SetOrthographic(true);
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(image_pos.x, image_pos.y, scaled_size.x, scaled_size.y);
				if (!ImGui::GetIO().WantTextInput && (ImGui::IsWindowHovered() || ImGui::IsWindowFocused()))
				{
					if (ImGui::IsKeyPressed(ImGuiKey_Q)) op = (ImGuizmo::OPERATION)-1;
					if (ImGui::IsKeyPressed(ImGuiKey_W)) op = ImGuizmo::TRANSLATE;
					if (ImGui::IsKeyPressed(ImGuiKey_E)) op = ImGuizmo::ROTATE;
					if (ImGui::IsKeyPressed(ImGuiKey_R)) op = ImGuizmo::SCALE;
				}
				if (op == -1) return;

				float snapValues[3] = { snapValue, snapValue, snapValue };

				if (ImGuizmo::Manipulate(
					glm::value_ptr(-camera.GetViewMatrix()),
					glm::value_ptr(-camera.GetProjectionMatrix()),
					(ImGuizmo::OPERATION)op,
					(ImGuizmo::MODE)gizmoMode,
					matrix,
					nullptr,
					snapEnabled && op != ImGuizmo::SCALE ? snapValues : nullptr
				))
				{
					mat3f newMat;
					newMat.m[0] = matrix[0];
					newMat.m[1] = matrix[1];
					newMat.m[3] = matrix[4];
					newMat.m[4] = matrix[5];
					newMat.m[6] = matrix[12];
					newMat.m[7] = matrix[13];

					switch (op)
					{
					case ImGuizmo::TRANSLATE:
					{
						vec2f newPosition = newMat.GetTranslation();
						if (snapEnabled)
						{
							newPosition.x = std::round(newPosition.x / snapValue) * snapValue;
							newPosition.y = std::round(newPosition.y / snapValue) * snapValue;
						}
						transform->SetWorldPosition(newPosition);
						break;
					}
					case ImGuizmo::ROTATE:
					{
						float rotation_deg = newMat.GetRotation() * (180.0f / PI);
						if (snapEnabled)
						{
							rotation_deg = std::round(rotation_deg / snapValue) * snapValue;
						}
						transform->SetWorldAngle(rotation_deg);
						break;
					}
					case ImGuizmo::SCALE:
					{
						transform->SetWorldScale(newMat.GetScale());
						break;
					}
					default:
						break;
					}
				}
			}
		}
	}
}

void PanelScene::DrawTopBarControls()
{
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 5));
	ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);

	const ImVec2 button_size_default = ImVec2(50, 0);
	const float combo_width = button_size_default.x * 2.5;

	if (ImGui::Button("None##ImGuizmoFunctionality", button_size_default)) op = (ImGuizmo::OPERATION)-1;
	ImGui::SameLine();
	if (ImGui::Button("Move##ImGuizmoFunctionality", button_size_default)) op = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::Button("Rotate##ImGuizmoFunctionality", button_size_default)) op = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::Button("Scale##ImGuizmoFunctionality", button_size_default)) op = ImGuizmo::SCALE;

	ImGui::SameLine();
	ImGui::SetNextItemWidth(combo_width);
	if (ImGui::BeginCombo("##Settings##ImGuizmoFunctionality", "Snap Settings"))
	{
		ImGui::Text("Pivote Mode:");
		if (ImGui::RadioButton("Global##ImGuizmoFunctionality", gizmoMode == ImGuizmo::WORLD))
			gizmoMode = ImGuizmo::WORLD;
		ImGui::SameLine();
		if (ImGui::RadioButton("Local##ImGuizmoFunctionality", gizmoMode == ImGuizmo::LOCAL))
			gizmoMode = ImGuizmo::LOCAL;

		ImGui::Checkbox("Snap Grid##ImGuizmoFunctionality", &snapEnabled);
		if (snapEnabled) {
			ImGui::DragFloat("GridSize##ImGuizmoFunctionality", &snapValue, 0.1f, 0.1f, 10.0f, "%.2f m");
		}
		ImGui::EndCombo();
	}

	ImGui::SameLine();
	ImGui::Checkbox("Lights##ImGuizmoFunctionality", &engine->renderer_em->rend_lights);
	ImGui::SameLine();
	ImGui::Checkbox("Grid##ImGuizmoFunctionality", &render_grid);
	ImGui::SameLine();
	ImGui::Checkbox("Colliders##ImGuizmoFunctionality", &engine->renderer_em->rend_colliders);

	ImGui::SameLine();
	ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - 170, 0));

	ImGui::SameLine();
	ImGui::Text("Background:");
	ImGui::SameLine();
	ML_Color bg_color = engine->renderer_em->GetBackGroundColor();
	ImVec4 background_color = ImVec4(
		(float)((float)bg_color.r / 255),
		(float)((float)bg_color.g / 255),
		(float)((float)bg_color.b / 255),
		(float)((float)bg_color.a / 255));

	ImGui::ColorEdit3("##BackgroundColor",
		&background_color.x,
		ImGuiColorEditFlags_NoInputs |
		ImGuiColorEditFlags_NoLabel |
		ImGuiColorEditFlags_NoTooltip);

	engine->renderer_em->SetBackGroundColor({
		background_color.x * 255,
		background_color.y * 255,
		background_color.z * 255,
		background_color.w * 255
		});

	ImGui::SameLine();
	if (ImGui::Button("Default"))
	{
		engine->renderer_em->SetBackGroundColor({
			102,
			102,
			102,
			255
			});
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(450.0f);
		ImGui::TextUnformatted("Set Default Color to the Background");
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
}