#include "GUI.h"

#include "EngineCore.h"
#include "RendererEM.h"
#include "WindowEM.h"
#include "InputEM.h"

#include "GuiPanel.h"
#include "PanelHierarchy.h"
#include "PanelProject.h"
#include "PanelInspector.h"
#include "PanelScene.h"
#include "PanelGame.h"
#include "PanelConsole.h"
#include "PanelAnimation.h"
#include "PanelAbout.h"

#if defined(_WIN32)
#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX
#   include <windows.h>
#   include <GL/glew.h>
#endif

#include <SDL2/SDL_opengl.h>
#include <gl/GL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <shellapi.h>

Gui::Gui(App* parent) : Module(parent),
hierarchy_panel(nullptr),
project_panel(nullptr),
inspector_panel(nullptr),
scene_panel(nullptr),
game_panel(nullptr),
console_panel(nullptr),
animation_panel(nullptr),
about_panel(nullptr)
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

	console_panel = new PanelConsole(PanelType::CONSOLE, "Console", true);
	panels.push_back(console_panel);
	ret *= IsInitialized(console_panel);

	animation_panel = new PanelAnimation(PanelType::ANIMATION, "Animation", true);
	panels.push_back(animation_panel);
	ret *= IsInitialized(animation_panel);

	about_panel = new PanelAbout(PanelType::ABOUT, "About", false);
	panels.push_back(about_panel);
	ret *= IsInitialized(about_panel);

	//"renders" last
	scene_panel = new PanelScene(PanelType::SCENE, "Scene", true);
	panels.push_back(scene_panel);
	ret *= IsInitialized(scene_panel);

	game_panel = new PanelGame(PanelType::GAME, "Game", true);
	panels.push_back(game_panel);
	ret *= IsInitialized(game_panel);

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
	ImGui_ImplSDL2_InitForOpenGL(window, engine->window_em->GetGLContext());
	ImGui_ImplOpenGL3_Init("#version 330");

#pragma region IMGUI_STYLE

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	// Colores base
	colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.23f, 0.23f, 0.23f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	// Elementos interactivos
	colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.46f, 0.03f, 0.42f, 0.20f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);

	// Títulos y cabeceras
	colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.07f, 0.07f, 0.07f, 0.51f);
	colors[ImGuiCol_Header] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.46f, 0.03f, 0.42f, 0.20f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);

	// Botones y controles
	colors[ImGuiCol_Button] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.46f, 0.03f, 0.42f, 0.20f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

	// Elementos de navegación
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.30f, 0.30f, 0.30f, 0.20f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.50f, 0.50f, 0.50f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.12f, 0.12f, 0.86f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.46f, 0.03f, 0.42f, 0.20f);
	colors[ImGuiCol_TabActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.10f, 0.10f, 0.0f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
	colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.7f, 0.3f, 0.6f, 1.0f);

	// Scrollbar
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

	// Separadores
	colors[ImGuiCol_Separator] = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	
	colors[ImGuiCol_DockingPreview] = ImVec4(0.46f, 0.03f, 0.42f, 0.60f);

	// Ajustes de estilo
	style.WindowRounding = 4.0f;
	style.ChildRounding = 4.0f;
	style.FrameRounding = 3.0f;
	style.PopupRounding = 4.0f;
	style.ScrollbarRounding = 4.0f;
	style.GrabRounding = 3.0f;
	style.TabRounding = 4.0f;

	style.FramePadding = ImVec2(6, 4);
	style.ItemSpacing = ImVec2(8, 6);
	style.ScrollbarSize = 12.0f;
	style.GrabMinSize = 8.0f;

	style.WindowBorderSize = 1.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupBorderSize = 1.0f;
	style.FrameBorderSize = 0.0f;
	style.TabBorderSize = 0.0f;

#pragma endregion IMGUI_STYLE

	for (const auto& panel : panels)
		panel->Start();

	return ret;
}

