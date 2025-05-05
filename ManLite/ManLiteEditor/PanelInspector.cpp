#include "PanelInspector.h"

#include "App.h"

#include "GUI.h"
#include "PanelAnimation.h"
#include "PanelTileMap.h"
#include "EngineCore.h"
#include "SceneManagerEM.h"
#include "GameObject.h"
#include "Layer.h"
#include "Component.h"
#include "Transform.h"
#include "Camera.h"
#include "Sprite2D.h"
#include "Animator.h"
#include "Animation.h"
#include "AudioSource.h"
#include "Collider2D.h"
#include "Canvas.h"
#include "ImageUI.h"
#include "ButtonImageUI.h"
#include "CheckBoxUI.h"
#include "SliderUI.h"
#include "TextUI.h"
#include "ParticleSystem.h"
#include "Emitter.h"
#include "TileMap.h"
#include "Script.h"

#include "ResourceManager.h"

#include "FileDialog.h"

#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(disable : 4996)
#endif

#include <imgui.h>
#include <filesystem>
#include <exception>

namespace fs = std::filesystem;


PanelInspector::PanelInspector(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
}

PanelInspector::~PanelInspector()
{
}

bool PanelInspector::Update()
{
	if (bringToFront)
	{
		ImGui::SetNextWindowFocus();
		bringToFront = false;
	}

	bool ret = true;

	if (ImGui::Begin(name.c_str(), &enabled))
	{
		if (engine->scene_manager_em->GetCurrentScene().GetSelectedGOs().size() > 0)
		{
			GameObject& go = *(engine->scene_manager_em->GetCurrentScene().GetSelectedGOs()[0].lock());
			if (engine->scene_manager_em->GetCurrentScene().GetSelectedGOs()[0].lock() != nullptr)
			{
				GeneralOptions(go);
				TransformOptions(go);
				CameraOptions(go);
				SpriteOptions(go);
				AnimatorOptions(go);
				AudioSourceOptions(go);
				Collider2DOptions(go);
				CanvasOptions(go);
				ParticleSystemOptions(go);
				TileMapOptions(go);
				ScriptsOptions(go);

				//last
				AddComponent(go);
			}
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

	bool header_open = ImGui::CollapsingHeader(camLabel.c_str(), treeFlags);

	if (ImGui::BeginPopupContextItem())
	{
		std::string context_label = "Remove Component##" + camLabel;
		if (ImGui::MenuItem(context_label.c_str()))
		{
			go.RemoveComponent(ComponentType::Camera);
			ImGui::EndPopup();
			return;
		}
		ImGui::EndPopup();
	}

	if (header_open)
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
		ImGui::DragInt(cam_zoom_label.c_str(), &cam_zoom, 1.0f, 1, 1000);
		cam->SetZoom((float)cam_zoom);


		ImGui::Dummy(ImVec2(0, 4));
		ImGui::Separator();
	}
}

void PanelInspector::SpriteOptions(GameObject& go)
{
	const uint treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
	Sprite2D* sprite = go.GetComponent<Sprite2D>();
	if (!sprite) return;

	const std::string headerLabel = "Sprite2D##" + std::to_string(go.GetID());

	bool header_open = ImGui::CollapsingHeader(headerLabel.c_str(), treeFlags);

	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem(("Remove Component##" + headerLabel).c_str()))
		{
			go.RemoveComponent(ComponentType::Sprite);
			ImGui::EndPopup();
			return;
		}
		ImGui::EndPopup();
	}

	if (header_open)
	{
		ImGui::Dummy(ImVec2(0, 4));

		if (ImGui::BeginTable("SpriteSettings", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit))
		{
			ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 100.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Preview");
			ImGui::TableSetColumnIndex(1);

			int texWidth, texHeight;
			sprite->GetTextureSize(texWidth, texHeight);
			const float aspectRatio = static_cast<float>(texHeight) / texWidth;

			ImGui::Image(
				sprite->GetTextureID(),
				ImVec2(100, 100 * aspectRatio),
				ImVec2(0, 1),
				ImVec2(1, 0),
				ImVec4(1, 1, 1, 1),
				ImVec4(0.9f, 0.9f, 0.9f, 0.5f)
			);

			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_IMAGE");
				if (payload)
				{
					const char* payload_path = static_cast<const char*>(payload->Data);
					std::string dragged_path(payload_path);
					std::replace(dragged_path.begin(), dragged_path.end(), '/', '\\');
					if (dragged_path.ends_with(".png")) sprite->SwapTexture(dragged_path);
					else LOG(LogType::LOG_WARNING, "Wrong Sprite Format. Correct format to import Sprite file is .png");
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::SameLine();
			if (ImGui::Button(("Change##" + headerLabel).c_str(), ImVec2(60, 24)))
			{
				std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Sprite file (*.png)\0*.png\0", "Assets\\Textures")).string();

				if (!filePath.empty() && filePath.ends_with(".png"))
				{
					sprite->SwapTexture(filePath);
				}
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Texture Path");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", sprite->GetTexturePath().c_str());

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Texture Size");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%d x %d", texWidth, texHeight);
			ImGui::SameLine();
			Gui::HelpMarker("Original texture dimensions");

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Pixel Art");
			ImGui::TableSetColumnIndex(1);
			bool pixelArt = sprite->IsPixelArt();
			if (ImGui::Checkbox(("##PixelArt" + headerLabel).c_str(), &pixelArt))
			{
				sprite->SetPixelArtRender(pixelArt);
			}
			ImGui::SameLine();
			Gui::HelpMarker("Enable nearest neighbor filtering\nfor pixel-perfect rendering");

			if (ImGui::TreeNodeEx("Texture Section", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ML_Rect section = sprite->GetTextureSection();
				int values[4] = { section.x, section.y, section.w, section.h };

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Section (XYWH)");
				ImGui::TableSetColumnIndex(1);

				if (ImGui::DragInt4(("##Section" + headerLabel).c_str(), values, 1.0f))
				{
					sprite->SetTextureSection(
						values[0],
						values[1],
						values[2],
						values[3]
					);
				}
				ImGui::TreePop();
			}



			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Offset");
			ImGui::TableSetColumnIndex(1);
			vec2f offset = sprite->GetOffset();
			if (ImGui::DragFloat("X##OffsetSprite", &offset.x, 0.001f, -10000.0f, 10000.0f)) {
				sprite->SetOffset(offset);
			}
			if (ImGui::DragFloat("Y##OffsetSprite", &offset.y, 0.001f, -10000.0f, 10000.0f)) {
				sprite->SetOffset(offset);
			}

			ImGui::EndTable();
		}

		ImGui::Dummy(ImVec2(0, 8));
		ImGui::Separator();
	}
}

void PanelInspector::AnimatorOptions(GameObject& go)
{
	uint treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
	Animator* animator = go.GetComponent<Animator>();
	if (animator == nullptr) return;
	Sprite2D* sprite = animator->GetContainerGO()->GetComponent<Sprite2D>();

	std::string animatorLabel = std::string("Animator##" + std::to_string(go.GetID()));

	ImGui::BeginGroup();
	bool header_open = ImGui::CollapsingHeader(animatorLabel.c_str(), treeFlags);

	if (ImGui::BeginPopupContextItem())
	{
		std::string context_label = "Remove Component##" + animatorLabel;
		if (ImGui::MenuItem(context_label.c_str()))
		{
			go.RemoveComponent(ComponentType::Animator);
			ImGui::EndPopup();
			return;
		}
		ImGui::EndPopup();
	}

	if (header_open)
	{
		ImGui::Dummy(ImVec2(0, 5));

		if (ImGui::BeginTable("AnimationsTable", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchSame))
		{
			ImGui::TableSetupColumn("Header", ImGuiTableColumnFlags_WidthFixed, 100.0f);
			ImGui::TableSetupColumn("Content", ImGuiTableColumnFlags_WidthStretch);

			for (const auto& animation_map : animator->GetAnimations())
			{
				std::string animation_name = animation_map.first;
				std::string animation_path = animation_map.second.filePath;
				Animation* animation = animation_map.second.animation;
				bool is_selected = (selected_animation == animation_name);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::BeginGroup();
				{
					const bool isPlaying = (animator->GetCurrentAnimationName() == animation_name);
					ImGui::TextColored(isPlaying ? ImVec4(0, 1, 0, 1) : ImVec4(0.5f, 0.5f, 0.5f, 1),
						isPlaying ? "Playing" : "Stopped");

					ImGui::SameLine();
					ImGui::TextDisabled("(%d fr)", animation->totalFrames);

					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Text("Duration: %.1f", animation->totalFrames * animation->speed * app->GetDT() * 60);
						ImGui::Text("Speed: %.1f", animation->speed);
						ImGui::Text("Loop: %s", animation->loop ? "Yes" : "No");
						ImGui::EndTooltip();
					}
				}
				ImGui::EndGroup();

				ImGui::TableSetColumnIndex(1);


				ImGui::TableSetColumnIndex(1);

				if (ImGui::Selectable(animation_name.c_str(), is_selected, ImGuiSelectableFlags_AllowDoubleClick))
				{
					selected_animation = animation_name;
				}
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					selected_animation = animation_name;
					app->gui->animation_panel->SetAnimation(animation_path);
					app->gui->animation_panel->SetSprite(sprite->GetTexturePath());

				}

				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Remove Animation"))
					{
						animator->RemoveAnimation(animation_name);
						ImGui::EndPopup();
						break;
					}
					ImGui::EndPopup();
				}

				if (is_selected)
				{
					ImGui::Indent(10.0f);
					ImGui::Spacing();

					if (ImGui::Button("Play", ImVec2(60, 0))) animator->Play(selected_animation);
					ImGui::SameLine();
					if (ImGui::Button("Stop", ImVec2(60, 0))) if (animator->GetCurrentAnimationName() == selected_animation) animator->Stop();
					ImGui::SameLine();
					if (ImGui::Button("Edit", ImVec2(60, 0)))
					{
						app->gui->animation_panel->RequestFocus();
						selected_animation = animation_name;
						app->gui->animation_panel->SetAnimation(animation_path);
						app->gui->animation_panel->SetSprite(sprite->GetTexturePath());
					}

					ImGui::Spacing();
					ImGui::DragFloat("Preview Size", &image_animation_size, 3.0f, 10.0f, 2000.0f, "%.0f px");

					if (sprite && sprite->GetTextureID() != 0)
					{
						int w, h;
						sprite->GetTextureSize(w, h);

						animation->Update(app->GetDT(), this->currentFrame);
						ML_Rect section = animation->GetCurrentFrame(this->currentFrame);

						if (animation->totalFrames <= 0 || section.w <= 0 || section.h <= 0)
						{
							section.w = w;
							section.h = h;
						}

						ML_Rect uvs = PanelAnimation::GetUVs(section, w, h);

						float aspectRatio = (section.h > 0) ? (float)section.w / section.h : 1.0f;
						float previewHeight = image_animation_size / aspectRatio;

						ImGui::Text("Texture: %dx%d | Section: %.0f,%.0f,%.0f,%.0f",
							w, h, section.x, section.y, section.w, section.h);

						ImGui::Image(
							(ImTextureID)sprite->GetTextureID(),
							ImVec2(image_animation_size, previewHeight),
							ImVec2(uvs.x, uvs.y),
							ImVec2(uvs.w, uvs.h),
							ImVec4(1, 1, 1, 1),
							ImVec4(0.8f, 0.8f, 0.8f, 0.5f)
						);
					}
					else
					{
						ImGui::TextColored(ImVec4(1, 0, 0, 1), "No valid sprite/texture found!");
					}

					ImGui::Unindent(10.0f);
				}
			}
			ImGui::EndTable();
		}

		ImGui::Dummy(ImVec2(0, 10));
		ImGui::Separator();
		ImGui::Dummy(ImVec2(0, 5));

		float buttonWidth = (ImGui::GetContentRegionAvail().x - 10) * 0.5f;
		if (ImGui::Button("Add Animation", ImVec2(buttonWidth, 0)))
		{
			std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Animation file (*.animation)\0*.animation\0", "Assets\\Animations")).string();
			if (!filePath.empty() && filePath.ends_with(".animation"))
			{
				std::string animName = fs::path(filePath).stem().string();

				if (!animator->HasAnimation(animName))
				{
					animator->AddAnimation(animName, filePath);
					selected_animation = animName;
					if (animator->GetAnimations().size() == 1)
					{
						animator->Play(filePath);
					}
				}
				else
				{
					LOG(LogType::LOG_WARNING, "Animation %s already exists!", animName.c_str());
					if (animator->GetAnimations().size() == 1) animator->Play(filePath);
				}
			}
		}

		ImGui::SameLine(0, 10);

		if (ImGui::Button("Create New", ImVec2(buttonWidth, 0)))
		{
			std::string filePath = std::filesystem::relative(FileDialog::SaveFile("Save Animation file (*.animation)\0*.animation\0", "Assets\\Animations")).string();
			if (!filePath.empty())
			{
				std::string animName = fs::path(filePath).stem().string();
				if (!filePath.ends_with(".animation")) filePath += ".animation";
				Animation a;
				if (a.SaveToFile(filePath))
					LOG(LogType::LOG_OK, "Animation file saved to: %s", filePath.c_str());

				animator->AddAnimation(animName, filePath);
			}
		}

		ImGui::Dummy(ImVec2(0, 4));
		ImGui::Separator();
	}
	ImGui::EndGroup();

	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_ANIMATION");
		if (payload)
		{
			const char* payload_path = static_cast<const char*>(payload->Data);
			std::string dragged_path(payload_path);
			std::string dragged_name = fs::path(dragged_path).stem().string();
			std::replace(dragged_path.begin(), dragged_path.end(), '/', '\\');

			if (dragged_path.ends_with(".animation"))
				animator->AddAnimation(dragged_name, dragged_path);
			else
				LOG(LogType::LOG_WARNING, "Wrong Animation Format. Correct format to import Animation file is .animation");
		}
		ImGui::EndDragDropTarget();
	}
}

