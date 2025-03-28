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

void PanelInspector::SpriteOptions(GameObject& go)
{
	uint treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
	Sprite2D* sprite = go.GetComponent<Sprite2D>();
	if (sprite == nullptr) return;
	std::string spriteLabel = std::string("Sprite2D##" + std::to_string(go.GetID()));
	if (ImGui::CollapsingHeader(spriteLabel.c_str(), treeFlags))
	{
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
	std::string spriteLabel = std::string("Audio Source##" + std::to_string(go.GetID()));
	if (ImGui::CollapsingHeader(spriteLabel.c_str(), treeFlags))
	{
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

		if (!ImGui::IsWindowFocused())
		{
			show_component_window = false;
		}

		ImGui::End();
	}
}
