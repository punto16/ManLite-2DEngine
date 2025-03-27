#include "PanelAnimation.h"

#include "GUI.h"
#include "Log.h"
#include "App.h"

#include "FileDialog.h"
#include "ResourceManager.h"
#include "Sprite2D.h"
#include "Animation.h"

#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(disable : 4996)
#endif

#include <filesystem>
#include <exception>
#include <imgui.h>
#include "string"

PanelAnimation::PanelAnimation(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
	animation = new Animation();
}

PanelAnimation::~PanelAnimation()
{
}

bool PanelAnimation::Update()
{
	bool ret = true;

	if (ImGui::Begin(name.c_str(), &enabled))
	{
		DrawTopBarControls();
		DrawImportedSprite();
		DrawAnimatorControls();
	}
	ImGui::End();

	return ret;
}

void PanelAnimation::DrawTopBarControls()
{
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 5));
    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);

    const ImVec2 button_size_default = ImVec2(150, 0);

	if (ImGui::Button("Load Animation File", button_size_default))
	{
		std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Animation file (*.animation)\0*.animation\0")).string();
		if (!filePath.empty() && filePath.ends_with(".animation"))
		{
			SetAnimation(filePath);
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Save Animation As...", button_size_default))
	{
		std::string filePath = std::filesystem::relative(FileDialog::SaveFile("Save Animation file (*.animation)\0*.animation\0")).string();
		if (!filePath.empty())
		{
			if (this->animation->SaveToFile(filePath.ends_with(".animation") ? filePath : filePath + ".animation"))
				LOG(LogType::LOG_OK, "Animation file saved to: %s", (filePath.ends_with(".animation") ? filePath : filePath + ".animation").c_str());
			else
				LOG(LogType::LOG_ERROR, "ERROR on Animation file save to: %s", (filePath.ends_with(".animation") ? filePath : filePath + ".animation").c_str());

		}
	}

	if (animation_path != "" && !animation_path.empty())
	{
		ImGui::SameLine();
		if (ImGui::Button("Save Animation", button_size_default))
		{
			if (this->animation->SaveToFile(animation_path.ends_with(".animation") ? animation_path : animation_path + ".animation"))
				LOG(LogType::LOG_OK, "Animation file saved to: %s", (animation_path.ends_with(".animation") ? animation_path : animation_path + ".animation").c_str());
			else
				LOG(LogType::LOG_ERROR, "ERROR on Animation file save to: %s", (animation_path.ends_with(".animation") ? animation_path : animation_path + ".animation").c_str());
		}
	}

	if (ImGui::Button("Choose Sprite", button_size_default))
	{
		std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Sprite file (*.png)\0*.png\0")).string();
		if (!filePath.empty() && filePath.ends_with(".png"))
		{
			SetSprite(filePath);
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Clear Panel", button_size_default))
	{
		ResourceManager::GetInstance().ReleaseTexture(sprite_path);
		sprite_path = "";
		w = 0;
		h = 0;
		sprite = 0;

		ResourceManager::GetInstance().ReleaseAnimation(animation_path);
		animation_path = "";
		selected_frame = -1;
		animation_frame_panel = false;
		animation = new Animation();
	}

	ImGui::SameLine();
	ImGui::Dummy(ImVec2(10, 0));
	ImGui::SameLine();
	if (ImGui::Button("Animator Panel##Button", button_size_default))
	{
		animator_panel = !animator_panel;
	}

    ImGui::Separator();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

void PanelAnimation::DrawImportedSprite()
{
	if (sprite == 0) return;
	ImGui::DragFloat("Sprite Size##AnimationPanel", &image_size, 3.0f, 10.0f, 2000.0f);
	animation->Update(app->GetDT(), this->currentFrame);
	ML_Rect section = animation->GetCurrentFrame(this->currentFrame);
	if (animation->totalFrames <= 0)
	{
		section.w = this->w;
		section.h = this->h;
	}
	ML_Rect uvs = GetUVs(section, w, h);

	ImGui::Image(sprite,
		ImVec2(image_size, image_size * section.h / section.w),
		ImVec2(uvs.x, uvs.y),
		ImVec2(uvs.w, uvs.h)
	);
}

void PanelAnimation::DrawAnimatorControls()
{
	if (!animator_panel) return;
	
	ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_Leaf;
	if (ImGui::Begin("Animator Panel##Panel", &animator_panel))
	{
		std::string speed_animation_label = std::string("Animation Speed##Animation_Config");
		ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.8);
		ImGui::DragFloat(speed_animation_label.c_str(), &animation->speed, 0.005f, 0.0f, 100.0f);
		speed_animation_label = std::string("Animation Loop##Animation_Config");
		ImGui::Checkbox(speed_animation_label.c_str(), &animation->loop);
		speed_animation_label = std::string("Animation PingPong##Animation_Config");
		ImGui::Checkbox(speed_animation_label.c_str(), &animation->pingpong);

		ImGui::Separator();

		for (size_t i = 0; i < animation->totalFrames; i++)
		{
			if (i == selected_frame) treeFlags |= ImGuiTreeNodeFlags_Selected;

			std::string frame_label = "Frame_" + std::to_string(i);
			if (ImGui::CollapsingHeader(frame_label.c_str(), treeFlags))
			{
				if (ImGui::BeginPopupContextItem())
				{
					frame_label = "Duplicate Frame##" + frame_label;
					if (ImGui::MenuItem(frame_label.c_str()))
					{
						animation->PushBack(animation->frames[i]);
						selected_frame = animation->totalFrames - 1;
						animation_frame_panel = true;
					}

					frame_label = "Delete Frame##" + frame_label;
					if (ImGui::MenuItem(frame_label.c_str()))
					{
						animation->PopFrame(i);
						selected_frame = 0;
						animation_frame_panel = true;
					}
					ImGui::EndPopup();
				}
			}
			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				selected_frame = i;
				animation_frame_panel = true;
			}
		}

		//add frame
		const ImVec2 button_size_default = ImVec2(150, 0);
		ImGui::Dummy(ImVec2(0, 10));
		ImGui::Dummy(ImVec2((ImGui::GetWindowWidth() - button_size_default.x - 30) * 0.5, 0));
		ImGui::SameLine();

		if (ImGui::Button("Add Frame##Animator_Panel", button_size_default))
		{
			animation->PushBack(ML_Rect(0, 0, w, h));
			selected_frame = animation->totalFrames - 1;
			animation_frame_panel = true;
		}
	}

	//frame configuration
	std::string frame_panel_label = "Animation Frame##PanelFrame";
	if (selected_frame != -1)
	{
		if (ImGui::Begin(frame_panel_label.c_str(), &animation_frame_panel))
		{
			const ImVec2 button_size_default = ImVec2(150, 0);
			ImGui::Dummy(ImVec2((ImGui::GetWindowWidth() - button_size_default.x - 30) * 0.5, 0));
			ImGui::SameLine();
			ImGui::Text(std::string("Animation Frame " + std::to_string(selected_frame)).c_str());
			ImGui::Separator();


			ML_Rect frame_rect = animation->frames[selected_frame];

			frame_panel_label = std::string("X    ##Section_Animation_frame");
			ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.4);
			ImGui::DragFloat(frame_panel_label.c_str(), &frame_rect.x, 1.0f);

			ImGui::SameLine();
			frame_panel_label = std::string("Y##Section_Animation_frame");
			ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.4);
			ImGui::DragFloat(frame_panel_label.c_str(), &frame_rect.y, 1.0f);

			frame_panel_label = std::string("Width##Section_Animation_frame");
			ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.4);
			ImGui::DragFloat(frame_panel_label.c_str(), &frame_rect.w, 1.0f);

			ImGui::SameLine();
			frame_panel_label = std::string("Height##Section_Animation_frame");
			ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.4);
			ImGui::DragFloat(frame_panel_label.c_str(), &frame_rect.h, 1.0f);

			animation->frames[selected_frame] = frame_rect;

			ImGui::Separator();
			ImGui::DragFloat("Sprite Size##AnimationPanelFrame", &image_frame_size, 3.0f, 10.0f, 2000.0f);

			if (sprite != 0)
			{
				ML_Rect uvs = GetUVs(frame_rect, this->w, this->h);

				ImGui::Image(sprite,
					ImVec2(image_frame_size, image_frame_size * frame_rect.h / frame_rect.w),
					ImVec2(uvs.x, uvs.y),
					ImVec2(uvs.w, uvs.h)
				);
			}
		}
		ImGui::End();
	}
	
	ImGui::End();
}

