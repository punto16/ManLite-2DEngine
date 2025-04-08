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
#include "ParticleSystem.h"
#include "Emmiter.h"

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
							std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Sprite file (*.png)\0*.png\0")).string();
							if (!filePath.empty() && filePath.ends_with(".png") && filePath != checkboxUI->GetTexturePath())
							{
								checkboxUI->SwapTexture(filePath);
							}
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
										ImGui::DragFloat4(dragLabel.c_str(), &sections[i]->x, 1.0f, 0, checkboxUI->GetTextureSize().x);
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
							std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Sprite file (*.png)\0*.png\0")).string();
							if (!filePath.empty() && filePath.ends_with(".png") && filePath != sliderUI->GetTexturePath())
							{
								sliderUI->SwapTexture(filePath);
							}
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
								FileDialog::OpenFile("Font files (*.ttf)\0*.ttf\0")
							).string();

							if (!filePath.empty() && filePath.ends_with(".ttf") && filePath != textUI->GetFontPath())
							{
								textUI->SwapFont(filePath);
							}
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
			if (ImGui::MenuItem("CheckBox"))
			{
				std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Image file (*.png)\0*.png\0")).string();
				if (!filePath.empty() && filePath.ends_with(".png"))
				{
					canvas->AddUIElement<CheckBoxUI>(filePath);
				}
			}
			if (ImGui::MenuItem("Slider"))
			{
				std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Image file (*.png)\0*.png\0")).string();
				if (!filePath.empty() && filePath.ends_with(".png"))
				{
					canvas->AddUIElement<SliderUI>(filePath);
				}
			}
			if (ImGui::MenuItem("Text"))
			{
				std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Image file (*.ttf)\0*.ttf\0")).string();
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
	if (ImGui::CollapsingHeader(psystemLabel.c_str(), treeFlags)) {
		if (ImGui::BeginPopupContextItem()) {
			std::string context_label = "Remove Component##" + psystemLabel;
			if (ImGui::MenuItem(context_label.c_str())) {
				go.RemoveComponent(ComponentType::ParticleSystem);
				ImGui::EndPopup();
				return;
			}
			ImGui::EndPopup();
		}
		ImGui::Dummy(ImVec2(0, 4));

		auto& emmiters = psystem->GetEmmiters();
		for (size_t i = 0; i < emmiters.size(); ++i) {
			std::string emitterLabel = "Emitter " + std::to_string(i + 1) + "##" + std::to_string(go.GetID());

			if (ImGui::TreeNodeEx(emitterLabel.c_str(), treeFlags)) {
				ImGui::SameLine(ImGui::GetWindowWidth() - 30);
				if (ImGui::SmallButton(("X##" + std::to_string(i)).c_str())) {
					emmiters.erase(emmiters.begin() + i);
					ImGui::TreePop();
					break;
				}

				Emmiter* emitter = emmiters[i].get();
				EmmiterTypeManager* typeManager = emitter->GetEmmiterTypeManager();
				UpdateOptionsEnabled* updateOptions = &typeManager->update_options_enabled;

				// Sección SPAWN
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
						ImGui::TableSetColumnIndex(1);
						int maxParticles = emitter->GetMaxParticles();
						if (ImGui::DragInt("##MaxParticles", &maxParticles, 1, 1, 10000)) {
							emitter->SetMaxParticles(maxParticles);
						}

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Spawn Rate");
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

				// Sección INIT
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

				// Sección UPDATE (FINAL)
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

				// Sección RENDER
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
							// Implementar carga de textura...
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
							// Implementar carga de fuente...
						}

						ImGui::EndTable();
					}
					ImGui::TreePop();
				}

				ImGui::TreePop();
			}
		}

		if (ImGui::Button("Add New Emitter")) {
			psystem->GetEmmiters().emplace_back(std::make_shared<Emmiter>(go.weak_from_this()));
		}

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

		if (ImGui::Selectable("Particle System"))
		{
			go.AddComponent<ParticleSystem>();
			show_component_window = false;
		}

		if (!ImGui::IsWindowFocused())
		{
			show_component_window = false;
		}

		ImGui::End();
	}
}