void PanelInspector::AudioSourceOptions(GameObject& go)
{
	const uint treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
	AudioSource* audio = go.GetComponent<AudioSource>();
	if (audio == nullptr) return;

	const std::string headerLabel = "Audio Source##" + std::to_string(go.GetID());
	ImGui::BeginGroup();
	bool header_open = ImGui::CollapsingHeader(headerLabel.c_str(), treeFlags);

	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem(("Remove Component##" + headerLabel).c_str()))
		{
			go.RemoveComponent(ComponentType::AudioSource);
			ImGui::EndPopup();
			return;
		}
		ImGui::EndPopup();
	}

	if (header_open)
	{
		ImGui::Dummy(ImVec2(0, 4));

		auto DrawAudioSection = [&](const char* sectionName, auto& audioMap, const char* type) {
			const float indent = ImGui::GetStyle().IndentSpacing;
			const ImVec4 headerColor = (strcmp(type, "Music") == 0) ?
				ImVec4(0.2f, 0.6f, 1.0f, 1.0f) : ImVec4(0.8f, 0.4f, 0.1f, 1.0f);

			ImGui::PushStyleColor(ImGuiCol_Header, headerColor);
			bool audio_type_header_open = ImGui::CollapsingHeader(sectionName, ImGuiTreeNodeFlags_DefaultOpen);
			if (audio_type_header_open)
			{
				if (audioMap.empty())
				{
					ImGui::Indent(indent);
					ImGui::TextDisabled("No %s files loaded", type);
					ImGui::Unindent(indent);
				}
				else
				{
					for (auto& [name, audioRef] : audioMap)
					{
						const std::string label = name + "##" + std::to_string(audioRef.volume);
						bool isSelected = (selected_audio == name);

						ImGui::Indent(indent);
						ImGui::Selectable(label.c_str(), isSelected,
							ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SpanAllColumns);

						if (ImGui::BeginPopupContextItem())
						{
							if (ImGui::MenuItem("Remove"))
							{
								if constexpr (std::is_same_v<decltype(audioRef), MusicRef>)
									audio->RemoveMusic(name);
								else
									audio->RemoveSound(name);
								selected_audio.clear();
								ImGui::EndPopup();
								ImGui::Unindent(indent);
								ImGui::PopStyleColor();
								return;
							}
							ImGui::EndPopup();
						}

						if (isSelected)
						{
							ImGui::Indent(indent);
							if (ImGui::BeginTable("AudioSettings", 2,
								ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchSame))
							{
								ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 100.0f);
								ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

								// Loop setting
								ImGui::TableNextRow();
								ImGui::TableSetColumnIndex(0);
								ImGui::Text("Loop");
								ImGui::TableSetColumnIndex(1);
								ImGui::Checkbox("##Loop", &audioRef.loop);
								ImGui::SameLine();
								Gui::HelpMarker("Repeat audio continuously");

								// Play on awake
								ImGui::TableNextRow();
								ImGui::TableSetColumnIndex(0);
								ImGui::Text("Play On Awake");
								ImGui::TableSetColumnIndex(1);
								ImGui::Checkbox("##PlayAwake", &audioRef.play_on_awake);
								ImGui::SameLine();
								Gui::HelpMarker("Play automatically when scene starts");

								// Volume control
								ImGui::TableNextRow();
								ImGui::TableSetColumnIndex(0);
								ImGui::Text("Volume");
								ImGui::TableSetColumnIndex(1);
								int vol = audioRef.volume;
								if (ImGui::SliderInt("##Vol", &vol, 0, 100, "%d%%",
									ImGuiSliderFlags_AlwaysClamp))
								{
									if constexpr (std::is_same_v<decltype(audioRef), MusicRef>)
										audio->SetMusicVolume(name, vol);
									else
										audio->SetSoundVolume(name, vol);
								}

								if constexpr (std::is_same_v<decltype(audioRef), SoundRef>)
								{
									ImGui::TableNextRow();
									ImGui::TableSetColumnIndex(0);
									ImGui::Text("Spatial Sound");
									ImGui::TableSetColumnIndex(1);
									bool spatial = audioRef.spatial;
									if (ImGui::Checkbox("##Spatial", &spatial))
									{
										audioRef.spatial = spatial;
									}
									ImGui::SameLine();
									Gui::HelpMarker("Enable 3D positional audio effect");

									if (audioRef.spatial)
									{
										ImGui::TableNextRow();
										ImGui::TableSetColumnIndex(0);
										ImGui::Text("Max Distance");
										ImGui::TableSetColumnIndex(1);
										int distance = audioRef.spatial_distance;
										if (ImGui::DragInt("##Distance", &distance, 1, 0, 5000))
										{
											audioRef.spatial_distance = std::clamp(distance, 0, 5000);
										}

										ImGui::TableNextRow();
										ImGui::TableSetColumnIndex(0);
										ImGui::Text("Listener");
										ImGui::TableSetColumnIndex(1);
										GameObject* listener_go = audioRef.listener.lock().get();
										if (!listener_go && audioRef.listener_id != 0)
										{
											audio->SetListener(name, engine->scene_manager_em->GetCurrentScene().FindGameObjectByID(audioRef.listener_id));
											listener_go = audioRef.listener.lock().get();
										}
										ImGui::Button(listener_go ? listener_go->GetName().c_str() : "Default");
										if (ImGui::BeginDragDropTarget())
										{
											ImGui::PushStyleColor(ImGuiCol_DragDropTarget, IM_COL32(45, 85, 230, 255));
											if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_MULTI_NODE"))
											{
												GameObject* droppedGo = *(GameObject**)payload->Data;
												if (droppedGo)
												{
													audio->SetListener(name, droppedGo->GetSharedPtr());
												}
											}
											ImGui::PopStyleColor();
											ImGui::EndDragDropTarget();
										}
										if (listener_go && ImGui::BeginPopupContextItem())
										{
											std::string context_label = "Remove Listener##" + std::to_string(listener_go->GetID());
											if (ImGui::MenuItem(context_label.c_str()))
											{
												audio->RemoveListener(name);
											}
											ImGui::EndPopup();
										}
										ImGui::SameLine();
										Gui::HelpMarker("Default listener is the current scene camera\nYou can drag and drop a game object to set it as the listener");
									}
								}

								ImGui::EndTable();
							}

							// Playback controls
							const float buttonWidth = (ImGui::GetContentRegionAvail().x -
								ImGui::GetStyle().ItemSpacing.x) * 0.5f;

							if (ImGui::Button("Play", ImVec2(buttonWidth, 0)))
							{
								if constexpr (std::is_same_v<decltype(audioRef), MusicRef>)
									audio->PlayMusic(name);
								else
									audio->PlaySound(name);
							}

							ImGui::SameLine();

							if (ImGui::Button("Stop", ImVec2(buttonWidth, 0)))
							{
								if constexpr (std::is_same_v<decltype(audioRef), MusicRef>)
									audio->StopMusic(name);
								else
									audio->StopSound(name);
							}

							ImGui::Unindent(indent);
							ImGui::Separator();
						}

						if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
							selected_audio = isSelected ? "" : name;

						ImGui::Unindent(indent);
					}
				}
			}
			ImGui::PopStyleColor();
			};

		DrawAudioSection("Music", audio->GetMusics(), "Music");
		ImGui::Dummy(ImVec2(0, 5));

		DrawAudioSection("Sound Effects", audio->GetSounds(), "Sound");
		ImGui::Dummy(ImVec2(0, 5));

		const float buttonWidth = (ImGui::GetContentRegionAvail().x -
			ImGui::GetStyle().ItemSpacing.x) * 0.5f;

		if (ImGui::Button("Add Music", ImVec2(buttonWidth, 0)))
		{
			std::string filePath = std::filesystem::relative(
				FileDialog::OpenFile("Music Files (*.ogg)\0*.ogg\0", "Assets\\Audio\\Music")).string();

			if (!filePath.empty() && filePath.ends_with(".ogg"))
			{
				const std::string audioName = std::filesystem::path(filePath).stem().string();
				audio->AddMusic(audioName, filePath);
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Add Sound", ImVec2(buttonWidth, 0)))
		{
			std::string filePath = std::filesystem::relative(
				FileDialog::OpenFile("Sound Files (*.wav)\0*.wav\0", "Assets\\Audio\\SoundEffects")).string();

			if (!filePath.empty() && filePath.ends_with(".wav"))
			{
				const std::string audioName = std::filesystem::path(filePath).stem().string();
				audio->AddSound(audioName, filePath);
			}
		}

		ImGui::Dummy(ImVec2(0, 4));
		ImGui::Separator();
	}
	ImGui::EndGroup();

	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_AUDIO");
		if (payload)
		{
			const char* payload_path = static_cast<const char*>(payload->Data);
			std::string dragged_path(payload_path);
			std::string dragged_name = fs::path(dragged_path).stem().string();
			std::replace(dragged_path.begin(), dragged_path.end(), '/', '\\');

			if (dragged_path.ends_with(".ogg"))
				audio->AddMusic(dragged_name, dragged_path);
			else if (dragged_path.ends_with(".wav"))
				audio->AddSound(dragged_name, dragged_path);
			else
				LOG(LogType::LOG_WARNING, "Wrong Audio Format. Correct format to import Audio file is .wav and .ogg");
		}
		ImGui::EndDragDropTarget();
	}
}

