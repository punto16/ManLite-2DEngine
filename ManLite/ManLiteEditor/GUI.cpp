#include "GUI.h"

#include "EngineCore.h"
#include "RendererEM.h"
#include "WindowEM.h"

#include "GuiPanel.h"
#include "PanelHierarchy.h"
#include "PanelProject.h"
#include "PanelInspector.h"
#include "PanelScene.h"
#include "PanelGame.h"
#include "PanelConsole.h"
#include "PanelAnimation.h"

#include <SDL2/SDL_opengl.h>
#include <gl/GL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

Gui::Gui(App* parent) : Module(parent),
hierarchy_panel(nullptr),
project_panel(nullptr),
inspector_panel(nullptr),
scene_panel(nullptr),
game_panel(nullptr),
console_panel(nullptr),
animation_panel(nullptr)
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

	game_panel = new PanelGame(PanelType::GAME, "Game", true);
	panels.push_back(game_panel);
	ret *= IsInitialized(game_panel);

	console_panel = new PanelConsole(PanelType::CONSOLE, "Console", true);
	panels.push_back(console_panel);
	ret *= IsInitialized(console_panel);

	animation_panel = new PanelAnimation(PanelType::ANIMATION, "Animation", true);
	panels.push_back(animation_panel);
	ret *= IsInitialized(animation_panel);

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

	SDL_Window* window = engine->window_em->GetSDLWindow();
	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, gl_context);
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init("#version 330");

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

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) MainWindowDockspace();

	return ret;
}

bool Gui::Update(double dt)
{
	bool ret = true;

	MainMenuBar();

	//test window
	ImGui::ShowDemoWindow();

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
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
		SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();

		SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
	}

	SDL_GL_SwapWindow(engine->window_em->GetSDLWindow());

	return ret;
}

bool Gui::CleanUp()
{
	bool ret = true;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	SDL_GL_DeleteContext(SDL_GL_GetCurrentContext());

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

void Gui::MainMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			FileMenu();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			EditMenu();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Assets"))
		{
			AssetsMenu();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("GameObject"))
		{
			GameObjectMenu();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Component"))
		{
			ComponentMenu();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window"))
		{
			WindowMenu();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help"))
		{
			HelpMenu();
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void Gui::FileMenu()
{
	ImGui::Text("File Menu test text");
}

void Gui::EditMenu()
{
	ImGui::Text("Edit Menu test text");
}

void Gui::AssetsMenu()
{
	ImGui::Text("Assets Menu test text");
}

void Gui::GameObjectMenu()
{
	ImGui::Text("GameObject Menu test text");
}

void Gui::ComponentMenu()
{
	ImGui::Text("Component Menu test text");
}

void Gui::WindowMenu()
{
	ImGui::Text("Window Menu test text");
}

void Gui::HelpMenu()
{
	ImGui::Text("Help Menu test text");
}

void Gui::HandleInput()
{
}

void Gui::ProcessEvent()
{
}
