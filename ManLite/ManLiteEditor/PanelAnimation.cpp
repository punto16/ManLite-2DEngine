#include "PanelAnimation.h"

#include "GUI.h"
#include "Log.h"

#include "FileDialog.h"
#include "ResourceManager.h"
#include "Sprite2D.h"

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
			if (animation != nullptr) ResourceManager::GetInstance().ReleaseAnimation(animation_path);
			//pseudocode here
			// animation = animator::load(loadjson.load(filepath));
			animation_path = filePath;
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Save Animation File", button_size_default))
	{
		std::string filePath = std::filesystem::relative(FileDialog::SaveFile("Save Animation file (*.animation)\0*.animation\0")).string();
		if (!filePath.empty() && filePath.ends_with(".animation"))
		{
			LOG(LogType::LOG_OK, "Animation file saved to: %s", filePath.c_str());
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Choose Sprite", button_size_default))
	{
		std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Sprite file (*.png)\0*.png\0")).string();
		if (!filePath.empty() && filePath.ends_with(".png"))
		{
			if (sprite != 0) ResourceManager::GetInstance().ReleaseTexture(sprite_path);
			sprite = ResourceManager::GetInstance().LoadTexture(filePath, w, h);
			this->sprite_path = filePath;
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
		RELEASE(animation);
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
	ImGui::DragFloat("Sprite Size##AnimationPanel", &image_size, 3.0f, 100.0f, 2000.0f);

	ImGui::Image(sprite,
		ImVec2(image_size, image_size * h / w),
		ImVec2(0, 1),
		ImVec2(1, 0)
	);
}

void PanelAnimation::DrawAnimatorControls()
{
	if (!animator_panel) return;
	
	ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_Leaf;
	if (ImGui::Begin("Animator Panel##Panel", &animator_panel))
	{
		for (size_t i = 0; i < animation->totalFrames; i++)
		{
			if (i == selected_frame) treeFlags |= ImGuiTreeNodeFlags_Selected;

			std::string frame_label = "Frame_" + std::to_string(i);
			if (ImGui::CollapsingHeader(frame_label.c_str(), treeFlags))
			{
				//context aka right click
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
			animation->PushBack(ML_Rect(0, 0, 0, 0));
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

			frame_panel_label = std::string("X Section##Animation_frame");
			ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.8);
			ImGui::DragFloat(frame_panel_label.c_str(), &frame_rect.x, 1.0f);
			frame_panel_label = std::string("Y Section##Animation_frame");
			ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.8);
			ImGui::DragFloat(frame_panel_label.c_str(), &frame_rect.y, 1.0f);
			frame_panel_label = std::string("Width Section##Animation_frame");
			ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.8);
			ImGui::DragFloat(frame_panel_label.c_str(), &frame_rect.w, 1.0f);
			frame_panel_label = std::string("Height Section##Animation_frame");
			ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.8);
			ImGui::DragFloat(frame_panel_label.c_str(), &frame_rect.h, 1.0f);

			animation->frames[selected_frame] = frame_rect;
		}
		ImGui::End();
	}
	
	ImGui::End();
}