void PanelInspector::Collider2DOptions(GameObject& go)
{
	uint treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
	uint tableFlags = ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersInnerV;
	Collider2D* collider2d = go.GetComponent<Collider2D>();
	if (collider2d == nullptr) return;
	std::string collider2dLabel = std::string("Collider2D##" + std::to_string(go.GetID()));

	bool header_open = ImGui::CollapsingHeader(collider2dLabel.c_str(), treeFlags);

	if (ImGui::BeginPopupContextItem())
	{
		std::string context_label = "Remove Component##" + collider2dLabel;
		if (ImGui::MenuItem(context_label.c_str()))
		{
			go.RemoveComponent(ComponentType::Collider2D);
			ImGui::EndPopup();
			return;
		}
		ImGui::EndPopup();
	}

	if (header_open)
	{
		ImGui::Dummy(ImVec2(0, 4));

		if (ImGui::BeginTable("Collider2DTable", 2, ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 100.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Shape Type");
			ImGui::TableSetColumnIndex(1);

			const char* shapeTypes[] = { "Rectangle", "Circle" };
			int currentShape = static_cast<int>(collider2d->GetShapeType());
			if (ImGui::Combo("##ShapeType", &currentShape, shapeTypes, IM_ARRAYSIZE(shapeTypes)))
			{
				collider2d->SetShapeType(static_cast<ShapeType>(currentShape));
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Color");
			ImGui::TableSetColumnIndex(1);

			ML_Color color = collider2d->GetColor();
			ImVec4 imColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
			if (ImGui::ColorEdit4("##ColliderColor", (float*)&imColor, ImGuiColorEditFlags_NoInputs))
			{
				color.r = static_cast<uint8_t>(imColor.x * 255);
				color.g = static_cast<uint8_t>(imColor.y * 255);
				color.b = static_cast<uint8_t>(imColor.z * 255);
				color.a = static_cast<uint8_t>(imColor.w * 255);
				collider2d->SetColor(color);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Body Type");
			ImGui::TableSetColumnIndex(1);

			bool isDynamic = collider2d->IsDynamic();
			if (ImGui::Checkbox("Is Dynamic##Collider", &isDynamic))
			{
				collider2d->SetDynamic(isDynamic);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Sensor Mode");
			ImGui::TableSetColumnIndex(1);

			bool isSensor = collider2d->IsSensor();
			if (ImGui::Checkbox("Is Sensor##Collider", &isSensor))
			{
				collider2d->SetSensor(isSensor);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Lock Rotation");
			ImGui::TableSetColumnIndex(1);

			bool lockRotation = collider2d->GetLockRotation();
			if (ImGui::Checkbox("##LockRotation", &lockRotation))
			{
				collider2d->SetLockRotation(lockRotation);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Mass");
			ImGui::TableSetColumnIndex(1);

			float mass = collider2d->GetMass();
			if (ImGui::DragFloat("##Mass", &mass, 0.1f, 0.0f, 1000.0f, "%.2f kg"))
			{
				collider2d->SetMass(mass);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Linear Damping");
			ImGui::TableSetColumnIndex(1);

			float damping = collider2d->GetLinearDamping();
			if (ImGui::DragFloat("##Damping", &damping, 0.01f, 0.0f, 5.0f, "%.2f"))
			{
				collider2d->SetLinearDamping(damping);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Friction");
			ImGui::TableSetColumnIndex(1);

			float friction = collider2d->GetFriction();
			if (ImGui::SliderFloat("##Friction", &friction, 0.0f, 1.0f, "%.2f"))
			{
				collider2d->SetFriction(friction);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Bounce");
			ImGui::SameLine();
			Gui::HelpMarker("Bounce coefficient:\n  0 for NOT bounce\n  1 for same energy bounce\n  +1 for more energy bounce");
			ImGui::TableSetColumnIndex(1);

			float restitution = collider2d->GetRestitution();
			if (ImGui::DragFloat("Restitution##Bounce", &restitution, 0.01f, 0.0f, 100.0f))
			{
				collider2d->SetRestitution(restitution);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Use Gravity");
			ImGui::TableSetColumnIndex(1);

			bool useGravity = collider2d->GetUseGravity();
			if (ImGui::Checkbox("##UseGravity", &useGravity))
			{
				collider2d->SetUseGravity(useGravity);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Dimensions");
			ImGui::TableSetColumnIndex(1);

			if (collider2d->GetShapeType() == ShapeType::RECTANGLE) {
				float width = collider2d->GetWidth();
				float height = collider2d->GetHeight();

				if (ImGui::DragFloat("Width##Collider", &width, 0.05f, 0.1f, 10000.0f)) {
					collider2d->SetSize(width, height);
				}
				if (ImGui::DragFloat("Height##Collider", &height, 0.05f, 0.1f, 10000.0f)) {
					collider2d->SetSize(width, height);
				}
			}
			else {
				float radius = collider2d->GetRadius();
				if (ImGui::DragFloat("Radius##Collider", &radius, 0.05f, 0.1f, 10000.0f)) {
					collider2d->SetRadius(radius);
				}
			}

			ImGui::EndTable();
		}
		ImGui::Dummy(ImVec2(0, 4));
		ImGui::Separator();
	}
}

void PanelInspector::CanvasOptions(GameObject& go)
{
	uint treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
	Canvas* canvas = go.GetComponent<Canvas>();
	if (canvas == nullptr) return;
	std::string canvasLabel = std::string("Canvas##" + std::to_string(go.GetID()));

	bool header_open = ImGui::CollapsingHeader(canvasLabel.c_str(), treeFlags);

	if (ImGui::BeginPopupContextItem())
	{
		std::string context_label = "Remove Component##" + canvasLabel;
		if (ImGui::MenuItem(context_label.c_str()))
		{
			go.RemoveComponent(ComponentType::Canvas);
			ImGui::EndPopup();
			return;
		}
		ImGui::EndPopup();
	}

	if (header_open)
	{
		//ui elements
		ImGui::Dummy(ImVec2(0, 4));
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "UI Elements:");

		auto& ui_elements = canvas->GetUIElements();

		for (size_t i = 0; i < ui_elements.size(); ++i)
		{
			auto& ui_element = ui_elements[i];
			std::string elementLabel = ui_element->GetName() +
				"##" + std::to_string(ui_element->GetID());

			if (ImGui::ArrowButton(("##up" + std::to_string(ui_element->GetID())).c_str(), ImGuiDir_Up)) {
				if (i > 0) std::swap(ui_elements[i], ui_elements[i - 1]);
				break;
			}
			ImGui::SameLine();
			if (ImGui::ArrowButton(("##down" + std::to_string(ui_element->GetID())).c_str(), ImGuiDir_Down)) {
				if (i < ui_elements.size() - 1) std::swap(ui_elements[i], ui_elements[i + 1]);
				break;
			}
			ImGui::SameLine();

			bool ui_element_header_open = ImGui::TreeNodeEx(elementLabel.c_str(), treeFlags);

			if (ImGui::BeginPopupContextItem())
			{
				std::string context_label = "Remove UIElement##" + elementLabel;
				if (ImGui::MenuItem(context_label.c_str()))
				{
					canvas->RemoveItemUI(ui_element->GetID());
					ImGui::EndPopup();
					if (ui_element_header_open) ImGui::TreePop();
					return;
				}
				ImGui::EndPopup();
			}

			if (ui_element_header_open)
			{
				std::string nameLabel = "Name##" + std::to_string(ui_element->GetID());
				char nameBuffer[32];
				strcpy(nameBuffer, ui_element->GetName().c_str());
				if (ImGui::InputText(nameLabel.c_str(), nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsNoBlank))
				{
					ui_element->SetName(nameBuffer);
				}

				std::string enabledLabel = "Enabled##" + std::to_string(ui_element->GetID());
				bool enabled = ui_element->IsEnabled();
				if (ImGui::Checkbox(enabledLabel.c_str(), &enabled))
				{
					ui_element->SetEnabled(enabled);
				}

				vec2f position = ui_element->GetPosition();
				std::string posLabel = "Position##" + std::to_string(ui_element->GetID());
				if (ImGui::DragFloat2(posLabel.c_str(), &position.x, 0.05f))
				{
					ui_element->SetPosition(position);
				}

				float angle = ui_element->GetAngle();
				std::string angleLabel = "Angle##" + std::to_string(ui_element->GetID());
				if (ImGui::DragFloat(angleLabel.c_str(), &angle, 0.1f))
				{
					ui_element->SetAngle(angle);
				}

#pragma region SCALE
				vec2f scale = ui_element->GetScale();
				std::string scaleLabel = "Scale##" + std::to_string(ui_element->GetID());

				bool lockAspect = ui_element->IsAspectLocked();
				float aspectRatio = ui_element->GetLockedAspectRatio();

				if (ImGui::Checkbox(("Lock Aspect##" + std::to_string(ui_element->GetID())).c_str(), &lockAspect))
				{
					ui_element->SetAspectLocked(lockAspect);

					if (lockAspect)
					{
						aspectRatio = scale.x / scale.y;
						ui_element->SetLockedAspectRatio(aspectRatio);
					}
				}

				if (lockAspect)
				{
					ImGui::SameLine();
					ImGui::TextDisabled("(%.2f:1.00)", aspectRatio);

					ImGui::BeginGroup();
					{
						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.45f);

						if (ImGui::DragFloat(("X##ScaleX_" + std::to_string(ui_element->GetID())).c_str(),
							&scale.x, 0.01f, 0.01f, 10.0f, "%.2f"))
						{
							scale.y = scale.x / aspectRatio;
							ui_element->SetScale(scale);
						}

						ImGui::SameLine();

						if (ImGui::DragFloat(("Y##ScaleY_" + std::to_string(ui_element->GetID())).c_str(),
							&scale.y, 0.01f, 0.01f, 10.0f, "%.2f"))
						{
							scale.x = scale.y * aspectRatio;
							ui_element->SetScale(scale);
						}

						ImGui::PopItemWidth();
					}
					ImGui::EndGroup();
				}
				else
				{
					if (ImGui::DragFloat2(scaleLabel.c_str(), &scale.x, 0.01f, 0.01f, 10.0f, "%.2f"))
					{
						ui_element->SetScale(scale);
					}
				}
#pragma endregion SCALE


				if (ui_element->GetType() == UIElementType::Image)
				{
					ImageUI* imageUI = dynamic_cast<ImageUI*>(ui_element.get());
					if (imageUI)
					{
						ImGui::Separator();
						ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Image Properties");

						std::string texLabel = "Texture Path";
						ImGui::Text("%s: %s", texLabel.c_str(), imageUI->GetTexturePath().c_str());

						std::string changeTextureLabel = "Change Texture##" + std::to_string(ui_element->GetID());
						if (ImGui::Button(changeTextureLabel.c_str()))
						{
							std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Sprite file (*.png)\0*.png\0", "Assets\\Textures")).string();
							if (!filePath.empty() && filePath.ends_with(".png") && filePath != imageUI->GetTexturePath())
							{
								imageUI->SwapTexture(filePath);
							}
						}
						if (ImGui::BeginDragDropTarget())
						{
							const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_IMAGE");
							if (payload)
							{
								const char* payload_path = static_cast<const char*>(payload->Data);
								std::string dragged_path(payload_path);
								std::replace(dragged_path.begin(), dragged_path.end(), '/', '\\');
								if (dragged_path.ends_with(".png")) imageUI->SwapTexture(dragged_path);
								else LOG(LogType::LOG_WARNING, "Wrong Sprite Format. Correct format to import Sprite file is .png");
							}
							ImGui::EndDragDropTarget();
						}

						std::string sizeLabel = "Texture Size";
						ImGui::Text("%s: %d x %d", sizeLabel.c_str(), (int)imageUI->GetTextureSize().x, (int)imageUI->GetTextureSize().y);

						ML_Rect section = imageUI->GetSection();
						std::string sectionLabel = "Section##" + std::to_string(ui_element->GetID());
						if (ImGui::DragFloat4(sectionLabel.c_str(), &section.x, 1.0f))
						{
							imageUI->SetSection(section);
						}

						std::string pixelArtLabel = "Pixel Art##" + std::to_string(ui_element->GetID());
						bool pixel_art = imageUI->IsPixelArt();
						ImGui::Checkbox(pixelArtLabel.c_str(), &pixel_art);
						imageUI->SetIsPixelArt(pixel_art);
					}
				}
				if (ui_element->GetType() == UIElementType::ButtonImage)
				{
					ButtonImageUI* buttonImage = dynamic_cast<ButtonImageUI*>(ui_element.get());
					if (buttonImage)
					{
						ImGui::Separator();
						ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.4f, 1.0f), "Button Image Properties");

						std::string texLabel = "Texture Path";
						ImGui::Text("%s: %s", texLabel.c_str(), buttonImage->GetTexturePath().c_str());

						std::string changeTextureLabel = "Change Texture##" + std::to_string(ui_element->GetID());
						if (ImGui::Button(changeTextureLabel.c_str()))
						{
							std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Sprite file (*.png)\0*.png\0", "Assets\\Textures")).string();
							if (!filePath.empty() && filePath.ends_with(".png") && filePath != buttonImage->GetTexturePath())
							{
								buttonImage->SwapTexture(filePath);
							}
						}
						if (ImGui::BeginDragDropTarget())
						{
							const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_IMAGE");
							if (payload)
							{
								const char* payload_path = static_cast<const char*>(payload->Data);
								std::string dragged_path(payload_path);
								std::replace(dragged_path.begin(), dragged_path.end(), '/', '\\');
								if (dragged_path.ends_with(".png")) buttonImage->SwapTexture(dragged_path);
								else LOG(LogType::LOG_WARNING, "Wrong Sprite Format. Correct format to import Sprite file is .png");
							}
							ImGui::EndDragDropTarget();
						}

						std::string sizeLabel = "Texture Size";
						ImGui::Text("%s: %d x %d", sizeLabel.c_str(), (int)buttonImage->GetTextureSize().x, (int)buttonImage->GetTextureSize().y);

						std::string pixelArtLabel = "Pixel Art##" + std::to_string(ui_element->GetID());
						bool pixel_art = buttonImage->IsPixelArt();
						ImGui::Checkbox(pixelArtLabel.c_str(), &pixel_art);
						buttonImage->SetIsPixelArt(pixel_art);

						const char* buttonStates[] = { "Idle", "Hovered", "Selected", "Hovered+Selected", "Disabled" };
						int currentState = static_cast<int>(buttonImage->GetButtonSectionManager().button_state);
						std::string stateLabel = "Preview State##" + std::to_string(ui_element->GetID());
						if (ImGui::Combo(stateLabel.c_str(), &currentState, buttonStates, IM_ARRAYSIZE(buttonStates)))
						{
							buttonImage->GetButtonSectionManager().button_state = static_cast<ButtonState>(currentState);
						}

						auto DrawSection = [&](const char* label, ML_Rect& section) {
							std::string headerLabel = label + std::string("##") + std::to_string(ui_element->GetID());
							if (ImGui::TreeNodeEx(headerLabel.c_str(), treeFlags))
							{
								std::string dragLabel = "Section##" + std::string(label) + std::to_string(ui_element->GetID());
								ImGui::DragFloat4(dragLabel.c_str(), &section.x, 1.0f);
								ImGui::TreePop();
							}
							};

						DrawSection("Idle Section", buttonImage->GetButtonSectionManager().section_idle);
						DrawSection("Hovered Section", buttonImage->GetButtonSectionManager().section_hovered);
						DrawSection("Selected Section", buttonImage->GetButtonSectionManager().section_selected);
						DrawSection("Hovered+Selected Section", buttonImage->GetButtonSectionManager().section_hovered_selected);
						DrawSection("Disabled Section", buttonImage->GetButtonSectionManager().section_disabled);
					}
				}
				if (ui_element->GetType() == UIElementType::CheckBox)
				{
					CheckBoxUI* checkboxUI = dynamic_cast<CheckBoxUI*>(ui_element.get());
					if (checkboxUI)
					{
						ImGui::Separator();
						ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.8f, 1.0f), "CheckBox Properties");

						std::string texLabel = "Texture Path";
						ImGui::Text("%s: %s", texLabel.c_str(), checkboxUI->GetTexturePath().c_str());

						std::string changeTextureLabel = "Change Texture##" + std::to_string(ui_element->GetID());
						if (ImGui::Button(changeTextureLabel.c_str()))
						{
							std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Sprite file (*.png)\0*.png\0", "Assets\\Textures")).string();
							if (!filePath.empty() && filePath.ends_with(".png") && filePath != checkboxUI->GetTexturePath())
							{
								checkboxUI->SwapTexture(filePath);
							}
						}
						if (ImGui::BeginDragDropTarget())
						{
							const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_IMAGE");
							if (payload)
							{
								const char* payload_path = static_cast<const char*>(payload->Data);
								std::string dragged_path(payload_path);
								std::replace(dragged_path.begin(), dragged_path.end(), '/', '\\');
								if (dragged_path.ends_with(".png")) checkboxUI->SwapTexture(dragged_path);
								else LOG(LogType::LOG_WARNING, "Wrong Sprite Format. Correct format to import Sprite file is .png");
							}
							ImGui::EndDragDropTarget();
						}

						std::string sizeLabel = "Texture Size";
						ImGui::Text("%s: %d x %d", sizeLabel.c_str(), (int)checkboxUI->GetTextureSize().x, (int)checkboxUI->GetTextureSize().y);

						std::string pixelArtLabel = "Pixel Art##" + std::to_string(ui_element->GetID());
						bool pixel_art = checkboxUI->IsPixelArt();
						ImGui::Checkbox(pixelArtLabel.c_str(), &pixel_art);
						checkboxUI->SetIsPixelArt(pixel_art);

						std::string valueLabel = "Value##" + std::to_string(ui_element->GetID());
						bool currentValue = checkboxUI->GetValue();
						if (ImGui::Checkbox(valueLabel.c_str(), &currentValue))
						{
							checkboxUI->SetValue(currentValue);
						}

						const char* checkboxStates[] = { "Idle", "Hovered", "Selected", "Hovered+Selected", "Disabled" };
						int currentState = static_cast<int>(checkboxUI->GetSectionManager().checkbox_state);
						std::string stateLabel = "Interaction State##" + std::to_string(ui_element->GetID());
						if (ImGui::Combo(stateLabel.c_str(), &currentState, checkboxStates, IM_ARRAYSIZE(checkboxStates)))
						{
							checkboxUI->GetSectionManager().checkbox_state = static_cast<CheckBoxState>(currentState);
						}

						ImGui::Separator();
						ImGui::TextColored(ImVec4(1, 1, 0, 1), "State Sections");
						ImGui::Dummy(ImVec2(0, 5));

						auto DrawSectionGroup = [&](const char* stateName, bool forTrueState) {
							ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 10.0f);
							std::string headerLabel = std::string(stateName) + " Sections##" + std::to_string(ui_element->GetID());
							if (ImGui::TreeNodeEx(headerLabel.c_str(), treeFlags))
							{
								ML_Rect* sections[5] = {
									forTrueState ? &checkboxUI->GetSectionManager().section_idle_true : &checkboxUI->GetSectionManager().section_idle_false,
									forTrueState ? &checkboxUI->GetSectionManager().section_hovered_true : &checkboxUI->GetSectionManager().section_hovered_false,
									forTrueState ? &checkboxUI->GetSectionManager().section_selected_true : &checkboxUI->GetSectionManager().section_selected_false,
									forTrueState ? &checkboxUI->GetSectionManager().section_hovered_selected_true : &checkboxUI->GetSectionManager().section_hovered_selected_false,
									forTrueState ? &checkboxUI->GetSectionManager().section_disabled_true : &checkboxUI->GetSectionManager().section_disabled_false
								};

								const char* subStates[] = { "Idle", "Hovered", "Selected", "Hovered+Selected", "Disabled" };

								for (int i = 0; i < 5; ++i)
								{
									std::string subLabel = subStates[i] + std::string("##") + std::to_string(ui_element->GetID()) + stateName + std::to_string(i);
									if (ImGui::TreeNodeEx(subLabel.c_str(), treeFlags))
									{
										std::string dragLabel = "Section##" + std::string(subStates[i]) + std::to_string(ui_element->GetID());
										ImGui::DragFloat4(dragLabel.c_str(), &sections[i]->x, 1.0f);
										ImGui::TreePop();
									}
								}
								ImGui::TreePop();
							}
							ImGui::PopStyleVar();
							};

						ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.8f, 0.2f, 0.4f));
						DrawSectionGroup("When True", true);
						ImGui::PopStyleColor();

						ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.8f, 0.2f, 0.2f, 0.4f));
						DrawSectionGroup("When False", false);
						ImGui::PopStyleColor();
					}
				}
				if (ui_element->GetType() == UIElementType::Slider)
				{
					SliderUI* sliderUI = dynamic_cast<SliderUI*>(ui_element.get());
					if (sliderUI)
					{
						ImGui::Separator();
						ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.2f, 1.0f), "Slider Properties");

						std::string texLabel = "Texture Path";
						ImGui::Text("%s: %s", texLabel.c_str(), sliderUI->GetTexturePath().c_str());

						std::string changeTextureLabel = "Change Texture##" + std::to_string(ui_element->GetID());
						if (ImGui::Button(changeTextureLabel.c_str()))
						{
							std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Sprite file (*.png)\0*.png\0", "Assets\\Textures")).string();
							if (!filePath.empty() && filePath.ends_with(".png") && filePath != sliderUI->GetTexturePath())
							{
								sliderUI->SwapTexture(filePath);
							}
						}
						if (ImGui::BeginDragDropTarget())
						{
							const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_IMAGE");
							if (payload)
							{
								const char* payload_path = static_cast<const char*>(payload->Data);
								std::string dragged_path(payload_path);
								std::replace(dragged_path.begin(), dragged_path.end(), '/', '\\');
								if (dragged_path.ends_with(".png")) sliderUI->SwapTexture(dragged_path);
								else LOG(LogType::LOG_WARNING, "Wrong Sprite Format. Correct format to import Sprite file is .png");
							}
							ImGui::EndDragDropTarget();
						}

						std::string sizeLabel = "Texture Size";
						ImGui::Text("%s: %d x %d", sizeLabel.c_str(), (int)sliderUI->GetTextureSize().x, (int)sliderUI->GetTextureSize().y);

						std::string pixelArtLabel = "Pixel Art##" + std::to_string(ui_element->GetID());
						bool pixel_art = sliderUI->IsPixelArt();
						ImGui::Checkbox(pixelArtLabel.c_str(), &pixel_art);
						sliderUI->SetIsPixelArt(pixel_art);

						int minVal = sliderUI->GetMinValue();
						int maxVal = sliderUI->GetMaxValue();
						int currentVal = sliderUI->GetValue();

						ImGui::DragInt("Max Value", &maxVal, 0.1f, minVal, 100);
						ImGui::SliderInt("Current Value", &currentVal, minVal, maxVal);
						sliderUI->SetRange(minVal, maxVal);
						sliderUI->SetValue(currentVal);

						const char* styles[] = { "All Equal", "First/Last Different" };
						int currentStyle = static_cast<int>(sliderUI->GetSliderStyle());
						if (ImGui::Combo("Slider Style", &currentStyle, styles, IM_ARRAYSIZE(styles)))
						{
							sliderUI->SetSliderStyle(static_cast<SliderStyle>(currentStyle));
						}

						const char* alignments[] = { "Left", "Center", "Right" };
						int currentAlign = static_cast<int>(sliderUI->GetAlignment());
						if (ImGui::Combo("Alignment", &currentAlign, alignments, IM_ARRAYSIZE(alignments)))
						{
							sliderUI->SetAlignment(static_cast<SliderAlignment>(currentAlign));
						}

						float offset, offsetFirst, offsetLast;
						sliderUI->GetOffsets(offset, offsetFirst, offsetLast);
						ImGui::DragFloat("Main Offset", &offset, 0.01f);
						ImGui::DragFloat("First Offset", &offsetFirst, 0.01f);
						ImGui::DragFloat("Last Offset", &offsetLast, 0.01f);
						sliderUI->SetOffsets(offset, offsetFirst, offsetLast);

						const char* states[] = { "Idle", "Hovered", "Disabled" };
						int currentState = static_cast<int>(sliderUI->GetSliderState());
						if (ImGui::Combo("Interaction State", &currentState, states, IM_ARRAYSIZE(states)))
						{
							sliderUI->SetSliderState(static_cast<SliderState>(currentState));
						}

						auto DrawSliderSection = [&](const char* partName, SliderSectionPart& part) {
							ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 10.0f);
							std::string headerLabel = partName + std::string("##") + std::to_string(ui_element->GetID());
							if (ImGui::TreeNodeEx(headerLabel.c_str(), treeFlags))
							{
								ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.6f, 1.0f, 0.4f));
								if (ImGui::TreeNodeEx("True State", treeFlags))
								{
									ImGui::DragFloat4("Idle##True", &part.section_idle_true.x, 1.0f);
									ImGui::DragFloat4("Hovered##True", &part.section_hovered_true.x, 1.0f);
									ImGui::DragFloat4("Disabled##True", &part.section_disabled_true.x, 1.0f);
									ImGui::TreePop();
								}
								ImGui::PopStyleColor();

								ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(1.0f, 0.4f, 0.2f, 0.4f));
								if (ImGui::TreeNodeEx("False State", treeFlags))
								{
									ImGui::DragFloat4("Idle##False", &part.section_idle_false.x, 1.0f);
									ImGui::DragFloat4("Hovered##False", &part.section_hovered_false.x, 1.0f);
									ImGui::DragFloat4("Disabled##False", &part.section_disabled_false.x, 1.0f);
									ImGui::TreePop();
								}
								ImGui::PopStyleColor();

								ImGui::TreePop();
							}
							ImGui::PopStyleVar();
							};

						DrawSliderSection("Regular Part", sliderUI->GetRegularPart());
						if (sliderUI->GetSliderStyle() == SliderStyle::FIRST_AND_LAST_DIFFERENT)
						{
							DrawSliderSection("First Part", sliderUI->GetFirstPart());
							DrawSliderSection("Last Part", sliderUI->GetLastPart());
						}
					}
				}
				if (ui_element->GetType() == UIElementType::Text)
				{
					TextUI* textUI = dynamic_cast<TextUI*>(ui_element.get());
					if (textUI)
					{
						ImGui::Separator();
						ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.4f, 1.0f), "Text Properties");

						std::string textLabel = "Text Content##" + std::to_string(ui_element->GetID());
						char buffer[1024];
						strncpy(buffer, textUI->GetText().c_str(), sizeof(buffer));
						buffer[sizeof(buffer) - 1] = '\0';

						if (ImGui::InputTextMultiline(textLabel.c_str(), buffer, sizeof(buffer),
							ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4)
							))
						{
							textUI->SetText(buffer);
						}

						std::string fontLabel = "Font Path: " + textUI->GetFontPath();
						ImGui::Text("%s", fontLabel.c_str());

						std::string changeFontLabel = "Change Font##" + std::to_string(ui_element->GetID());
						if (ImGui::Button(changeFontLabel.c_str()))
						{
							std::string filePath = std::filesystem::relative(
								FileDialog::OpenFile("Font files (*.ttf)\0*.ttf\0", "Assets\\Fonts")
							).string();

							if (!filePath.empty() && filePath.ends_with(".ttf") && filePath != textUI->GetFontPath())
							{
								textUI->SwapFont(filePath);
							}
						}

						if (ImGui::BeginDragDropTarget())
						{
							const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_FONT");
							if (payload)
							{
								const char* payload_path = static_cast<const char*>(payload->Data);
								std::string dragged_path(payload_path);
								std::string dragged_name = fs::path(dragged_path).stem().string();
								std::replace(dragged_path.begin(), dragged_path.end(), '/', '\\');

								if (dragged_path.ends_with(".ttf"))
									textUI->SwapFont(dragged_path);
								else
									LOG(LogType::LOG_WARNING, "Wrong Font Format. Correct format to import Font file is .ttf");
							}
							ImGui::EndDragDropTarget();
						}

						static const char* alignmentItems[] = { "Left", "Centered", "Right" };
						int currentAlignment = static_cast<int>(textUI->GetTextAlignment());
						std::string alignmentLabel = "Alignment##" + std::to_string(ui_element->GetID());

						if (ImGui::Combo(alignmentLabel.c_str(), &currentAlignment, alignmentItems, IM_ARRAYSIZE(alignmentItems)))
						{
							textUI->SetTextAlignment(static_cast<TextAlignment>(currentAlignment));
						}

						std::string colorLabel = "Text Color##" + std::to_string(ui_element->GetID());
						float colorValues[4] = {
							textUI->GetColor().r / 255.0f,
							textUI->GetColor().g / 255.0f,
							textUI->GetColor().b / 255.0f,
							textUI->GetColor().a / 255.0f
						};

						if (ImGui::ColorEdit4(colorLabel.c_str(), colorValues))
						{
							textUI->SetColor({
								static_cast<uint8_t>(colorValues[0] * 255),
								static_cast<uint8_t>(colorValues[1] * 255),
								static_cast<uint8_t>(colorValues[2] * 255),
								static_cast<uint8_t>(colorValues[3] * 255)
								});
						}
					}
				}

				ImGui::TreePop();
			}
		}

		//add uielement
		ImGui::Dummy(ImVec2(0, 4));
		ImGui::Separator();

		std::string addButtonLabel = "Add UI Element##" + std::to_string(go.GetID());
		if (ImGui::Button(addButtonLabel.c_str()))
		{
			ImGui::OpenPopup("add_ui_element_popup");
		}

		if (ImGui::BeginPopup("add_ui_element_popup"))
		{
			if (ImGui::MenuItem("Image"))
			{
				std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Image file (*.png)\0*.png\0", "Assets\\Textures")).string();
				if (!filePath.empty() && filePath.ends_with(".png"))
				{
					canvas->AddUIElement<ImageUI>(filePath);
				}
			}
			if (ImGui::MenuItem("Button Image"))
			{
				std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Image file (*.png)\0*.png\0", "Assets\\Textures")).string();
				if (!filePath.empty() && filePath.ends_with(".png"))
				{
					canvas->AddUIElement<ButtonImageUI>(filePath);
				}
			}
			if (ImGui::MenuItem("CheckBox"))
			{
				std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Image file (*.png)\0*.png\0", "Assets\\Textures")).string();
				if (!filePath.empty() && filePath.ends_with(".png"))
				{
					canvas->AddUIElement<CheckBoxUI>(filePath);
				}
			}
			if (ImGui::MenuItem("Slider"))
			{
				std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Image file (*.png)\0*.png\0", "Assets\\Textures")).string();
				if (!filePath.empty() && filePath.ends_with(".png"))
				{
					canvas->AddUIElement<SliderUI>(filePath);
				}
			}
			if (ImGui::MenuItem("Text"))
			{
				std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Image file (*.ttf)\0*.ttf\0", "Assets\\Fonts")).string();
				if (!filePath.empty() && filePath.ends_with(".ttf"))
				{
					canvas->AddUIElement<TextUI>(filePath);
				}
			}

			ImGui::EndPopup();
		}
		//

		ImGui::Dummy(ImVec2(0, 4));
		ImGui::Separator();
	}
}