bool Gui::PreUpdate()
{
	bool ret = true;

	HandleInput();

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
			//IMGUI LOGIC
			if (!panel->Update()) return false;
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

	//IMGUI RENDERS ITS TABS HERE
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
	if (ImGui::MenuItem("New Scene", "Ctrl+N", false, false))
	{

	}
	if (ImGui::MenuItem("Open Scene", "Ctrl+O", false, false))
	{

	}

	ImGui::Separator();

	if (ImGui::MenuItem("Save Scene", "Ctrl+S", false, false))
	{

	}
	if (ImGui::MenuItem("Save As...", 0, false, false))
	{

	}

	ImGui::Separator();

	if (ImGui::MenuItem("Settings", 0, false, false))
	{

	}

	ImGui::Separator();

	if (ImGui::MenuItem("Build", 0, false, false))
	{

	}

	ImGui::Separator();

	if (ImGui::MenuItem("Exit"))
	{
		engine->input_em->CloseApp();
	}
}

void Gui::EditMenu()
{
	if (ImGui::MenuItem("Undo", "Ctrl+Z", false, false))
	{

	}
	if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false))
	{

	}

	ImGui::Separator();

	if (ImGui::MenuItem("Copy", "Ctrl+C", false, false))
	{

	}
	if (ImGui::MenuItem("Paste", "Ctrl+V", false, false))
	{

	}
	if (ImGui::MenuItem("Cut", "Ctrl+X", false, false))
	{

	}
	if (ImGui::MenuItem("Duplicate", "Ctrl+D", false, false))
	{

	}
}

void Gui::AssetsMenu()
{
	if (ImGui::BeginMenu("Create"))
	{
		if (ImGui::MenuItem("Folder", 0, false, false))
		{

		}

		ImGui::Separator();

		if (ImGui::MenuItem("Script", 0, false, false))
		{

		}
		if (ImGui::MenuItem("Shader", 0, false, false))
		{

		}

		ImGui::EndMenu();
	}
}

void Gui::GameObjectMenu()
{
	if (ImGui::MenuItem("Create Empty", "Ctrl+Shift+N", false, false))
	{

	}
	if (ImGui::MenuItem("Camera", 0, false, false))
	{

	}
	if (ImGui::MenuItem("Canvas", 0, false, false))
	{

	}
}

void Gui::ComponentMenu()
{
	if (ImGui::MenuItem("Camera", 0, false, false))
	{

	}
	if (ImGui::MenuItem("Collider 2D", 0, false, false))
	{

	}
	if (ImGui::MenuItem("Script", 0, false, false))
	{

	}

	ImGui::Separator();

	if (ImGui::MenuItem("Canvas", 0, false, false))
	{

	}

	ImGui::Separator();

	if (ImGui::MenuItem("Particle System", 0, false, false))
	{

	}

	ImGui::Separator();

	if (ImGui::MenuItem("Audio Source", 0, false, false))
	{

	}
	if (ImGui::MenuItem("Audio Listener", 0, false, false))
	{

	}

	ImGui::Separator();

	if (ImGui::MenuItem("Animation", 0, false, false))
	{

	}
}

void Gui::WindowMenu()
{
	for (auto panel : panels)
	{
		if (panel->GetType() == PanelType::ABOUT || panel->GetType() == PanelType::BUILD)
			continue;

		if (ImGui::MenuItem(panel->GetName().c_str(), NULL, panel->GetState()))
			panel->SwitchState();
	}
}

void Gui::HelpMenu()
{
	if (ImGui::MenuItem("About ManLite Engine"))
	{
		about_panel->SwitchState();
	}

	ImGui::Separator();

	if (ImGui::MenuItem("Documentation"))
	{
		OpenURL("https://github.com/punto16/ManLite-2DEngine");
	}
}

void Gui::HandleInput()
{
	for (auto& event : engine->input_em->GetSDLEvents())
		ImGui_ImplSDL2_ProcessEvent(&event);
}

void Gui::ProcessEvent()
{
}

void Gui::OpenURL(const char* url) const
{
	ShellExecuteA(0, 0, url, 0, 0, SW_SHOW);
}