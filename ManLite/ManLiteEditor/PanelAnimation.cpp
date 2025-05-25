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
	if (bringToFront)
	{
		ImGui::SetNextWindowFocus();
		bringToFront = false;
	}

	bool ret = true;
	const float buttonWidth = 150.0f;
	const ImVec4 accentColor = ImVec4(1.0f, 0.9f, 0.0f, 1.0f);
	const ImVec4 warningColor = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);

	if (ImGui::Begin(name.c_str(), &enabled, ImGuiWindowFlags_MenuBar))
	{
		// Menu Bar
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Load Animation"))
				{
					std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Animation file (*.animation)\0*.animation\0", "Assets\\Animations")).string();
					if (!filePath.empty() && filePath.ends_with(".animation"))
					{
						SetAnimation(filePath);
					}
				}

				if (animation_path != "" && !animation_path.empty())
					if (ImGui::MenuItem("Save Animation"))
					{
						if (this->animation->SaveToFile(animation_path.ends_with(".animation") ? animation_path : animation_path + ".animation"))
							LOG(LogType::LOG_OK, "Animation file saved to: %s", (animation_path.ends_with(".animation") ? animation_path : animation_path + ".animation").c_str());
						else
							LOG(LogType::LOG_ERROR, "ERROR on Animation file save to: %s", (animation_path.ends_with(".animation") ? animation_path : animation_path + ".animation").c_str());
					}

				if (ImGui::MenuItem("Save As..."))
				{
					std::string filePath = std::filesystem::relative(FileDialog::SaveFile("Save Animation file (*.animation)\0*.animation\0", "Assets\\Animations")).string();
					if (!filePath.empty())
					{
						if (this->animation->SaveToFile(filePath.ends_with(".animation") ? filePath : filePath + ".animation"))
							LOG(LogType::LOG_OK, "Animation file saved to: %s", (filePath.ends_with(".animation") ? filePath : filePath + ".animation").c_str());
						else
							LOG(LogType::LOG_ERROR, "ERROR on Animation file save to: %s", (filePath.ends_with(".animation") ? filePath : filePath + ".animation").c_str());
					}
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Load Sprite"))
				{
					std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Sprite file (*.png)\0*.png\0", "Assets\\Textures")).string();
					if (!filePath.empty() && filePath.ends_with(".png"))
					{
						SetSprite(filePath);
					}
				}

				if (ImGui::MenuItem("Clear All"))
				{
					ResourceManager::GetInstance().ReleaseTexture(sprite_path);
					sprite_path = "";
					w = 0;
					h = 0;
					sprite = 0;

					ResourceManager::GetInstance().ReleaseAnimation(animation_path);
					animation_path = "";
					selected_frame = -1;
					animation = new Animation();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		// Main Content
		ImGui::BeginChild("AnimationWorkspace", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
		{
			// Sprite Preview
			DrawSpriteSection();
		}
		ImGui::EndChild();

		// Status Bar
		ImGui::Separator();
		DrawStatusBar();
	}
	ImGui::End();

	return ret;
}

void PanelAnimation::DrawSpriteSection()
{
	const float buttonWidth = 150.0f;
	const ImVec4 accentColor = ImVec4(1.0f, 0.9f, 0.0f, 1.0f);
	const ImVec4 warningColor = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);

	if (sprite == 0)
	{
		ImGui::TextColored(warningColor, "No sprite loaded!");
		return;
	}

	const float availableWidth = ImGui::GetContentRegionAvail().x;
	const float panelWidth = availableWidth * 0.4f;

	ImGui::BeginChild("SpriteSheet", ImVec2(panelWidth, 0), true);
	{
		ImGui::Text("Current Sprite Sheet: ");
		ImGui::SameLine();
		ImGui::TextColored(accentColor, "%s", sprite_path.c_str());
		ImGui::SameLine();
		ImGui::Text("| Texture: %dx%d", w, h);
		ImGui::Separator();

		animation->Update(app->GetDT(), this->currentFrame);
		ML_Rect section = animation->GetCurrentFrame(currentFrame);
		ML_Rect uvs = GetUVs(section, w, h);

		const float frameAspect = (section.h > 0) ? static_cast<float>(section.h) / section.w : 1.0f;

		const ImVec2 availableSpace = ImGui::GetContentRegionAvail();
		const float panelAspect = availableSpace.y / availableSpace.x;

		float imageWidth, imageHeight;

		if (frameAspect > panelAspect) {
			imageHeight = availableSpace.y - 30.0f;
			imageWidth = imageHeight / frameAspect;
		}
		else {
			imageWidth = availableSpace.x - 30.0f;
			imageHeight = imageWidth * frameAspect;
		}

		imageWidth = std::min(imageWidth, availableSpace.x - 10.0f);
		imageHeight = std::min(imageHeight, availableSpace.y - 10.0f);

		ImGui::SetCursorPosX((availableSpace.x - imageWidth) * 0.5f);
		ImGui::SetCursorPosY((availableSpace.y - imageHeight + 60) * 0.5f);

		ImGui::Image(sprite,
			ImVec2(imageWidth, imageHeight),
			ImVec2(uvs.x, uvs.y),
			ImVec2(uvs.w, uvs.h),
			ImVec4(1, 1, 1, 1),
			ImVec4(0.8f, 0.8f, 0.8f, 0.5f)
		);
	}
	ImGui::EndChild();

	ImGui::SameLine();

	// Right Panel - Animation Frames
	ImGui::BeginChild("AnimationFrames", ImVec2(0, 0), true);
	{
		ImGui::Text("Animation Frames");
		ImGui::Separator();

		if (ImGui::BeginTable("FrameControls", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame))
		{
			ImGui::TableSetupColumn("Properties", ImGuiTableColumnFlags_WidthFixed, 200);
			ImGui::TableSetupColumn("Preview");

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			// Animation Properties
			ImGui::Text("Animation Settings");
			ImGui::Separator();

			ImGui::SetNextItemWidth(150);
			ImGui::DragFloat("Speed", &animation->speed, 0.005f, 0.0f, 100.0f, "%.2fx");
			ImGui::Checkbox("Loop", &animation->loop);
			ImGui::SameLine();
			ImGui::Checkbox("Ping Pong", &animation->pingpong);

			ImGui::Checkbox("Flip Horizontal", &animation->flip_h);
			ImGui::SameLine();
			ImGui::Checkbox("Flip Vertical", &animation->flip_v);

			ImGui::Separator();
			ImGui::Text("Frame List");

			// Frame List
			ImGui::BeginChild("FrameList", ImVec2(0, 200), true);
			{
				for (size_t i = 0; i < animation->totalFrames; ++i)
				{
					const bool isSelected = (selected_frame == static_cast<int>(i));
					ImGui::PushID(static_cast<int>(i));

					if (ImGui::Selectable(("Frame " + std::to_string(i)).c_str(), isSelected))
						selected_frame = isSelected ? -1 : static_cast<int>(i);

					if (ImGui::BeginPopupContextItem())
					{
						if (ImGui::MenuItem("Duplicate"))
						{
							animation->PushBack(animation->frames[i]);
							selected_frame = animation->totalFrames - 1;
						}

						if (ImGui::MenuItem("Delete"))
						{
							animation->PopFrame(i);
							selected_frame = 0;
						}

						ImGui::EndPopup();
					}

					ImGui::PopID();
				}
			}
			ImGui::EndChild();

			ImGui::SetNextItemWidth(100);
			if (ImGui::Button("Add Frame"))
			{
				animation->PushBack(ML_Rect(0, 0, w, h));
				selected_frame = animation->totalFrames - 1;
			}

			ImGui::TableSetColumnIndex(1);

			if (selected_frame != -1)
			{
				DrawFrameProperties();
			}

			ImGui::EndTable();
		}
	}
	ImGui::EndChild();
}

void PanelAnimation::DrawStatusBar()
{
	const float buttonWidth = 150.0f;
	const ImVec4 accentColor = ImVec4(1.0f, 0.9f, 0.0f, 1.0f);
	const ImVec4 warningColor = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);

	ImGui::Text("Current Animation: ");
	ImGui::SameLine();

	if (!animation_path.empty())
	{
		ImGui::TextColored(accentColor, "%s", animation_path.c_str());
		ImGui::SameLine();
		ImGui::Text("| Frames: %d | Speed: %.2f", animation->totalFrames, animation->speed);
	}
	else
	{
		ImGui::TextColored(warningColor, "No animation loaded!");
	}
}

