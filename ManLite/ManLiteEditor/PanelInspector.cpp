#include "PanelInspector.h"

#include "App.h"

#include "GUI.h"
#include "PanelAnimation.h"
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
	if (ImGui::CollapsingHeader(camLabel.c_str(), treeFlags))
	{
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
	uint treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
	Sprite2D* sprite = go.GetComponent<Sprite2D>();
	if (sprite == nullptr) return;
	std::string spriteLabel = std::string("Sprite2D##" + std::to_string(go.GetID()));
	if (ImGui::CollapsingHeader(spriteLabel.c_str(), treeFlags))
	{
		if (ImGui::BeginPopupContextItem())
		{
			std::string context_label = "Remove Component##" + spriteLabel;
			if (ImGui::MenuItem(context_label.c_str()))
			{
				go.RemoveComponent(ComponentType::Sprite);
				ImGui::EndPopup();
				return;
			}
			ImGui::EndPopup();
		}
		int w, h;
		sprite->GetTextureSize(w, h);
		if (w == 0) w = 1;

		ImGui::Image(sprite->GetTextureID(),
			ImVec2(100, 100 * h / w),
			ImVec2(0, 1),
			ImVec2(1, 0)
			);

		ImGui::SameLine();
		spriteLabel = "Change Image##" + spriteLabel;
		if (ImGui::Button(spriteLabel.c_str()))
		{
			std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Sprite file (*.png)\0*.png\0")).string();
			if (!filePath.empty() && filePath.ends_with(".png") && filePath != sprite->GetTexturePath())
			{
				sprite->SwapTexture(filePath);
			}
		}
		ImGui::Text(sprite->GetTexturePath().c_str());
		ImGui::Dummy(ImVec2(0, 4));

		bool pixel_art_rendering = sprite->IsPixelArt();
		std::string sprite_section_label = std::string("Pixel Art Rendering##" + std::to_string(go.GetID()));
		ImGui::Checkbox(sprite_section_label.c_str(), &pixel_art_rendering);
		sprite->SetPixelArtRender(pixel_art_rendering);

		ML_Rect section = sprite->GetTextureSection();
		int section_x = section.x;
		int section_y = section.y;
		int section_w = section.w;
		int section_h = section.h;

		sprite_section_label = std::string("X Section##" + std::to_string(go.GetID()));
		ImGui::DragInt(sprite_section_label.c_str(), &section_x, 1.0f);
		sprite_section_label = std::string("Y Section##" + std::to_string(go.GetID()));
		ImGui::DragInt(sprite_section_label.c_str(), &section_y, 1.0f);
		sprite_section_label = std::string("Width Section##" + std::to_string(go.GetID()));
		ImGui::DragInt(sprite_section_label.c_str(), &section_w, 1.0f);
		sprite_section_label = std::string("Height Section##" + std::to_string(go.GetID()));
		ImGui::DragInt(sprite_section_label.c_str(), &section_h, 1.0f);
		sprite->SetTextureSection(section_x, section_y, section_w, section_h);
		
		ImGui::Dummy(ImVec2(0, 4));
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
	if (ImGui::CollapsingHeader(animatorLabel.c_str(), treeFlags))
	{
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
		//display animations
		for (const auto& animation_map : animator->GetAnimations())
		{
			std::string animation_name = animation_map.first;
			std::string animation_path = animation_map.second.filePath;
			Animation* animation = animation_map.second.animation;
			animatorLabel = animation_name + "##" + animatorLabel;
			ImGui::CollapsingHeader(animatorLabel.c_str(), ImGuiTreeNodeFlags_Leaf);
			if (ImGui::BeginPopupContextItem())
			{
				std::string context_label = "Remove Animation##" + animatorLabel;
				if (ImGui::MenuItem(context_label.c_str()))
				{
					animator->RemoveAnimation(animation_name);
					ImGui::EndPopup();
					break;
				}
				ImGui::EndPopup();
			}
			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				selected_animation = animation_name;
				if (app->gui->animation_panel->IsAnimationEmpty())
				{
					app->gui->animation_panel->SetAnimation(animation_path);
					app->gui->animation_panel->SetSprite(sprite->GetTexturePath());
				}
			}
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				selected_animation = animation_name;
				app->gui->animation_panel->SetAnimation(animation_path);
				app->gui->animation_panel->SetSprite(sprite->GetTexturePath());

			}
			if (selected_animation == animation_name)
			{
				ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.4);
				if (ImGui::Button("Edit Animation"))
				{
					app->gui->animation_panel->RequestFocus();
					selected_animation = animation_name;
					app->gui->animation_panel->SetAnimation(animation_path);
					app->gui->animation_panel->SetSprite(sprite->GetTexturePath());
				}
				ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.4);

				int w, h;
				sprite->GetTextureSize(w, h);
				ImGui::SameLine();
				ImGui::DragFloat("Sprite Size##AnimationInspectorPanel", &image_animation_size, 3.0f, 10.0f, 2000.0f);
				animation->Update(app->GetDT(), this->currentFrame);

				ML_Rect section = animation->GetCurrentFrame(this->currentFrame);
				if (animation->totalFrames <= 0)
				{
					section.w = w;
					section.h = h;
				}
				ML_Rect uvs = PanelAnimation::GetUVs(section, w, h);

				ImGui::Image(sprite->GetTextureID(),
					ImVec2(image_animation_size, image_animation_size * section.h / section.w),
					ImVec2(uvs.x, uvs.y),
					ImVec2(uvs.w, uvs.h)
				);
			}
		}
		
		//seek animation
		if (selected_animation.empty()) selected_animation = animator->GetCurrentAnimationName();
		ImGui::Dummy(ImVec2(0, 10));
		ImGui::Separator();
		ImGui::Dummy(ImVec2(0, 5));

		if (ImGui::Button("Play", ImVec2(60, 0)))
		{
			animator->Play(selected_animation);
		}

		ImGui::SameLine();
		if (ImGui::Button("Stop", ImVec2(60, 0))) {
			animator->Stop();
		}

		//add animation
		const ImVec2 button_size_default = ImVec2(150, 0);
		ImGui::Dummy(ImVec2(0, 10));
		ImGui::Dummy(ImVec2((ImGui::GetWindowWidth() - (button_size_default.x * 2 + 10) - 30) * 0.5, 0));
		ImGui::SameLine();
	
		std::string add_animation_label = "Add Animation##" + std::to_string(go.GetID());
	
		if (ImGui::Button(add_animation_label.c_str(), button_size_default))
		{
			std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Animation file (*.animation)\0*.animation\0")).string();
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
				}
			}
		}
		std::string create_animation_label = "Create Animation##" + std::to_string(go.GetID());
		ImGui::SameLine(0, 10);
		if (ImGui::Button(create_animation_label.c_str(), button_size_default))
		{
			std::string filePath = std::filesystem::relative(FileDialog::SaveFile("Save Animation file (*.animation)\0*.animation\0")).string();
			if (!filePath.empty())
			{
				std::string animName = fs::path(filePath).stem().string();
				if (!filePath.ends_with(".animation")) filePath += ".animation";
				Animation a;
				if (a.SaveToFile(filePath)) LOG(LogType::LOG_OK, "Animation file saved to: %s", filePath.c_str());
				else LOG(LogType::LOG_ERROR, "ERROR on Animation file save to: %s", filePath.c_str());

				animator->AddAnimation(animName, filePath);
			}
		}


		ImGui::Dummy(ImVec2(0, 4));
		ImGui::Separator();
	}
}