void PanelInspector::ParticleSystemOptions(GameObject& go)
{
	uint treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
	ParticleSystem* psystem = go.GetComponent<ParticleSystem>();
	if (psystem == nullptr) return;

	std::string psystemLabel = std::string("Particle System##" + std::to_string(go.GetID()));

	ImGui::BeginGroup();
	bool header_open = ImGui::CollapsingHeader(psystemLabel.c_str(), treeFlags);

	if (ImGui::BeginPopupContextItem()) {
		std::string context_label = "Remove Component##" + psystemLabel;
		if (ImGui::MenuItem(context_label.c_str())) {
			go.RemoveComponent(ComponentType::ParticleSystem);
			ImGui::EndPopup();
			return;
		}
		ImGui::EndPopup();
	}

	if (header_open) {
		ImGui::Dummy(ImVec2(0, 4));
		bool hasPath = !psystem->GetPath().empty();
		if (hasPath && ImGui::Button("Save")) {
			psystem->SaveParticleSystemToFile(psystem->GetPath());
		}
		if (ImGui::Button("Save As...")) {
			std::string filePath = std::filesystem::relative(FileDialog::SaveFile("Save ManLite Particle System file (*.mlparticle)\0*.mlparticle\0", "Assets\\Particles")).string();
			if (!filePath.empty()) {
				psystem->SaveParticleSystemToFile(filePath.ends_with(".mlparticle") ? filePath : filePath + ".mlparticle");
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Load")) {
			std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open ManLite Particle System file (*.mlparticle)\0*.mlparticle\0", "Assets\\Particles")).string();
			if (!filePath.empty() && filePath.ends_with(".mlparticle")) {
				psystem->LoadParticleSystemToFile(filePath);
			}
		}

		ImGui::Dummy(ImVec2(0, 4));

		auto& emitters = psystem->GetEmitters();
		auto emitters_c = std::vector<std::shared_ptr<Emitter>>(psystem->GetEmitters());

		for (size_t i = 0; i < emitters_c.size(); ++i)
		{
			std::shared_ptr<Emitter> emitter = emitters_c[i];
			std::string emitterLabel = emitter->GetName() + "##" + std::to_string(emitter->GetID());

			if (ImGui::ArrowButton(("##up" + std::to_string(emitter->GetID())).c_str(), ImGuiDir_Up)) {
				if (i > 0) std::swap(emitters[i], emitters[i - 1]);
			}
			ImGui::SameLine();
			if (ImGui::ArrowButton(("##down" + std::to_string(emitter->GetID())).c_str(), ImGuiDir_Down)) {
				if (i < emitters.size() - 1) std::swap(emitters[i], emitters[i + 1]);
			}
			ImGui::SameLine();

			bool node_open = ImGui::TreeNodeEx(emitterLabel.c_str(), treeFlags);

			if (ImGui::BeginPopupContextItem()) {
				if (ImGui::MenuItem("Duplicate")) {
					auto newEmitter = std::make_shared<Emitter>(*emitter, psystem->GetContainerGO());
					emitters.insert(emitters.begin() + i + 1, newEmitter);
				}
				if (ImGui::MenuItem("Delete")) {
					emitters.erase(emitters.begin() + i);
					ImGui::EndPopup();
					if (node_open) ImGui::TreePop();
					break;
				}
				ImGui::EndPopup();
			}
			if (node_open)
			{
				ImGui::SameLine(ImGui::GetWindowWidth() - 30);
				if (ImGui::SmallButton(("X##" + std::to_string(emitter->GetID())).c_str())) {
					emitters.erase(emitters.begin() + i);
					ImGui::TreePop();
					break;
				}

				ImGui::Dummy(ImVec2(0, 4));
				ImGui::Text("Emitter Name:");
				ImGui::SameLine();
				char nameBuffer[32];
				strcpy(nameBuffer, emitter->GetName().c_str());
				if (ImGui::InputText(("##EmitterName" + std::to_string(emitter->GetID())).c_str(),
					nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsNoBlank)) {
					emitter->SetName(nameBuffer);
				}

				EmitterTypeManager* typeManager = emitter->GetEmitterTypeManager();
				UpdateOptionsEnabled* updateOptions = &typeManager->update_options_enabled;

				if (ImGui::TreeNodeEx("Spawn Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
					if (ImGui::BeginTable("SpawnSettings", 2, ImGuiTableFlags_BordersInnerV)) {
						ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 120.0f);
						ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Spawn Type");
						ImGui::TableSetColumnIndex(1);
						const char* spawnTypes[] = { "Constant", "Burst", "Constant + Burst" };
						int spawnType = static_cast<int>(typeManager->spawn_type);
						if (ImGui::Combo("##SpawnType", &spawnType, spawnTypes, IM_ARRAYSIZE(spawnTypes))) {
							typeManager->spawn_type = static_cast<SpawnType>(spawnType);
						}

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Max Particles");
						ImGui::SameLine();
						std::string curr_amount_particles = "Current amount of Particles in this Emitter:\n<< " + std::to_string(emitter->GetActiveParticlesCount()) + " particles>>";
						Gui::HelpMarker(curr_amount_particles);
						ImGui::TableSetColumnIndex(1);
						int maxParticles = emitter->GetMaxParticles();
						if (ImGui::DragInt("##MaxParticles", &maxParticles, 1, 1, 1000)) {
							emitter->SetMaxParticles(maxParticles);
						}

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Spawn Rate");
						ImGui::SameLine();
						Gui::HelpMarker("Spawn Rate over time:\n0 means spawn each frame\n10 means spawn each 10 second");
						ImGui::TableSetColumnIndex(1);
						float spawnRate = emitter->GetSpawnRate();
						if (ImGui::DragFloat("##SpawnRate", &spawnRate, 0.01f, 0.0f, 60.0f)) {
							emitter->SetSpawnRate(spawnRate);
						}

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Particles/Spawn");
						ImGui::TableSetColumnIndex(1);
						int particlesPerSpawn = emitter->GetParticlesPerSpawn();
						if (ImGui::DragInt("##ParticlesPerSpawn", &particlesPerSpawn, 1, 1, 1000)) {
							emitter->SetParticlesPerSpawn(particlesPerSpawn);
						}

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Lifetime");
						ImGui::TableSetColumnIndex(1);
						float lifeMin = emitter->GetParticleDurationMin();
						float lifeMax = emitter->GetParticleDurationMax();
						if (ImGui::DragFloatRange2("##Lifetime", &lifeMin, &lifeMax, 0.1f, 0.1f, 100.0f, "Min: %.1f", "Max: %.1f")) {
							emitter->SetParticleDurationMin(lifeMin);
							emitter->SetParticleDurationMax(lifeMax);
						}

						ImGui::EndTable();
					}
					ImGui::TreePop();
				}

				if (ImGui::TreeNodeEx("Initial Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
					if (ImGui::BeginTable("InitSettings", 2, ImGuiTableFlags_BordersInnerV)) {
						ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 120.0f);
						ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Init Color");
						ImGui::TableSetColumnIndex(1);
						ML_Color initMin = emitter->GetInitColorMin();
						ML_Color initMax = emitter->GetInitColorMax();
						ImVec4 colorMin(initMin.r / 255.0f, initMin.g / 255.0f, initMin.b / 255.0f, initMin.a / 255.0f);
						ImVec4 colorMax(initMax.r / 255.0f, initMax.g / 255.0f, initMax.b / 255.0f, initMax.a / 255.0f);
						ImGui::ColorEdit4("Min##InitColor", (float*)&colorMin, ImGuiColorEditFlags_NoInputs);
						emitter->SetInitColorMin(ML_Color(colorMin.x * 255, colorMin.y * 255, colorMin.z * 255, colorMin.w * 255));
						ImGui::SameLine();
						ImGui::ColorEdit4("Max##InitColor", (float*)&colorMax, ImGuiColorEditFlags_NoInputs);
						emitter->SetInitColorMax(ML_Color(colorMax.x * 255, colorMax.y * 255, colorMax.z * 255, colorMax.w * 255));

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Init Position");
						ImGui::TableSetColumnIndex(1);
						vec2f initPosMin = emitter->GetInitPositionMin();
						vec2f initPosMax = emitter->GetInitPositionMax();
						ImGui::DragFloat2("Min##InitPos", &initPosMin.x, 0.1f);
						emitter->SetInitPositionMin(initPosMin);
						ImGui::DragFloat2("Max##InitPos", &initPosMax.x, 0.1f);
						emitter->SetInitPositionMax(initPosMax);

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Init Direction");
						ImGui::TableSetColumnIndex(1);
						vec2f initDirMin = emitter->GetInitDirectionMin();
						vec2f initDirMax = emitter->GetInitDirectionMax();
						ImGui::DragFloat2("Min##InitDir", &initDirMin.x, 0.1f);
						emitter->SetInitDirectionMin(initDirMin);
						ImGui::DragFloat2("Max##InitDir", &initDirMax.x, 0.1f);
						emitter->SetInitDirectionMax(initDirMax);

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Init Speed");
						ImGui::TableSetColumnIndex(1);
						float speedMin = emitter->GetInitSpeedMin();
						float speedMax = emitter->GetInitSpeedMax();
						if (ImGui::DragFloatRange2("##InitSpeed", &speedMin, &speedMax, 0.1f, 0.0f, 1000.0f, "Min: %.1f", "Max: %.1f")) {
							emitter->SetInitSpeedMin(speedMin);
							emitter->SetInitSpeedMax(speedMax);
						}

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Init Angle");
						ImGui::TableSetColumnIndex(1);
						float angleMin = emitter->GetInitAngleMin();
						float angleMax = emitter->GetInitAngleMax();
						if (ImGui::DragFloatRange2("##InitAngle", &angleMin, &angleMax, 1.0f, 0.0f, 360.0f, "Min: %.1f", "Max: %.1f")) {
							emitter->SetInitAngleMin(angleMin);
							emitter->SetInitAngleMax(angleMax);
						}

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Init Angular Speed");
						ImGui::TableSetColumnIndex(1);
						float angSpeedMin = emitter->GetInitAngleSpeedMin();
						float angSpeedMax = emitter->GetInitAngleSpeedMax();
						if (ImGui::DragFloatRange2("##InitAngSpeed", &angSpeedMin, &angSpeedMax, 0.1f, -360.0f, 360.0f, "Min: %.1f", "Max: %.1f")) {
							emitter->SetInitAngleSpeedMin(angSpeedMin);
							emitter->SetInitAngleSpeedMax(angSpeedMax);
						}

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Init Scale");
						ImGui::TableSetColumnIndex(1);
						vec2f scaleMin = emitter->GetInitScaleMin();
						vec2f scaleMax = emitter->GetInitScaleMax();
						ImGui::DragFloat2("Min##InitScale", &scaleMin.x, 0.01f);
						emitter->SetInitScaleMin(scaleMin);
						ImGui::DragFloat2("Max##InitScale", &scaleMax.x, 0.01f);
						emitter->SetInitScaleMax(scaleMax);

						ImGui::EndTable();
					}
					ImGui::TreePop();
				}

				if (ImGui::TreeNodeEx("Update Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
					if (ImGui::BeginTable("UpdateSettings", 2, ImGuiTableFlags_BordersInnerV)) {
						ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 120.0f);
						ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

						// Final Speed
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Final Speed");
						ImGui::TableSetColumnIndex(1);
						bool finalSpeed = updateOptions->final_speed;
						if (ImGui::Checkbox("##FinalSpeed", &finalSpeed)) updateOptions->final_speed = finalSpeed;
						if (finalSpeed) {
							float finalMin = emitter->GetFinalSpeedMin();
							float finalMax = emitter->GetFinalSpeedMax();
							if (ImGui::DragFloatRange2("##FinalSpeedVal", &finalMin, &finalMax, 0.1f, 0.0f, 1000.0f, "Min: %.1f", "Max: %.1f")) {
								emitter->SetFinalSpeedMin(finalMin);
								emitter->SetFinalSpeedMax(finalMax);
							}
						}

						// Final Color
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Final Color");
						ImGui::TableSetColumnIndex(1);
						bool finalColor = updateOptions->final_color;
						if (ImGui::Checkbox("##FinalColor", &finalColor)) updateOptions->final_color = finalColor;
						if (finalColor) {
							ML_Color finalMin = emitter->GetFinalColorMin();
							ML_Color finalMax = emitter->GetFinalColorMax();
							ImVec4 colMin(finalMin.r / 255.0f, finalMin.g / 255.0f, finalMin.b / 255.0f, finalMin.a / 255.0f);
							ImVec4 colMax(finalMax.r / 255.0f, finalMax.g / 255.0f, finalMax.b / 255.0f, finalMax.a / 255.0f);
							ImGui::ColorEdit4("Min##FinalColor", (float*)&colMin, ImGuiColorEditFlags_NoInputs);
							emitter->SetFinalColorMin(ML_Color(colMin.x * 255, colMin.y * 255, colMin.z * 255, colMin.w * 255));
							ImGui::SameLine();
							ImGui::ColorEdit4("Max##FinalColor", (float*)&colMax, ImGuiColorEditFlags_NoInputs);
							emitter->SetFinalColorMax(ML_Color(colMax.x * 255, colMax.y * 255, colMax.z * 255, colMax.w * 255));
						}

						// Final Position
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Final Position");
						ImGui::TableSetColumnIndex(1);
						bool finalPos = updateOptions->final_position;
						if (ImGui::Checkbox("##FinalPosition", &finalPos)) updateOptions->final_position = finalPos;
						if (finalPos) {
							vec2f finalPosMin = emitter->GetFinalPositionMin();
							vec2f finalPosMax = emitter->GetFinalPositionMax();
							ImGui::DragFloat2("Min##FinalPos", &finalPosMin.x, 0.1f);
							emitter->SetFinalPositionMin(finalPosMin);
							ImGui::DragFloat2("Max##FinalPos", &finalPosMax.x, 0.1f);
							emitter->SetFinalPositionMax(finalPosMax);
						}

						// Final Direction
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Final Direction");
						ImGui::TableSetColumnIndex(1);
						bool finalDir = updateOptions->final_direction;
						if (ImGui::Checkbox("##FinalDirection", &finalDir)) updateOptions->final_direction = finalDir;
						if (finalDir) {
							vec2f finalDirMin = emitter->GetFinalDirectionMin();
							vec2f finalDirMax = emitter->GetFinalDirectionMax();
							ImGui::DragFloat2("Min##FinalDir", &finalDirMin.x, 0.1f);
							emitter->SetFinalDirectionMin(finalDirMin);
							ImGui::DragFloat2("Max##FinalDir", &finalDirMax.x, 0.1f);
							emitter->SetFinalDirectionMax(finalDirMax);
						}

						// Final Angular Speed
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Final Angular Speed");
						ImGui::TableSetColumnIndex(1);
						bool finalAngSpeed = updateOptions->final_angular_speed;
						if (ImGui::Checkbox("##FinalAngSpeed", &finalAngSpeed)) updateOptions->final_angular_speed = finalAngSpeed;
						if (finalAngSpeed) {
							float finalAngMin = emitter->GetFinalAngleSpeedMin();
							float finalAngMax = emitter->GetFinalAngleSpeedMax();
							if (ImGui::DragFloatRange2("##FinalAngSpeedVal", &finalAngMin, &finalAngMax, 1.0f, -360.0f, 360.0f, "Min: %.1f", "Max: %.1f")) {
								emitter->SetFinalAngleSpeedMin(finalAngMin);
								emitter->SetFinalAngleSpeedMax(finalAngMax);
							}
						}

						// Final Scale
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Final Scale");
						ImGui::TableSetColumnIndex(1);
						bool finalScale = updateOptions->final_scale;
						if (ImGui::Checkbox("##FinalScale", &finalScale)) updateOptions->final_scale = finalScale;
						if (finalScale) {
							vec2f finalScaleMin = emitter->GetFinalScaleMin();
							vec2f finalScaleMax = emitter->GetFinalScaleMax();
							ImGui::DragFloat2("Min##FinalScale", &finalScaleMin.x, 0.01f);
							emitter->SetFinalScaleMin(finalScaleMin);
							ImGui::DragFloat2("Max##FinalScale", &finalScaleMax.x, 0.01f);
							emitter->SetFinalScaleMax(finalScaleMax);
						}

						// Wind Effect
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Wind Effect");
						ImGui::TableSetColumnIndex(1);
						bool windEffect = updateOptions->wind_effect;
						if (ImGui::Checkbox("##WindEffect", &windEffect)) updateOptions->wind_effect = windEffect;
						if (windEffect) {
							vec2f windMin = emitter->GetWindEffectMin();
							vec2f windMax = emitter->GetWindEffectMax();
							ImGui::DragFloat2("Min##Wind", &windMin.x, 0.1f);
							emitter->SetWindEffectMin(windMin);
							ImGui::DragFloat2("Max##Wind", &windMax.x, 0.1f);
							emitter->SetWindEffectMax(windMax);
						}

						ImGui::EndTable();
					}
					ImGui::TreePop();
				}

				if (ImGui::TreeNodeEx("Render Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
					if (ImGui::BeginTable("RenderSettings", 2, ImGuiTableFlags_BordersInnerV)) {
						ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 120.0f);
						ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Render Type");
						ImGui::TableSetColumnIndex(1);

						const char* renderTypes[] = { "Square", "Circle", "Image", "Character" };
						int renderType = static_cast<int>(typeManager->render_type);
						if (ImGui::Combo("##RenderType", &renderType, renderTypes, IM_ARRAYSIZE(renderTypes))) {
							typeManager->render_type = static_cast<RenderType>(renderType);
						}

						if (typeManager->render_type == RenderType::IMAGE) {
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("Texture");
							ImGui::TableSetColumnIndex(1);
							ImGui::Text(emitter->GetTexturePath().c_str());
							if (ImGui::Button("Change Texture"))
							{
								std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Texture file (*.png)\0*.png\0", "Assets\\Textures")).string();
								if (!filePath.empty() && filePath.ends_with(".png"))
								{
									emitter->SwapTexture(filePath);
								}
							}
							if (ImGui::BeginDragDropTarget())
							{
								const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_IMAGE");
								if (payload)
								{
									const char* payload_path = static_cast<const char*>(payload->Data);
									std::string dragged_path(payload_path);
									std::replace(dragged_path.begin(), dragged_path.end(), '/', '\\');
									if (dragged_path.ends_with(".png")) emitter->SwapTexture(dragged_path);
									else LOG(LogType::LOG_WARNING, "Wrong Sprite Format. Correct format to import Sprite file is .png");
								}
								ImGui::EndDragDropTarget();
							}

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("Pixel Art");
							ImGui::TableSetColumnIndex(1);
							bool pixel_art = emitter->IsPixelArt();
							if (ImGui::Checkbox("##PixelArt", &pixel_art))
							{
								emitter->SetPixelArt(pixel_art);
							}
						}
						else if (typeManager->render_type == RenderType::CHARACTER) {
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("Characters");
							ImGui::TableSetColumnIndex(1);
							char buffer[256];
							strcpy(buffer, emitter->GetCharacters().c_str());
							if (ImGui::InputText("##Chars", buffer, sizeof(buffer))) {
								emitter->SetCharacters(buffer);
							}

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("Font");
							ImGui::TableSetColumnIndex(1);
							ImGui::Text(emitter->GetFontPath().c_str());
							if (ImGui::Button("Change Font"))
							{
								std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Font file (*.ttf)\0*.ttf\0", "Assets\\Fonts")).string();
								if (!filePath.empty() && filePath.ends_with(".ttf"))
								{
									emitter->SwapFont(filePath);
								}
							}
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("Pixel Art");
							ImGui::TableSetColumnIndex(1);
							bool pixel_art = emitter->IsPixelArt();
							if (ImGui::Checkbox("##PixelArt", &pixel_art))
							{
								emitter->SetPixelArt(pixel_art);
							}
						}

						ImGui::EndTable();
					}
					ImGui::TreePop();
				}

				ImGui::TreePop();
			}
		}

		if (ImGui::Button("Add New Emitter")) {
			psystem->AddEmptyEmitter();
		}

		ImGui::Dummy(ImVec2(0, 4));
		ImGui::Separator();
	}
	ImGui::EndGroup();

	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_PARTICLES");
		if (payload)
		{
			const char* payload_path = static_cast<const char*>(payload->Data);
			std::string dragged_path(payload_path);
			std::string dragged_name = fs::path(dragged_path).stem().string();
			std::replace(dragged_path.begin(), dragged_path.end(), '/', '\\');

			if (dragged_path.ends_with(".mlparticle"))
				psystem->LoadParticleSystemToFile(dragged_path);
			else
				LOG(LogType::LOG_WARNING, "Wrong Particle Format. Correct format to import Particle file is .mlparticle");
		}
		ImGui::EndDragDropTarget();
	}
}

