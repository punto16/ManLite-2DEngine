#include "PanelInspector.h"

#include "GUI.h"
#include "EngineCore.h"
#include "SceneManagerEM.h"
#include "GameObject.h"
#include "Layer.h"
#include "Component.h"
#include "Transform.h"
#include "Camera.h"
#include "Sprite2D.h"

#include "FileDialog.h"

#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(disable : 4996)
#endif

#include <imgui.h>
#include <filesystem>
#include <exception>


PanelInspector::PanelInspector(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
}

PanelInspector::~PanelInspector()
{
}

bool PanelInspector::Update()
{
	bool ret = true;

	if (ImGui::Begin(name.c_str(), &enabled))
	{
		if (engine->scene_manager_em->GetCurrentScene().GetSelectedGOs().size() > 0)
		{
			GameObject& go = *(engine->scene_manager_em->GetCurrentScene().GetSelectedGOs()[0].lock());
			GeneralOptions(go);
			TransformOptions(go);
			CameraOptions(go);

			//last
			AddComponent(go);
		}
	}
	ImGui::End();

	return ret;
}

void PanelInspector::GeneralOptions(GameObject& go)
{
	//enable disable checkbox
	bool enabled = go.IsEnabled();
	ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.3);
	std::string isEnabledLabel = std::string("Enable##" + std::to_string(go.GetID()));
	ImGui::Checkbox(isEnabledLabel.c_str(), &enabled);
	go.SetEnabled(enabled);
	//game object name
	ImGui::SameLine();
	static char newNameBuffer[32];
	strcpy(newNameBuffer, go.GetName().c_str());
	uint input_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsNoBlank;
	std::string inputTextLabel = std::string("Name##" + std::to_string(go.GetID()));
	ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.7);
	if (ImGui::InputText(inputTextLabel.c_str(), newNameBuffer, sizeof(newNameBuffer), input_flags))
	{
		std::string new_name(newNameBuffer);
		new_name = GameObject::GenerateUniqueName(new_name, go.GetSharedPtr().get());
		LOG(LogType::LOG_INFO, "GameObject <%s> has been renamed to <%s>", go.GetName().c_str(), new_name.c_str());
		go.SetName(new_name);
		newNameBuffer[0] = '\0';
	}
	//game object tag
	ImGui::Separator();
	static char newTagBuffer[16];
	strcpy(newTagBuffer, go.GetTag().c_str());
	std::string inputTagLabel = std::string("Tag##" + std::to_string(go.GetID()));
	ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.3);
	if (ImGui::InputText(inputTagLabel.c_str(), newTagBuffer, sizeof(newTagBuffer), input_flags))
	{
		std::string new_tag(newTagBuffer);
		new_tag = GameObject::GenerateUniqueName(new_tag, go.GetSharedPtr().get());
		LOG(LogType::LOG_INFO, "GameObject Tag from <%s> to <%s>", go.GetTag().c_str(), new_tag.c_str());
		go.SetTag(new_tag);
		newTagBuffer[0] = '\0';
	}
	//dropbox game object layer
	ImGui::SameLine();
	static int selected_layer_idx = -1;
	auto& layers = engine->scene_manager_em->GetCurrentScene().GetSceneLayers();
	std::string preview = (selected_layer_idx >= 0 && selected_layer_idx < layers.size()) ?
		layers[selected_layer_idx]->GetLayerName().c_str() :
		"Select a Layer";
	if (go.GetParentLayer().lock() == nullptr) preview = "Select a Layer";
	if (auto parent_layer = go.GetParentLayer().lock()) {
		preview = parent_layer->GetLayerName();
		for (int i = 0; i < layers.size(); ++i)
		{
			if (layers[i] == parent_layer)
			{
				selected_layer_idx = i;
				break;
			}
		}
	}
	std::string layerLabel = std::string("Layer##" + std::to_string(go.GetID()));
	ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.7);
	if (ImGui::BeginCombo(layerLabel.c_str(), preview.c_str()))
	{
		for (int n = 0; n < layers.size(); ++n)
		{
			const bool is_selected = (selected_layer_idx == n);
			if (ImGui::Selectable(layers[n]->GetLayerName().c_str(), is_selected))
			{
				selected_layer_idx = n;
				engine->scene_manager_em->GetCurrentScene().ReparentToLayer(go.GetSharedPtr(), layers[n]);
			}
			if (is_selected) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0,4));
	//prefab options
	//if (go.IsPrefab())
	//{
	//	Prefab(go);
	//}
}