void PanelInspector::AudioSourceOptions(GameObject& go)
{
	uint treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
	AudioSource* audio = go.GetComponent<AudioSource>();
	if (audio == nullptr) return;
	std::string audioLabel = std::string("Audio Source##" + std::to_string(go.GetID()));
	if (ImGui::CollapsingHeader(audioLabel.c_str(), treeFlags))
	{
		if (ImGui::BeginPopupContextItem())
		{
			std::string context_label = "Remove Component##" + audioLabel;
			if (ImGui::MenuItem(context_label.c_str()))
			{
				go.RemoveComponent(ComponentType::AudioSource);
				ImGui::EndPopup();
				return;
			}
			ImGui::EndPopup();
		}
		ImGui::Dummy(ImVec2(10, 0));
		ImGui::SameLine();
		if (ImGui::CollapsingHeader("Musics"))
		{
			for (auto& musics_map : audio->GetMusics())
			{
				std::string audio_name = musics_map.first;
				std::string audio_path = musics_map.second.filePath;
				MusicRef* music = &(musics_map.second);

				std::string audio_label = audio_name + "##" + audio_path + std::to_string(go.GetID());
				ImGui::Dummy(ImVec2(30, 0));
				ImGui::SameLine();
				ImGui::CollapsingHeader(audio_label.c_str(), ImGuiTreeNodeFlags_Leaf);
				if (ImGui::BeginPopupContextItem())
				{
					std::string context_label = "Remove Music##" + audio_label;
					if (ImGui::MenuItem(context_label.c_str()))
					{
						audio->RemoveMusic(audio_name);
						ImGui::EndPopup();
						break;
					}
					ImGui::EndPopup();
				}
				if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					selected_audio = audio_name;
				}
				if (selected_audio == audio_name)
				{
					ImGui::Dummy(ImVec2(30, 0));
					ImGui::SameLine();
					ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.4);
					ImGui::Checkbox("Loop##MusicInspectorPanel", &music->loop);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.4);
					ImGui::Checkbox("Play On Awake##MusicInspectorPanel", &music->play_on_awake);

					ImGui::Dummy(ImVec2(30, 0));
					ImGui::SameLine();
					int volume = music->volume;
					ImGui::DragInt("Volume##MusicInspectorPanel", &volume, 1.0f, 0, 100);
					if (music->volume != volume) audio->SetMusicVolume(audio_name, volume);
					ImGui::Dummy(ImVec2(30, 0));
					ImGui::SameLine();
					ImGui::Separator();

					const ImVec2 button_size_default = ImVec2(60, 0);
					ImGui::Dummy(ImVec2((ImGui::GetWindowWidth() - (button_size_default.x * 2 + 10) - 30) * 0.5, 0));
					ImGui::SameLine();

					if (ImGui::Button("Play", button_size_default))
					{
						audio->PlayMusic(audio_name);
					}

					ImGui::SameLine();
					if (ImGui::Button("Stop", button_size_default))
					{
						audio->StopMusic(audio_name);
					}

					ImGui::Dummy(ImVec2(0, 5));
				}
			}
			ImGui::Dummy(ImVec2(10, 0));
			ImGui::SameLine();
			ImGui::Separator();
		}

		ImGui::Dummy(ImVec2(10, 0));
		ImGui::SameLine();
		if (ImGui::CollapsingHeader("Sound Effects"))
		{
			for (auto& sounds_map : audio->GetSounds())
			{
				std::string audio_name = sounds_map.first;
				std::string audio_path = sounds_map.second.filePath;
				SoundRef* sound = &(sounds_map.second);

				std::string audio_label = audio_name + "##" + audio_path + std::to_string(go.GetID());
				ImGui::Dummy(ImVec2(30, 0));
				ImGui::SameLine();
				ImGui::CollapsingHeader(audio_label.c_str(), ImGuiTreeNodeFlags_Leaf);
				if (ImGui::BeginPopupContextItem())
				{
					std::string context_label = "Remove Sound Effect##" + audio_label;
					if (ImGui::MenuItem(context_label.c_str()))
					{
						audio->RemoveSound(audio_name);
						ImGui::EndPopup();
						break;
					}
					ImGui::EndPopup();
				}
				if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					selected_audio = audio_name;
				}
				if (selected_audio == audio_name)
				{
					ImGui::Dummy(ImVec2(30, 0));
					ImGui::SameLine();
					ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.5);
					ImGui::Checkbox("Loop##MusicInspectorPanel", &sound->loop);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.5);
					ImGui::Checkbox("Play On Awake##MusicInspectorPanel", &sound->play_on_awake);

					ImGui::Dummy(ImVec2(30, 0));
					ImGui::SameLine();
					int volume = sound->volume;
					ImGui::DragInt("Volume##MusicInspectorPanel", &volume, 1.0f, 0, 100);
					if (sound->volume != volume) audio->SetSoundVolume(audio_name, volume);

					ImGui::Dummy(ImVec2(30, 0));
					ImGui::SameLine();
					ImGui::Separator();

					const ImVec2 button_size_default = ImVec2(60, 0);
					ImGui::Dummy(ImVec2((ImGui::GetWindowWidth() - (button_size_default.x * 2 + 10) - 30) * 0.5, 0));
					ImGui::SameLine();

					if (ImGui::Button("Play", button_size_default))
					{
						audio->PlaySound(audio_name);
					}

					ImGui::SameLine();
					if (ImGui::Button("Stop", button_size_default))
					{
						audio->StopSound(audio_name);
					}

					ImGui::Dummy(ImVec2(0, 5));
				}
			}
			ImGui::Dummy(ImVec2(10, 0));
			ImGui::SameLine();
			ImGui::Separator();
		}

		// add audio source
		const ImVec2 button_size_default = ImVec2(150, 0);
		ImGui::Dummy(ImVec2(0, 10));
		ImGui::Dummy(ImVec2((ImGui::GetWindowWidth() - (button_size_default.x * 2 + 10) - 30) * 0.5, 0));
		ImGui::SameLine();

		std::string add_music_label = "Add Music##" + std::to_string(go.GetID());

		if (ImGui::Button(add_music_label.c_str(), button_size_default))
		{
			std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Music file (*.ogg)\0*.ogg\0")).string();
			if (!filePath.empty() && filePath.ends_with(".ogg"))
			{
				std::string audioName = std::filesystem::path(filePath).stem().string();
				audio->AddMusic(audioName, filePath);
			}
		}
		std::string add_sound_label = "Add Sound Effect##" + std::to_string(go.GetID());
		ImGui::SameLine(0, 10);
		if (ImGui::Button(add_sound_label.c_str(), button_size_default))
		{
			std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Sound Effect file (*.wav)\0*.wav\0")).string();
			if (!filePath.empty() && filePath.ends_with(".wav"))
			{
				std::string audioName = std::filesystem::path(filePath).stem().string();
				audio->AddSound(audioName, filePath);
			}
		}

		ImGui::Dummy(ImVec2(0, 4));
		ImGui::Separator();
	}
}