bool PanelAnimation::IsAnimationEmpty()
{
	if (animation == nullptr) return true;
	if (animation->totalFrames == 0) return true;
	return false;
}

void PanelAnimation::SetAnimation(std::string new_animation_PATH)
{
	if (animation != nullptr) ResourceManager::GetInstance().ReleaseAnimation(animation_path);
	animation = ResourceManager::GetInstance().LoadAnimation(new_animation_PATH);
	animation_path = new_animation_PATH;
}

ML_Rect PanelAnimation::GetUVs(ML_Rect section, int w, int h)
{
	if (w > 0 && h > 0)
	{
		float u1 = static_cast<float>(section.x) / w;
		float u2 = static_cast<float>(section.x + section.w) / w;
		float v1 = static_cast<float>(h - (section.y + section.h)) / h;
		float v2 = static_cast<float>(h - section.y) / h;
		return ML_Rect(u1, v2, u2, v1);
	}
	else
	{
		return ML_Rect(0, 1, 1, 0);
	}
}

void PanelAnimation::SetSprite(std::string filePath)
{
	if (sprite != 0) ResourceManager::GetInstance().ReleaseTexture(sprite_path);
	sprite = ResourceManager::GetInstance().LoadTexture(filePath, w, h);
	this->sprite_path = filePath;
}