void PanelInspector::PrefabOptions(GameObject& go)
{
}

void PanelInspector::TransformOptions(GameObject& go)
{
	uint treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
	uint tableFlags = ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersInnerV;
	Transform* transform = go.GetComponent<Transform>();
	if (transform == nullptr) return;
	std::string transformLabel = std::string("Transform##" + std::to_string(go.GetID()));
	if (ImGui::CollapsingHeader(transformLabel.c_str(), treeFlags))
	{
		std::string transformTableLabel = std::string("TransformTable##" + std::to_string(go.GetID()));
		if (ImGui::BeginTable(transformTableLabel.c_str(), 3, tableFlags))
		{
			for (size_t row = 0; row < 3; row++)
			{
				ImGui::TableNextRow();
				for (size_t column = 0; column < 3; column++)
				{
					float dummy_size = 2.9f;
					ImGui::TableSetColumnIndex(column);
					if (column == 0 && row == 0)
					{
						ImGui::Text("Position");
						ImGui::Dummy(ImVec2(0, dummy_size));
						ImGui::Separator();
					}
					else if (column == 0 && row == 1)
					{
						ImGui::Text("Angle");
						ImGui::Dummy(ImVec2(0, dummy_size));
						ImGui::Separator();
					}
					else if (column == 0 && row == 2)
					{
						ImGui::Text("Scale");
						ImGui::SameLine();
						bool lock = transform->IsAspectRatioLocked();
						if (ImGui::Checkbox(std::string("##" + transformLabel + "keep_proportions_scale").c_str(), &lock))
						{
							transform->SetAspectRatioLock(lock);
						}
						ImGui::Dummy(ImVec2(0, dummy_size));
					}
					else if (column == 1 && row == 0)
					{
						float pos_x = transform->GetPosition().x;
						std::string pos_x_label = std::string("x##position_x" + std::to_string(go.GetID()));
						ImGui::DragFloat(pos_x_label.c_str(), &pos_x, 0.05f);
						transform->SetPosition(vec2f(pos_x, transform->GetPosition().y));
						ImGui::Separator();
					}
					else if (column == 1 && row == 1)
					{
						float angle = transform->GetAngle();
						std::string angle_label = std::string("##angle_degree" + std::to_string(go.GetID()));
						ImGui::DragFloat(angle_label.c_str(), &angle, 0.2f);
						transform->SetAngle(angle);
						ImGui::Separator();
					}
					else if (column == 1 && row == 2)
					{
						float scale_x = transform->GetScale().x;
						std::string scale_x_label = std::string("x##scale_x" + std::to_string(go.GetID()));
						ImGui::DragFloat(scale_x_label.c_str(), &scale_x, 0.05f);
						vec2f new_scale = transform->GetScale();
						new_scale.x = scale_x;
						transform->SetScale(new_scale);
					}
					else if (column == 2 && row == 0)
					{
						float pos_y = transform->GetPosition().y;
						std::string pos_y_label = std::string("y##position_y" + std::to_string(go.GetID()));
						ImGui::DragFloat(pos_y_label.c_str(), &pos_y, 0.05f);
						transform->SetPosition(vec2f(transform->GetPosition().x, pos_y));
						ImGui::Separator();
					}
					else if (column == 2 && row == 1)
					{
						ImGui::Text("Degree");
						ImGui::Dummy(ImVec2(0, dummy_size));
						ImGui::Separator();
					}
					else if (column == 2 && row == 2)
					{
						float scale_y = transform->GetScale().y;
						std::string scale_y_label = std::string("y##scale_y" + std::to_string(go.GetID()));
						ImGui::DragFloat(scale_y_label.c_str(), &scale_y, 0.05f);
						vec2f new_scale = transform->GetScale();
						new_scale.y = scale_y;
						transform->SetScale(new_scale);
					}
				}
			}
			ImGui::EndTable();
			if (ImGui::TreeNode(std::string("Log World Transform##" + std::to_string(go.GetID())).c_str()))
			{
				std::string world_transform_log = "World Position:   (%.2f,%.2f)\nWorld Angle:      (%.2f)\nWorld Scale:      (%.2f,%.2f)";
				ImGui::Text(world_transform_log.c_str(),
					transform->GetWorldPosition().x, transform->GetWorldPosition().y,
					transform->GetWorldAngle(),
					transform->GetWorldScale().x, transform->GetWorldScale().y);
				ImGui::TreePop();
			}
		}

		ImGui::Dummy(ImVec2(0, 4));
		ImGui::Separator();
	}
}