void PanelInspector::Collider2DOptions(GameObject& go)
{
	uint treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
	uint tableFlags = ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersInnerV;
	Collider2D* collider2d = go.GetComponent<Collider2D>();
	if (collider2d == nullptr) return;
	std::string collider2dLabel = std::string("Collider2D##" + std::to_string(go.GetID()));
	if (ImGui::CollapsingHeader(collider2dLabel.c_str(), treeFlags))
	{
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
	if (ImGui::CollapsingHeader(canvasLabel.c_str(), treeFlags))
	{
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

		//ui elements
		ImGui::Dummy(ImVec2(0, 4));
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "UI Elements:");

		for (auto& ui_element : canvas->GetUIElements())
		{
			std::string elementLabel = ui_element->GetName() +
				"##" + std::to_string(ui_element->GetID());

			if (ImGui::TreeNodeEx(elementLabel.c_str(), treeFlags))
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
							std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Sprite file (*.png)\0*.png\0")).string();
							if (!filePath.empty() && filePath.ends_with(".png") && filePath != imageUI->GetTexturePath())
							{
								imageUI->SwapTexture(filePath);
							}
						}

						std::string sizeLabel = "Texture Size";
						ImGui::Text("%s: %d x %d", sizeLabel.c_str(), (int)imageUI->GetTextureSize().x, (int)imageUI->GetTextureSize().y);

						ML_Rect section = imageUI->GetSection();
						std::string sectionLabel = "Section##" + std::to_string(ui_element->GetID());
						if (ImGui::DragFloat4(sectionLabel.c_str(), &section.x, 1.0f, 0, 4096))
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
							std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Sprite file (*.png)\0*.png\0")).string();
							if (!filePath.empty() && filePath.ends_with(".png") && filePath != buttonImage->GetTexturePath())
							{
								buttonImage->SwapTexture(filePath);
							}
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
								ImGui::DragFloat4(dragLabel.c_str(), &section.x, 1.0f, 0, buttonImage->GetTextureSize().x);
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
				std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Image file (*.png)\0*.png\0")).string();
				if (!filePath.empty() && filePath.ends_with(".png"))
				{
					canvas->AddUIElement<ImageUI>(filePath);
				}
			}
			if (ImGui::MenuItem("Button Image"))
			{
				std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Image file (*.png)\0*.png\0")).string();
				if (!filePath.empty() && filePath.ends_with(".png"))
				{
					canvas->AddUIElement<ButtonImageUI>(filePath);
				}
			}
			if (ImGui::MenuItem("Text"))
			{
				// canvas->AddUIElement<TextUI>();
			}

			ImGui::EndPopup();
		}
		//

		ImGui::Dummy(ImVec2(0, 4));
		ImGui::Separator();
	}
}

void PanelInspector::AddComponent(GameObject& go)
{
	const ImVec2 button_size_default = ImVec2(150, 0);
	const ImVec2 panel_size_default = ImVec2(200, 150);
	ImGui::Dummy(ImVec2(0, 10));
	ImGui::Dummy(ImVec2((ImGui::GetWindowWidth() - button_size_default.x - 30) * 0.5, 0));
	ImGui::SameLine();
	static bool show_component_window = false;
	static ImVec2 window_pos;

	std::string button_id = "AddComponent_" + std::to_string(go.GetID());

	if (ImGui::Button("Add Component", button_size_default))
	{
		const float margin = 10.0f; // Margen de seguridad
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
			std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Sprite file (*.png)\0*.png\0")).string();
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

		if (!ImGui::IsWindowFocused())
		{
			show_component_window = false;
		}

		ImGui::End();
	}
}