void PanelAnimation::DrawFrameProperties()
{
	if (selected_frame != -1)
	{
		ImGui::Text("Frame %d Settings", selected_frame);

		ImGui::BeginChild("FrameProperties", ImVec2(0, 0), true);
		{
			const float columnWidth = ImGui::GetContentRegionAvail().x * 0.4f;

			ImGui::BeginChild("FrameControls", ImVec2(columnWidth, 0), true);
			{
				ML_Rect& frame = animation->frames[selected_frame];

				ImGui::SetNextItemWidth(columnWidth - 35);
				ImGui::DragFloat("X", &frame.x, 1.0f, 0, w, "%.0f");

				ImGui::SetNextItemWidth(columnWidth - 35);
				ImGui::DragFloat("Y", &frame.y, 1.0f, 0, h, "%.0f");

				ImGui::SetNextItemWidth(columnWidth - 35);
				ImGui::DragFloat("W", &frame.w, 1.0f, 1, w - frame.x, "%.0f");

				ImGui::SetNextItemWidth(columnWidth - 35);
				ImGui::DragFloat("H", &frame.h, 1.0f, 1, h - frame.y, "%.0f");
			}
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("FramePreview", ImVec2(0, 0), true);
			{
				if (sprite != 0)
				{
					ML_Rect frame = animation->frames[selected_frame];
					ML_Rect uvs = GetUVs(frame, w, h);

					const float availableWidth = ImGui::GetContentRegionAvail().x - 10;
					const float availableHeight = ImGui::GetContentRegionAvail().y - 10;
					const float aspect = frame.h / frame.w;

					float imageWidth = availableWidth;
					float imageHeight = imageWidth * aspect;

					if (imageHeight > availableHeight)
					{
						imageHeight = availableHeight;
						imageWidth = imageHeight / aspect;
					}

					ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - imageWidth) * 0.5f);
					ImGui::SetCursorPosY((ImGui::GetContentRegionAvail().y - imageHeight) * 0.5f);

					ImGui::Image(sprite,
						ImVec2(imageWidth, imageHeight),
						ImVec2(uvs.x, uvs.y),
						ImVec2(uvs.w, uvs.h),
						ImVec4(1, 1, 1, 1),
						ImVec4(0.8f, 0.8f, 0.8f, 0.5f)
					);
				}
			}
			ImGui::EndChild();
		}
		ImGui::EndChild();
	}
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
