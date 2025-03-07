#include "PanelScene.h"

#include "GUI.h"
#include "EngineCore.h"
#include "RendererEM.h"
#include "InputEM.h"
#include "Defs.h"
#include "WindowEM.h"

#include <GL/glew.h>
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
	openglTextureID = engine->renderer_em->renderTexture;
	grid = new Grid(METERS_TO_PIXELS(1), 20);
}

bool PanelScene::Update()
{
	bool ret = true;

	if (ImGui::Begin(name.c_str(), &enabled))
	{
		grid->Draw(engine->renderer_em->GetSceneCamera().GetViewProjMatrix());

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
			// 1. Manejar wrapping del ratón
			bool wrapped = MouseHasTPed(current_mouse_x, current_mouse_y, prev_mouse_x, prev_mouse_y);

			// 2. Calcular delta solo si no hubo wrapping
			float delta_x = 0.0f;
			float delta_y = 0.0f;
			if (!wrapped)
			{
				delta_x = static_cast<float>(current_mouse_x - prev_mouse_x);
				delta_y = static_cast<float>(current_mouse_y - prev_mouse_y);
			}

			// 3. Obtener parámetros actuales
			ImVec2 panel_size = ImGui::GetContentRegionAvail();
			Camera2D& camera = engine->renderer_em->GetSceneCamera();
			ImVec2 panelSize = ImGui::GetContentRegionAvail();
			float panelWidth = panelSize.x;
			float panelHeight = panelSize.y;

			// 4. Convertir delta a coordenadas del mundo
			float scale_factor = 1.0f / camera.GetZoom();

			float aspect_ratio = panelWidth / panelHeight;

			float world_delta_x = -delta_x * scale_factor * (2.0f / panelWidth);
			float world_delta_y = delta_y * scale_factor * (2.0f / panelHeight) * 1 / aspect_ratio;

			float drag_sensitivity = 850.0f;
			world_delta_x *= drag_sensitivity;
			world_delta_y *= drag_sensitivity;

			// 5. Mover cámara
			camera.Move(glm::vec2(world_delta_x, world_delta_y));

			// 6. Actualizar posiciones anteriores
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

	// Horizontal wrapping
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

	// Vertical wrapping
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
		prev_mouse_x = new_x;  // ¡Clave para evitar saltos!
		prev_mouse_y = new_y;
	}

	return wrapped;
}