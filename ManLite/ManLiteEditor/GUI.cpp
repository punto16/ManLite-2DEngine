#include "GUI.h"

#include "EngineCore.h"
#include "RendererEM.h"
#include "WindowEM.h"

#include "GuiPanel.h"
#include "PanelHierarchy.h"
#include "PanelProject.h"
#include "PanelInspector.h"
#include "PanelScene.h"

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

Gui::Gui(App* parent) : Module(parent),
hierarchy_panel(nullptr),
project_panel(nullptr),
inspector_panel(nullptr),
scene_panel(nullptr)
{
}

Gui::~Gui()
{
	for (auto panel : panels)
		delete panel;
}

bool Gui::Awake()
{
	bool ret = true;

	hierarchy_panel = new PanelHierarchy(PanelType::HIERARCHY, "Hierarchy", true);
	panels.push_back(hierarchy_panel);
	ret *= IsInitialized(hierarchy_panel);

	project_panel = new PanelProject(PanelType::PROJECT, "Project", true);
	panels.push_back(project_panel);
	ret *= IsInitialized(project_panel);

	inspector_panel = new PanelInspector(PanelType::INSPECTOR, "Inspector", true);
	panels.push_back(inspector_panel);
	ret *= IsInitialized(inspector_panel);

	scene_panel = new PanelScene(PanelType::SCENE, "Scene", true);
	panels.push_back(scene_panel);
	ret *= IsInitialized(scene_panel);

	return ret;
}

bool Gui::Start()
{
	bool ret = true;

	IMGUI_CHECKVERSION();
	ImGuiContext* contextui = ImGui::CreateContext();
	if (!contextui) return false;

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	if (!&io) return false;

	ImGui_ImplSDL2_InitForSDLRenderer(engine->window_em->GetSDLWindow(), engine->renderer_em->GetRenderer());
	ImGui_ImplSDLRenderer2_Init(engine->renderer_em->GetRenderer());

#pragma region IMGUI_STYLE

	ImGui::StyleColorsDark();

#pragma endregion IMGUI_STYLE

	for (const auto& panel : panels)
		panel->Start();

	return ret;
}

bool Gui::PreUpdate()
{
	bool ret = true;

	ImGui_ImplSDLRenderer2_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) MainWindowDockspace();

	return ret;
}

bool Gui::Update(double dt)
{
	bool ret = true;

	//main menu bar
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ImGui::Text("Me acabo de masturbar");
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	//test window
	ImGui::ShowDemoWindow();

	//draw all panels
	for (const auto& panel : panels)
	{
		if (panel->GetState())
			if (!panel->Draw()) return false;
	}

	return ret;
}

bool Gui::PostUpdate()
{
	bool ret = true;

	ImGuiIO& io = ImGui::GetIO();
	unsigned int w, h;
	engine->window_em->GetWindowSize(w, h);
	io.DisplaySize = ImVec2((float)w, (float) h);

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

	for (const auto& panel : panels)
	{
		if (panel->GetState())
			panel->CleanUp();
	}

	return ret;
}

bool Gui::IsInitialized(Panel* panel)
{
	if (!panel)
	{
		//LOG(LogType::LOG_ERROR, "-%s", panel->GetName().c_str());
		return false;
	}
	//LOG(LogType::LOG_OK, "-%s", panel->GetName().c_str());
	return true;
}

void Gui::MainWindowDockspace()
{
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) window_flags |= ImGuiWindowFlags_NoBackground;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	static bool p_open = true;
	ImGui::Begin("DockSpace", &p_open, window_flags);
	ImGui::PopStyleVar(3);

	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowMinSize = ImVec2(150, 150);

	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("ManLite_Editor_DockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	ImGui::End();
}

void Gui::HandleInput()
{
}

void Gui::ProcessEvent()
{
}