void PanelInspector::CameraOptions(GameObject& go)
{
	uint treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
	Camera* cam = go.GetComponent<Camera>();
	if (cam == nullptr) return;
	std::string camLabel = std::string("Camera##" + std::to_string(go.GetID()));
	if (ImGui::CollapsingHeader(camLabel.c_str(), treeFlags))
	{
		int cam_width, cam_height;
		cam->GetViewportSize(cam_width, cam_height);
		std::string cam_width_label = std::string("Viewport Width##" + std::to_string(go.GetID()));
		ImGui::DragInt(cam_width_label.c_str(), &cam_width, 1.0f);

		std::string cam_height_label = std::string("Viewport Height##" + std::to_string(go.GetID()));
		ImGui::DragInt(cam_height_label.c_str(), &cam_height, 1.0f);
		cam->SetViewportSize(cam_width, cam_height);

		int cam_zoom = (int)cam->GetZoom();
		std::string cam_zoom_label = std::string("Camera Zoom##" + std::to_string(go.GetID()));
		ImGui::DragInt(cam_zoom_label.c_str(), &cam_zoom, 1.0f, 10, 200);
		cam->SetZoom((float)cam_zoom);


		ImGui::Dummy(ImVec2(0, 4));
		ImGui::Separator();
	}
}

void PanelInspector::AddComponent(GameObject& go)
{
	const ImVec2 button_size_default = ImVec2(150, 0);
	ImGui::Dummy(ImVec2(0, 10));
	ImGui::Dummy(ImVec2((ImGui::GetWindowWidth() - button_size_default.x - 30) * 0.5, 0));
	ImGui::SameLine();
	static bool show_component_window = false;
	static ImVec2 window_pos;

	std::string button_id = "AddComponent_" + std::to_string(go.GetID());

	if (ImGui::Button("Add Component", button_size_default))
	{
		window_pos = ImGui::GetMousePos();
		show_component_window = true;
		ImGui::SetNextWindowPos(window_pos);
	}

	if (show_component_window)
	{
		ImGui::SetNextWindowSize(ImVec2(200, 0), ImGuiCond_Always);
		ImGui::Begin(
			"Add Component###ComponentSelector",
			&show_component_window,
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove
		);

		if (ImGui::Selectable("Camera"))
		{
			go.AddComponent<Camera>();
			show_component_window = false;
		}

		if (ImGui::Selectable("Sprite"))
		{
			std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Sprite file (*.png)\0*.png\0")).string();
			if (!filePath.empty() && filePath.ends_with(".png"))
			{
				go.AddComponent<Sprite2D>(filePath);
			}
			show_component_window = false;
		}

		if (!ImGui::IsWindowFocused())
		{
			show_component_window = false;
		}

		ImGui::End();
	}
}
	//std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Font file (*.ttf)\0*.ttf\0")).string();
	//if (!filePath.empty() && filePath.ends_with(".ttf"))
	//{
	//	tempTextUI->SetFont(filePath.c_str());
	//}