void PanelInspector::TileMapOptions(GameObject& go)
{
	uint treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
	TileMap* tilemap = go.GetComponent<TileMap>();
	if (tilemap == nullptr) return;
	std::string tilemapLabel = std::string("TileMap##" + std::to_string(go.GetID()));
	bool header_open = ImGui::CollapsingHeader(tilemapLabel.c_str(), treeFlags);

	if (ImGui::BeginPopupContextItem())
	{
		std::string context_label = "Remove Component##" + tilemapLabel;
		if (ImGui::MenuItem(context_label.c_str()))
		{
			app->gui->tile_map_panel->SetMap(nullptr);
			go.RemoveComponent(ComponentType::TileMap);
			ImGui::EndPopup();
			return;
		}
		ImGui::EndPopup();
	}
	if (header_open)
	{
		ImGui::Dummy(ImVec2(0, 4));

		if (ImGui::BeginTable("TileMapSettings", 2,
			ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit))
		{
			ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 100.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Tileset");
			ImGui::TableSetColumnIndex(1);

			int texWidth, texHeight;
			tilemap->GetTextureSize(texWidth, texHeight);
			const float aspectRatio = static_cast<float>(texHeight) / texWidth;

			ImGui::Image(
				tilemap->GetTextureID(),
				ImVec2(100, 100 * aspectRatio),
				ImVec2(0, 1),
				ImVec2(1, 0),
				ImVec4(1, 1, 1, 1),
				ImVec4(0.9f, 0.9f, 0.9f, 0.5f)
			);

			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_IMAGE");
				if (payload)
				{
					const char* payload_path = static_cast<const char*>(payload->Data);
					std::string dragged_path(payload_path);
					std::replace(dragged_path.begin(), dragged_path.end(), '/', '\\');
					if (dragged_path.ends_with(".png")) tilemap->SwapTexture(dragged_path);
					else LOG(LogType::LOG_WARNING, "Wrong Sprite Format. Correct format to import Sprite file is .png");
				}
				ImGui::EndDragDropTarget();
			}
		
			ImGui::SameLine();
			if (ImGui::Button(("Change##" + tilemapLabel).c_str(), ImVec2(60, 24)))
			{
				std::string filePath = std::filesystem::relative(
					FileDialog::OpenFile("Open Tileset (*.png)\0*.png\0", "Assets\\TileMaps")
				).string();

				if (!filePath.empty() && filePath.ends_with(".png"))
				{
					tilemap->SwapTexture(filePath);
				}
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Texture Path");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s",
				tilemap->GetTexturePath().c_str());

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Texture Size");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%d x %d", texWidth, texHeight);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Pixel Art");
			ImGui::SameLine();
			Gui::HelpMarker("Enable pixel-perfect filtering");
			ImGui::TableSetColumnIndex(1);
			bool pixelArt = tilemap->IsPixelArt();
			if (ImGui::Checkbox(("##PixelArt" + tilemapLabel).c_str(), &pixelArt))
			{
				tilemap->SetPixelArtRender(pixelArt);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Grid Size");
			ImGui::TableSetColumnIndex(1);
			vec2f gridSize = tilemap->GetGridSize();
			if (ImGui::DragFloat2(("##GridSize" + tilemapLabel).c_str(), &gridSize.x, 1.0f, 1.0f, 1000.0f))
			{
				tilemap->ResizeGrid(gridSize, -1);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Tile Size");
			ImGui::TableSetColumnIndex(1);
			vec2f tileSize = tilemap->GetTileSize();
			if (ImGui::DragFloat2(("##TileSize" + tilemapLabel).c_str(), &tileSize.x, 1.0f, 1.0f, 1000.0f))
			{
				tilemap->SetTileSize(tileSize);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Tile Section");
			ImGui::SameLine();
			Gui::HelpMarker("Size of each tile in the tileset texture");
			ImGui::TableSetColumnIndex(1);
			vec2f sectionSize = tilemap->GetImageSectionSize();
			if (ImGui::DragFloat2(("##TileSection" + tilemapLabel).c_str(), &sectionSize.x, 1.0f, 1.0f, 4096.0f))
			{
				tilemap->SetImageSectionSize(sectionSize);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Edit");
			ImGui::TableSetColumnIndex(1);
			if (ImGui::Button(("Edit TileMap##" + tilemapLabel).c_str(), ImVec2(120, 24)))
			{
				app->gui->tile_map_panel->RequestFocus();
				app->gui->tile_map_panel->SetMap(tilemap);
			}

			ImGui::EndTable();
		}

		ImGui::Dummy(ImVec2(0, 4));
		ImGui::Separator();
	}
}

void PanelInspector::ScriptsOptions(GameObject& go)
{
	std::vector<Script*> scripts = go.GetComponents<Script>();
	if (scripts.empty()) return;

	for (auto& script : scripts)
	{
		ImGui::PushID(script->GetID());

		std::string scriptLabel = "Script: " + script->GetName() + "##" + std::to_string(script->GetID());
		bool headerOpen = ImGui::CollapsingHeader(scriptLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Remove Component"))
			{
				go.RemoveComponent(script);
				ImGui::EndPopup();
				ImGui::PopID();
				return;
			}
			ImGui::EndPopup();
		}

		if (headerOpen)
		{
			if (ImGui::BeginTable("ScriptFields", 2,
				ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchSame))
			{
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 100.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

				for (auto& [fieldName, field] : script->GetScriptFields())
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::AlignTextToFramePadding();
					ImGui::Text("%s", fieldName.c_str());

					ImGui::TableSetColumnIndex(1);
					ImGui::PushItemWidth(-FLT_MIN);
					ImGui::PushID(fieldName.c_str());

					switch (field.type)
					{
					case ScriptFieldType::Float:
					{
						float value = std::get<float>(field.value);
						if (ImGui::DragFloat("##Float", &value, 0.1f))
						{
							field.value = value;
							script->ApplyFieldValues();
						}
						break;
					}
					case ScriptFieldType::Int:
					{
						int value = std::get<int>(field.value);
						if (ImGui::DragInt("##Int", &value))
						{
							field.value = value;
							script->ApplyFieldValues();
						}
						break;
					}
					case ScriptFieldType::Bool:
					{
						bool value = std::get<bool>(field.value);
						if (ImGui::Checkbox("##Bool", &value))
						{
							field.value = value;
							script->ApplyFieldValues();
						}
						break;
					}
					case ScriptFieldType::String:
					{
						std::string strValue = std::get<std::string>(field.value);
						char buffer[256];
						strcpy_s(buffer, strValue.c_str());
						if (ImGui::InputText("##String", buffer, IM_ARRAYSIZE(buffer)))
						{
							field.value = std::string(buffer);
							script->ApplyFieldValues();
						}
						break;
					}
					case ScriptFieldType::GameObjectPtr:
					{
						uint go_id = std::get<uint>(field.value);
						GameObject* goPtr = engine->scene_manager_em->GetCurrentScene().FindGameObjectByID(go_id).get();
						std::string label = goPtr ?
							goPtr->GetName() + " (" + std::to_string(goPtr->GetID()) + ")" :
							"None";

						// Botón para mostrar/editar
						if (ImGui::Button(label.c_str(), ImVec2(-FLT_MIN, 0)))
						{

						}

						// Drag and Drop
						if (ImGui::BeginDragDropTarget())
						{
							ImGui::PushStyleColor(ImGuiCol_DragDropTarget, IM_COL32(45, 85, 230, 255));
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_MULTI_NODE"))
							{
								GameObject* droppedGo = *(GameObject**)payload->Data;
								field.value = droppedGo->GetID();
								script->ApplyFieldValues();
							}
							ImGui::PopStyleColor();
							ImGui::EndDragDropTarget();
						}
						break;
					}
					}

					if (ImGui::BeginPopupContextItem())
					{
						std::string temp_label = "Set Default Value##" + std::to_string(script->GetID()) + fieldName;
						if (ImGui::MenuItem(temp_label.c_str()))
						{
							script->SetValueDefault(fieldName);
						}
						ImGui::EndPopup();
					}

					ImGui::PopID();
					ImGui::PopItemWidth();
				}

				ImGui::EndTable();
			}

			ImGui::Separator();
		}

		ImGui::PopID();
	}
}

void PanelInspector::AddComponent(GameObject& go)
{
	const ImVec2 button_size_default = ImVec2(150, 0);
	const ImVec2 panel_size_default = ImVec2(200, 212);
	ImGui::Dummy(ImVec2(0, 10));
	ImGui::Dummy(ImVec2((ImGui::GetWindowWidth() - button_size_default.x - 30) * 0.5, 0));
	ImGui::SameLine();
	static bool show_component_window = false;
	static ImVec2 window_pos;

	std::string button_id = "AddComponent_" + std::to_string(go.GetID());

	if (ImGui::Button("Add Component", button_size_default))
	{
		const float margin = 10.0f;
		ImVec2 mouse_pos = ImGui::GetMousePos();
		ImVec2 panel_pos = ImGui::GetWindowPos();
		float panel_height = ImGui::GetWindowHeight();

		const float y_threshold = panel_pos.y + (panel_height * 0.75f);
		float target_y = mouse_pos.y;

		window_pos = ImVec2(mouse_pos.x, target_y);

		if (mouse_pos.y > y_threshold)
		{
			window_pos.y -= panel_size_default.y * 0.95;
		}

		show_component_window = true;
		ImGui::SetNextWindowPos(window_pos);
	}

	if (show_component_window)
	{
		ImGui::SetNextWindowSize(panel_size_default, ImGuiCond_Always);
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
			std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Sprite file (*.png)\0*.png\0", "Assets\\Textures")).string();
			if (!filePath.empty() && filePath.ends_with(".png"))
			{
				go.AddComponent<Sprite2D>(filePath);
			}
			show_component_window = false;
		}

		bool has_sprite = go.GetComponent<Sprite2D>();
		if (!has_sprite) ImGui::BeginDisabled();
		if (ImGui::Selectable("Animation"))
		{
			if (has_sprite)
			{
				go.AddComponent<Animator>();
			}
			else
			{
				LOG(LogType::LOG_WARNING, "Can NOT add Animator Component if GameObject does NOT have Sprite2D Component");
			}
			show_component_window = false;
		}
		if (!has_sprite) ImGui::EndDisabled();
		ImGui::SameLine();
		Gui::HelpMarker("Requires a Sprite2D component to add animations");

		if (ImGui::Selectable("Audio Source"))
		{
			go.AddComponent<AudioSource>();
			show_component_window = false;
		}

		if (ImGui::Selectable("Collider2D"))
		{
			go.AddComponent<Collider2D>();
			show_component_window = false;
		}

		if (ImGui::Selectable("Canvas"))
		{
			go.AddComponent<Canvas>();
			show_component_window = false;
		}

		if (ImGui::Selectable("Particle System"))
		{
			go.AddComponent<ParticleSystem>();
			show_component_window = false;
		}

		if (ImGui::Selectable("TileMap"))
		{
			std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open TileSet (*.png)\0*.png\0", "Assets\\TileMaps")).string();
			if (!filePath.empty() && filePath.ends_with(".png"))
			{
				go.AddComponent<TileMap>();
				go.GetComponent<TileMap>()->SwapTexture(filePath);
			}
			show_component_window = false;
		}

		if (ImGui::Selectable("Script"))
		{
			std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open ManLite Script file (*.cs)\0*.cs\0", "Assets\\Scripts")).string();
			if (!filePath.empty() && filePath.ends_with(".cs"))
			{
				std::string script_name = std::filesystem::path(filePath).stem().string();
				go.AddComponent<Script>(script_name);
				//if (!go.GetComponents<Script>()[go.GetComponents<Script>().size() - 1]->GetMonoObject())
				//	go.RemoveComponent(go.GetComponents<Script>()[go.GetComponents<Script>().size() - 1]);
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
