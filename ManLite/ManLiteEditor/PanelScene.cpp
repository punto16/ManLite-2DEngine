#include "PanelScene.h"

#include "GUI.h"
#include "EngineCore.h"
#include "RendererEM.h"
#include "InputEM.h"

#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL.h>
#include <gl/GL.h>
#include <imgui.h>
#include <vector>

PanelScene::PanelScene(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled), cam_speed(5.0f)
{
}

PanelScene::~PanelScene()
{
}

void PanelScene::Start()
{
	glGenTextures(1, &openglTextureID);
	glBindTexture(GL_TEXTURE_2D, openglTextureID);
	std::vector<uint8_t> emptyData(engine->renderer_em->GetViewPort()->w * engine->renderer_em->GetViewPort()->h * 4, 0); // RGBA: 0,0,0,0
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, engine->renderer_em->GetViewPort()->w, engine->renderer_em->GetViewPort()->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, emptyData.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool PanelScene::Update()
{
	bool ret = true;

	if (ImGui::Begin(name.c_str(), &enabled))
	{
		//draw grid
		engine->renderer_em->DrawGrid(100, { 0,0,0,100 });

		//movement of scene camera
		cam_speed = 5.0f;
		if (engine->input_em->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_REPEAT)
		{
			if (engine->input_em->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) cam_speed *= 2;
			if (engine->input_em->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT) cam_speed *= 0.5;
			if (engine->input_em->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) engine->renderer_em->MoveCamera({ 0,-(int)cam_speed,0,0 });
			if (engine->input_em->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) engine->renderer_em->MoveCamera({ 0,(int)cam_speed,0,0 });
			if (engine->input_em->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) engine->renderer_em->MoveCamera({ -(int)cam_speed,0,0,0 });
			if (engine->input_em->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) engine->renderer_em->MoveCamera({ (int)cam_speed,0,0,0 });
		}
		float zoom_speed = 0.1f;
		int mouse_wheel = engine->input_em->GetMouseWheelMotion();
		if (mouse_wheel != 0)
		{
			if (engine->input_em->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) zoom_speed *= 2;
			if (engine->input_em->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT) zoom_speed *= 0.5;
			engine->renderer_em->CameraZoom(mouse_wheel * zoom_speed);
		}

		static bool middle_drag = false;
		static int prev_mouse_x, prev_mouse_y;
		if (engine->input_em->GetMouseButtonDown(SDL_BUTTON_MIDDLE) == KEY_REPEAT)
		{
			int current_mouse_x, current_mouse_y;
			engine->input_em->GetMousePosition(current_mouse_x, current_mouse_y);

			if (!middle_drag)
			{
				prev_mouse_x = current_mouse_x;
				prev_mouse_y = current_mouse_y;
				middle_drag = true;
			}
			else
			{
				int delta_x = current_mouse_x - prev_mouse_x;
				int delta_y = current_mouse_y - prev_mouse_y;

				float drag_sensitivity = 1.75f;
				engine->renderer_em->MoveCamera({
					(int)(-delta_x * drag_sensitivity),
					(int)(-delta_y * drag_sensitivity),
					0,
					0
					});

				prev_mouse_x = current_mouse_x;
				prev_mouse_y = current_mouse_y;
			}
		}
		else
		{
			middle_drag = false;
		}


		//sdl_texture (type: streming) into opengl 2d image
		SDL_Texture* sdlTexture = engine->renderer_em->GetRendererTexture();
		void* pixels;
		int pitch;
		if (SDL_LockTexture(sdlTexture, nullptr, &pixels, &pitch))
		{
			const char* error = SDL_GetError();
			return false;
		}
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, openglTextureID);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, engine->renderer_em->GetViewPort()->w, engine->renderer_em->GetViewPort()->h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		SDL_UnlockTexture(sdlTexture);

		ImVec2 window_size = ImGui::GetContentRegionAvail();
		float image_width = engine->renderer_em->GetViewPort()->w;
		float image_height = engine->renderer_em->GetViewPort()->h;
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
			(ImTextureID)(intptr_t)openglTextureID,
			image_pos,
			ImVec2(image_pos.x + scaled_size.x, image_pos.y + scaled_size.y),
			ImVec2(0, 0),
			ImVec2(1, 1)
		);
	}
	ImGui::End();

	return ret;
}

bool PanelScene::CleanUp()
{
	bool ret = true;

	if (openglTextureID != 0)
	{
		glDeleteTextures(1, &openglTextureID);
		openglTextureID = 0;
	}

	return ret;
}
