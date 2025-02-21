#include "GUI.h"

#include "EngineCore.h"
#include "RendererEM.h"
#include "WindowEM.h"

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

Gui::Gui(App* parent) : Module(parent)
{
}

Gui::~Gui()
{
}

bool Gui::Awake()
{
	bool ret = true;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForSDLRenderer(engine->window_em->GetSDLWindow(), engine->renderer_em->GetRenderer());
	ImGui_ImplSDLRenderer2_Init(engine->renderer_em->GetRenderer());


	return ret;
}

bool Gui::Start()
{
	bool ret = true;

	return ret;
}

bool Gui::PreUpdate()
{
	bool ret = true;

	ImGui_ImplSDLRenderer2_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	return ret;
}

bool Gui::Update(double dt)
{
	bool ret = true;

	ImGui::Begin("Ejemplo SDL + ImGui");
	ImGui::Text("Hola, ImGui está funcionando!");
	ImGui::End();

	return ret;
}

bool Gui::PostUpdate()
{
	bool ret = true;

	ImGui::Render();
	SDL_SetRenderDrawColor(engine->renderer_em->GetRenderer(), 0, 0, 0, 255);
	SDL_RenderClear(engine->renderer_em->GetRenderer());
	ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), engine->renderer_em->GetRenderer());
	SDL_RenderPresent(engine->renderer_em->GetRenderer());

	return ret;
}

bool Gui::CleanUp()
{
	bool ret = true;

	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_Quit();

	return ret;
}

void Gui::HandleInput()
{
}

void Gui::ProcessEvent()
{
}
