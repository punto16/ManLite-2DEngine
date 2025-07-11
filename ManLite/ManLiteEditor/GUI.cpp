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
#include "PanelLayer.h"
#include "PanelSaveScene.h"
#include "PanelLoading.h"
#include "PanelTileMap.h"
#include "Script.h"

#include "FileDialog.h"
#include "SceneManagerEM.h"
#include "ScriptingEM.h"
#include "GameObject.h"
#include "Canvas.h"
#include "Camera.h"
#include "Sprite2D.h"
#include "Collider2D.h"
#include "AudioSource.h"
#include "Animator.h"
#include "ParticleSystem.h"
#include "TileMap.h"
#include "Light.h"
#include "FilesManager.h"

#if defined(_WIN32)
#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX
#   include <windows.h>
#   include <GL/glew.h>
#endif

#include <SDL2/SDL_opengl.h>
#include <gl/GL.h>
#include <imgui.h>
#include "ImGuizmo.h"
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include "filesystem"
#include "nlohmann/json.hpp"
#include "fstream"

Gui::Gui(App* parent) : Module(parent),
hierarchy_panel(nullptr),
project_panel(nullptr),
inspector_panel(nullptr),
scene_panel(nullptr),
game_panel(nullptr),
console_panel(nullptr),
animation_panel(nullptr),
about_panel(nullptr),
layer_panel(nullptr),
save_scene_panel(nullptr),
loading_panel(nullptr),
tile_map_panel(nullptr)
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

	layer_panel = new PanelLayer(PanelType::LAYER, "Layer", true);
	panels.push_back(layer_panel);
	ret *= IsInitialized(layer_panel);

	save_scene_panel = new PanelSaveScene(PanelType::SAVE_SCENE, "Save Scene", false);
	panels.push_back(save_scene_panel);
	ret *= IsInitialized(save_scene_panel);

	loading_panel = new PanelLoading(PanelType::LOADING, "Loading", false);
	panels.push_back(loading_panel);
	ret *= IsInitialized(loading_panel);

	tile_map_panel = new PanelTileMap(PanelType::TILEMAP, "TileMap", true);
	panels.push_back(tile_map_panel);
	ret *= IsInitialized(tile_map_panel);

	//"renders" last
	game_panel = new PanelGame(PanelType::GAME, "Game", true);
	panels.push_back(game_panel);
	ret *= IsInitialized(game_panel);

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

	SDL_Window* window = engine->window_em->GetSDLWindow();
	ImGui_ImplSDL2_InitForOpenGL(window, engine->window_em->GetGLContext());
	ImGui_ImplOpenGL3_Init("#version 330");

	SDL_StartTextInput();

#pragma region IMGUI_STYLE

	io.Fonts->AddFontFromFileTTF("Config\\Font\\Rubik.ttf", 14);

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	colors[ImGuiCol_Text]						= ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
	colors[ImGuiCol_TextDisabled]				= ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg]					= ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
	colors[ImGuiCol_ChildBg]					= ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
	colors[ImGuiCol_PopupBg]					= ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
	colors[ImGuiCol_Border]						= ImVec4(0.23f, 0.23f, 0.23f, 0.50f);
	colors[ImGuiCol_BorderShadow]				= ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	colors[ImGuiCol_FrameBg]					= ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_FrameBgHovered]				= ImVec4(0.46f, 0.03f, 0.42f, 0.20f);
	colors[ImGuiCol_FrameBgActive]				= ImVec4(0.30f, 0.30f, 0.30f, 1.00f);

	colors[ImGuiCol_TitleBg]					= ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
	colors[ImGuiCol_TitleBgActive]				= ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed]			= ImVec4(0.07f, 0.07f, 0.07f, 0.51f);
	colors[ImGuiCol_Header]						= ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_HeaderHovered]				= ImVec4(0.46f, 0.03f, 0.42f, 0.20f);
	colors[ImGuiCol_HeaderActive]				= ImVec4(0.25f, 0.25f, 0.25f, 1.00f);

	colors[ImGuiCol_Button]						= ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_ButtonHovered]				= ImVec4(0.46f, 0.03f, 0.42f, 0.20f);
	colors[ImGuiCol_ButtonActive]				= ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
	colors[ImGuiCol_CheckMark]					= ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	colors[ImGuiCol_SliderGrab]					= ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
	colors[ImGuiCol_SliderGrabActive]			= ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

	colors[ImGuiCol_ResizeGrip]					= ImVec4(0.30f, 0.30f, 0.30f, 0.20f);
	colors[ImGuiCol_ResizeGripHovered]			= ImVec4(0.40f, 0.40f, 0.40f, 0.67f);
	colors[ImGuiCol_ResizeGripActive]			= ImVec4(0.50f, 0.50f, 0.50f, 0.95f);
	colors[ImGuiCol_Tab]						= ImVec4(0.12f, 0.12f, 0.12f, 0.86f);
	colors[ImGuiCol_TabHovered]					= ImVec4(0.46f, 0.03f, 0.42f, 0.20f);
	colors[ImGuiCol_TabActive]					= ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_TabUnfocused]				= ImVec4(0.10f, 0.10f, 0.10f, 0.0f);
	colors[ImGuiCol_TabUnfocusedActive]			= ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
	colors[ImGuiCol_TabSelectedOverline]		= ImVec4(0.7f, 0.3f, 0.6f, 1.0f);

	colors[ImGuiCol_ScrollbarBg]				= ImVec4(0.10f, 0.10f, 0.10f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab]				= ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered]		= ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive]		= ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

	colors[ImGuiCol_Separator]					= ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
	colors[ImGuiCol_SeparatorHovered]			= ImVec4(0.40f, 0.40f, 0.40f, 0.78f);
	colors[ImGuiCol_SeparatorActive]			= ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	
	colors[ImGuiCol_DockingPreview]				= ImVec4(0.46f, 0.03f, 0.42f, 0.60f);

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

	ImGui_ImplSDL2_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) MainWindowDockspace();

	return ret;
}

bool Gui::Update(double dt)
{
	bool ret = true;

	if (engine->GetEngineState() == EngineState::PLAY)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
	}
	else
	{
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	}

	AutoSaveSceneBg((float)dt);
	HandleShortcut();

	MainMenuBar();
	BuildPanel();
	AutoSaveScenePanel();

	//test window
	if (show_demo_panel) ImGui::ShowDemoWindow();

	for (const auto& panel : panels)
	{
		if (panel->GetState())
			//IMGUI LOGIC
			if (!panel->Update()) return false;
	}

	//thread set loaded scene
	if (save_scene_panel->set_scene)
	{
		engine->scene_manager_em->CleanUp();
		engine->scene_manager_em->GetCurrentScene() = *save_scene_panel->new_scene;
		LOG(LogType::LOG_INFO, "Scene <%s> created", save_scene_panel->new_scene->GetSceneName().c_str());
		engine->scene_manager_em->FinishLoad();
		save_scene_panel->set_scene = false;
		ResourceManager::GetInstance().ReleaseTexture("Config\\placeholder.png");
		engine->scripting_em->stop_process_instantiate_queue = false;
		RestartAutoSaveTimer();
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
	if (!panel) return false;
	return true;
}

void Gui::HelpMarker(std::string text)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(450.0f);
		ImGui::TextUnformatted(text.c_str());
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
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

		//manage engine state
		ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x / 2 - 255, 0.0f));
		if (engine->scripting_em->ScriptsCompiledSuccessfully())
		{
			if (ImGui::Button(">"))
			{
				engine->SetEngineState(EngineState::PLAY);
			}
			if (ImGui::Button("l l"))
			{
				engine->SetEngineState(EngineState::PAUSE);
			}
			if (ImGui::Button("[]"))
			{
				engine->SetEngineState(EngineState::STOP);
			}
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255)); // red
			if (ImGui::Button(">"))
			{
			}
			if (ImGui::Button("l l"))
			{
			}
			if (ImGui::Button("[]"))
			{
			}
			ImGui::PopStyleColor();
			Gui::HelpMarker("Fix Scripts Errors Before Playing the Scene");
		}

		//show engine state
		ImGui::Dummy({50, 0});

		if (engine->GetEngineState() == EngineState::PLAY)
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255)); // green
		else if (engine->GetEngineState() == EngineState::PAUSE)
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255)); // red
		else
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 100, 100, 255)); // grey
		ImGui::Text("o");
		ImGui::PopStyleColor();

		ImGui::Dummy({5, 0});
		if (engine->GetEngineState() == EngineState::PLAY)
			ImGui::Text("Scene is now Playing");
		if (engine->GetEngineState() == EngineState::PAUSE)
			ImGui::Text("Scene is now Paused ");
		if (engine->GetEngineState() == EngineState::STOP)
			ImGui::Text("Scene is now Stopped");

		ImGui::Dummy({5, 0});
		int h = 0, m = 0, s = 0, ms = 0;
		Timer::SecondsToFormat(engine->GetGameTime(), h, m, s, ms);
		ImGui::Text("Game Duration: %02d:%02d:%02d:%03d", h, m, s, ms);

		//test buttons
		//ImGui::Dummy(ImVec2(40.0f, 0.0f));
		//if (engine->GetEngineState() == EngineState::STOP && ImGui::Button("Reload Scripts"))
		//{
		//	engine->scripting_em->RecompileScripts();
		//}
		//ImGui::Dummy(ImVec2(40.0f, 0.0f));
		//if (engine->GetEngineState() == EngineState::STOP && ImGui::Button("Reload Resources"))
		//{
		//	ResourceManager::GetInstance().ReloadAll();
		//}
		//
		ImGui::EndMainMenuBar();
	}
}

void Gui::FileMenu()
{
	if (ImGui::MenuItem("New Scene", "Ctrl+N"))
	{
		save_scene_panel->RequestFocus();
		save_scene_panel->save_panel_action = SavePanelAction::NEW_SCENE;
	}
	if (ImGui::MenuItem("Open Scene", "Ctrl+O"))
	{
		save_scene_panel->RequestFocus();
		save_scene_panel->save_panel_action = SavePanelAction::OPEN_SCENE;
	}

	ImGui::Separator();

	if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
	{
		std::string filePath = engine->scene_manager_em->GetCurrentScene().GetScenePath();
		if (filePath.empty() || filePath == "")
			filePath = "Assets\\Scenes\\" + engine->scene_manager_em->GetCurrentScene().GetSceneName() + ".mlscene";
		if (!filePath.empty() && filePath != "")
		{
			std::filesystem::path fullPath(filePath);
			std::string sceneName = fullPath.stem().string();
			std::string directory = fullPath.parent_path().string();

			if (!directory.empty() && directory.back() != std::filesystem::path::preferred_separator)
				directory += std::filesystem::path::preferred_separator;

			engine->scene_manager_em->SaveScene(directory, sceneName);
			RestartAutoSaveTimer();
		}
	}
	if (ImGui::MenuItem("Save As...", 0))
	{
		std::string filePath = std::filesystem::relative(FileDialog::SaveFile("Save ManLite Scene file (*.mlscene)\0*.mlscene\0", "Assets\\Scenes")).string();
		if (!filePath.empty())
		{
			std::filesystem::path fullPath(filePath);
			std::string sceneName = fullPath.stem().string();
			std::string directory = fullPath.parent_path().string();

			if (!directory.empty() && directory.back() != std::filesystem::path::preferred_separator)
				directory += std::filesystem::path::preferred_separator;

			engine->scene_manager_em->SaveScene(directory, sceneName);
			RestartAutoSaveTimer();
		}
	}
	if (ImGui::BeginMenu("Import"))
	{
		bool tiled_file = ImGui::MenuItem("Tiled File");
		ImGui::SameLine();
		Gui::HelpMarker("Import a tiled file exported as .json\nIMPORTANT: the tiled file must contain only 1 TileSet and Orthogonal and must be embed to the TileMap");
		if (tiled_file)
		{
			std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Tiled file (*.json)\0*.json\0", "Assets\\TileMaps\\Tiled")).string();
			if (!filePath.empty() && filePath.ends_with(".json"))
			{
				engine->scene_manager_em->ImportTiledFile(filePath);
			}
		}
		ImGui::EndMenu();
	}

	ImGui::Separator();

	if (ImGui::MenuItem("Settings", 0, false, false))
	{

	}

	ImGui::Separator();

	if (ImGui::MenuItem("Build"))
	{
		showBuildPanel = true;
		scenes = FilesManager::GetInstance().GetAllScenePaths();
		int w = 0, h = 0;
		if (icon_texture != 0 || !icon_path.empty()) ResourceManager::GetInstance().ReleaseTexture(icon_path);
		icon_path = "Config\\Icons\\build_icon.png";
		icon_texture = ResourceManager::GetInstance().LoadTexture(icon_path, w, h);
	}

	ImGui::Separator();

	if (ImGui::MenuItem("Exit"))
	{
		save_scene_panel->RequestFocus();
		save_scene_panel->save_panel_action = SavePanelAction::CLOSE_APP;
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
	if (ImGui::MenuItem("Duplicate", "Ctrl+D"))
	{
		auto& scene = engine->scene_manager_em->GetCurrentScene();
		const auto& selected = scene.GetSelectedGOs();

		std::vector<std::shared_ptr<GameObject>> new_selected;
		for (const auto& weakGo : selected)
		{
			if (auto go = weakGo.lock())
			{
				new_selected.push_back(scene.DuplicateGO(*go));
			}
		}
		scene.SelectGameObject(nullptr, false, true);
		for (const auto& go : new_selected)
		{
			scene.SelectGameObject(go, true);
		}
	}
}

void Gui::AssetsMenu()
{

	if (ImGui::BeginMenu("Create"))
	{
		if (ImGui::MenuItem("Folder"))
		{
			std::string folder_path = project_panel->current_directory->absolute_path + "\\" + "New Folder";
			if (FilesManager::GetInstance().CreateFolder(folder_path))
			{
				FilesManager::GetInstance().ProcessFromRoot();
				project_panel->UpdateCurrentDirectory(project_panel->current_directory);
				project_panel->RequestFocus();
			}
		}

		ImGui::Separator();

		if (ImGui::BeginMenu("Script"))
		{
			static char script_name_input[128] = "NewScript";
			ImGui::SetNextItemWidth(200.0f);

			if (ImGui::InputText("##script_name_input", script_name_input, IM_ARRAYSIZE(script_name_input),
				ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsNoBlank))
			{
				std::string script_name = script_name_input;

				std::replace(script_name.begin(), script_name.end(), ' ', '_');

				script_name.erase(std::remove_if(script_name.begin(), script_name.end(),
					[](char c) { return !std::isalnum(c) && c != '_'; }),
					script_name.end());

				if (!script_name.empty() && std::isdigit(static_cast<unsigned char>(script_name[0]))) {
					script_name = "Script_" + script_name;
				}

				if (script_name.empty()) {
					script_name = "NewScript";
				}

				if (script_name.length() < 3) {
					script_name = "NewScript";
				}

				engine->scripting_em->CreateScriptFile(script_name);

				FilesManager::GetInstance().ProcessFromRoot();
				project_panel->UpdateCurrentDirectory(project_panel->current_directory);

				// Resetear el input
				strcpy(script_name_input, "NewScript");
			}

			std::string name_str(script_name_input);
			bool valid_name = !name_str.empty() &&
				name_str.length() >= 3 &&
				std::all_of(name_str.begin(), name_str.end(), [](char c) {
				return std::isalnum(c) || c == '_';
					}) &&
				(!std::isdigit(static_cast<unsigned char>(name_str[0])));

					if (!valid_name) {
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
						ImGui::Text("Use letters, numbers and _. Min 3 chars. Can't start with number.");
						ImGui::PopStyleColor();
					}

					ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}
}

void Gui::GameObjectMenu()
{
	if (ImGui::MenuItem("Create Empty", "Ctrl+Shift+N"))
	{
		if (engine->scene_manager_em->GetCurrentScene().GetSelectedGOs().empty())
		{
			engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(engine->scene_manager_em->GetCurrentScene().GetSceneRoot());
		}
		else
		{
			for (const auto& go : engine->scene_manager_em->GetCurrentScene().GetSelectedGOs())
				engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(*go.lock());
		}
	}
	if (ImGui::MenuItem("Camera"))
	{
		GameObject* e_go = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(engine->scene_manager_em->GetCurrentScene().GetSceneRoot()).get();
		e_go->SetName("Camera");
		e_go->AddComponent<Camera>();
	}
	if (ImGui::MenuItem("Sprite"))
	{
		std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Sprite file (*.png)\0*.png\0", "Assets\\Textures")).string();
		if (!filePath.empty() && filePath.ends_with(".png"))
		{
			GameObject* e_go = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(engine->scene_manager_em->GetCurrentScene().GetSceneRoot()).get();
			e_go->SetName("Sprite");
			e_go->AddComponent<Sprite2D>(filePath);
		}
	}
	if (ImGui::MenuItem("Audio Source"))
	{
		GameObject* e_go = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(engine->scene_manager_em->GetCurrentScene().GetSceneRoot()).get();
		e_go->SetName("AudioSource");
		e_go->AddComponent<AudioSource>();
	}
	if (ImGui::MenuItem("Collider 2D"))
	{
		GameObject* e_go = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(engine->scene_manager_em->GetCurrentScene().GetSceneRoot()).get();
		e_go->SetName("Collider2D");
		e_go->AddComponent<Collider2D>();
	}
	if (ImGui::MenuItem("Canvas"))
	{
		GameObject* e_go = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(engine->scene_manager_em->GetCurrentScene().GetSceneRoot()).get();
		e_go->SetName("Canvas");
		e_go->AddComponent<Canvas>();
	}
	if (ImGui::MenuItem("Particle System"))
	{
		GameObject* e_go = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(engine->scene_manager_em->GetCurrentScene().GetSceneRoot()).get();
		e_go->SetName("ParticleSystem");
		e_go->AddComponent<ParticleSystem>();
	}
	if (ImGui::MenuItem("TileMap"))
	{
		std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open TileSet (*.png)\0*.png\0", "Assets\\TileMaps")).string();
		if (!filePath.empty() && filePath.ends_with(".png"))
		{
			GameObject* e_go = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(engine->scene_manager_em->GetCurrentScene().GetSceneRoot()).get();
			e_go->SetName("TileMap");
			e_go->AddComponent<TileMap>();
			e_go->GetComponent<TileMap>()->SwapTexture(filePath);
		}
	}
	if (ImGui::MenuItem("Script"))
	{
		std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open ManLite Script file (*.cs)\0*.cs\0", "Assets\\Scripts")).string();
		if (!filePath.empty() && filePath.ends_with(".png"))
		{
			GameObject* e_go = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(engine->scene_manager_em->GetCurrentScene().GetSceneRoot()).get();
			e_go->SetName("Script");
			std::filesystem::path fullPath(filePath);
			std::string script_name = fullPath.stem().string();
			e_go->AddComponent<Script>(script_name);
		}
	}
	if (ImGui::BeginMenu("Light"))
	{
		if (ImGui::MenuItem("Ambient Light"))
		{
			GameObject* e_go = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(engine->scene_manager_em->GetCurrentScene().GetSceneRoot()).get();
			e_go->SetName("AmbientLight");
			e_go->AddComponent<Light>();
			e_go->GetComponent<Light>()->SetType(LightType::AREA_LIGHT);
		}
		if (ImGui::MenuItem("Point Light"))
		{
			GameObject* e_go = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(engine->scene_manager_em->GetCurrentScene().GetSceneRoot()).get();
			e_go->SetName("PointLight");
			e_go->AddComponent<Light>();
			e_go->GetComponent<Light>()->SetType(LightType::POINT_LIGHT);
		}
		if (ImGui::MenuItem("Spot Light"))
		{
			GameObject* e_go = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(engine->scene_manager_em->GetCurrentScene().GetSceneRoot()).get();
			e_go->SetName("SpotLight");
			e_go->AddComponent<Light>();
			e_go->GetComponent<Light>()->SetType(LightType::RAY_LIGHT);
		}

		ImGui::EndMenu();
	}
}

void Gui::ComponentMenu()
{
	if (ImGui::MenuItem("Camera"))
	{
		for (const auto& item : engine->scene_manager_em->GetCurrentScene().GetSelectedGOs())
			item.lock()->AddComponent<Camera>();
	}
	if (ImGui::MenuItem("Collider 2D"))
	{
		for (const auto& item : engine->scene_manager_em->GetCurrentScene().GetSelectedGOs())
			item.lock()->AddComponent<Collider2D>();
	}
	if (ImGui::MenuItem("Script"))
	{
		std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open ManLite Script file (*.cs)\0*.cs\0", "Assets\\Scripts")).string();
		if (!filePath.empty() && filePath.ends_with(".cs"))
		{
			std::string script_name = std::filesystem::path(filePath).stem().string();

			for (const auto& item : engine->scene_manager_em->GetCurrentScene().GetSelectedGOs())
			{
				item.lock()->AddComponent<Script>(script_name);
				if (!item.lock()->GetComponents<Script>()[item.lock()->GetComponents<Script>().size() - 1]->GetMonoObject())
					item.lock()->RemoveComponent(item.lock()->GetComponents<Script>()[item.lock()->GetComponents<Script>().size() - 1]);
			}
		}
	}

	ImGui::Separator();

	if (ImGui::MenuItem("Canvas"))
	{
		for (const auto& item : engine->scene_manager_em->GetCurrentScene().GetSelectedGOs())
			item.lock()->AddComponent<Canvas>();
	}

	ImGui::Separator();

	if (ImGui::MenuItem("Particle System"))
	{
		for (const auto& item : engine->scene_manager_em->GetCurrentScene().GetSelectedGOs())
			item.lock()->AddComponent<ParticleSystem>();
	}

	ImGui::Separator();

	if (ImGui::MenuItem("Audio Source"))
	{
		for (const auto& item : engine->scene_manager_em->GetCurrentScene().GetSelectedGOs())
			item.lock()->AddComponent<AudioSource>();
	}

	ImGui::Separator();

	if (ImGui::MenuItem("Sprite"))
	{
		std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open Sprite file (*.png)\0*.png\0", "Assets\\Textures")).string();
		if (!filePath.empty() && filePath.ends_with(".png"))
		{
			for (const auto& item : engine->scene_manager_em->GetCurrentScene().GetSelectedGOs())
				item.lock()->AddComponent<Sprite2D>(filePath);
		}
	}
	if (ImGui::MenuItem("Animator"))
	{
		for (const auto& item : engine->scene_manager_em->GetCurrentScene().GetSelectedGOs())
			if (item.lock()->GetComponent<Sprite2D>())
				item.lock()->AddComponent<Animator>();
	}
	if (ImGui::MenuItem("TileMap"))
	{
		std::string filePath = std::filesystem::relative(FileDialog::OpenFile("Open TileSet file (*.png)\0*.png\0", "Assets\\TileMaps")).string();
		if (!filePath.empty() && filePath.ends_with(".png"))
		{
			for (const auto& item : engine->scene_manager_em->GetCurrentScene().GetSelectedGOs())
			{
				item.lock()->AddComponent<TileMap>();
				item.lock()->GetComponent<TileMap>()->SwapTexture(filePath);
			}
		}
	}
	ImGui::Separator();

	if (ImGui::MenuItem("Ambient Light"))
	{
		for (const auto& item : engine->scene_manager_em->GetCurrentScene().GetSelectedGOs())
		{
			item.lock()->AddComponent<Light>();
			item.lock()->GetComponent<Light>()->SetType(LightType::AREA_LIGHT);
		}
	}
	if (ImGui::MenuItem("Point Light"))
	{
		for (const auto& item : engine->scene_manager_em->GetCurrentScene().GetSelectedGOs())
		{
			item.lock()->AddComponent<Light>();
			item.lock()->GetComponent<Light>()->SetType(LightType::POINT_LIGHT);
		}
	}
	if (ImGui::MenuItem("Spot Light"))
	{
		for (const auto& item : engine->scene_manager_em->GetCurrentScene().GetSelectedGOs())
		{
			item.lock()->AddComponent<Light>();
			item.lock()->GetComponent<Light>()->SetType(LightType::RAY_LIGHT);
		}
	}
}

void Gui::WindowMenu()
{
	for (auto panel : panels)
	{
		if (panel->GetType() == PanelType::ABOUT ||
			panel->GetType() == PanelType::BUILD ||
			panel->GetType() == PanelType::SAVE_SCENE ||
			panel->GetType() == PanelType::LOADING)
			continue;

		if (ImGui::MenuItem(panel->GetName().c_str(), NULL, panel->GetState()))
			panel->SwitchState();
	}

	if (ImGui::MenuItem("ImGuiDemo", NULL, show_demo_panel))
		show_demo_panel = !show_demo_panel;
}

void Gui::HelpMenu()
{
	if (ImGui::MenuItem("About ManLite Engine"))
	{
		about_panel->SwitchState();
	}
	ImGui::Separator();

	ImGui::TextLinkOpenURL("Documentation", "https://github.com/punto16/ManLite-2DEngine");
}

void Gui::BuildPanel()
{
	if (showBuildPanel)
	{
		ImGui::OpenPopup("Build Settings");
		ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
		if (ImGui::BeginPopupModal("Build Settings", &showBuildPanel,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 12));
			ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);

			ImGui::Columns(2);
			if (ImGui::ImageButton("build_icon",
				icon_texture,
				ImVec2(64, 64),
				ImVec2(0, 1),
				ImVec2(1, 0)
				))
			{
				std::string filePath = std::filesystem::relative(
					FileDialog::OpenFile("Open Sprite file (*.png)\0*.png\0", "Assets\\Textures")
				).string();

				if (!filePath.empty() && filePath.ends_with(".png"))
				{
					int w = 0, h = 0;
					if (icon_texture != 0 || !icon_path.empty())
						ResourceManager::GetInstance().ReleaseTexture(icon_path);
					icon_path = filePath;
					icon_texture = ResourceManager::GetInstance().LoadTexture(filePath, w, h);
				}
			}
			ImGui::NextColumn();
			ImGui::InputText("App Name", app_name, IM_ARRAYSIZE(app_name));
			ImGui::Columns(1);

			ImGui::Spacing();
			ImGui::Separator();

			if (!includedScenesNames.empty())
			{
				std::vector<const char*> items;
				int currentItem = -1;

				for (size_t i = 0; i < includedScenesNames.size(); ++i) {
					items.push_back(includedScenesNames[i].c_str());
					if (includedScenesNames[i] == selectedMainScene) {
						currentItem = static_cast<int>(i);
					}
				}

				if (currentItem == -1 && !includedScenesNames.empty()) {
					currentItem = 0;
					selectedMainScene = includedScenesNames[0];
				}

				if (ImGui::Combo("Main Scene", &currentItem, items.data(), items.size())) {
					selectedMainScene = includedScenesNames[currentItem];
				}
			}
			else
			{
				ImGui::TextColored(ImVec4(1, 0, 0, 1), "No scenes included in build!");
			}

			{
				ImGui::Text("Included Scenes:");

				const float child_height = ImGui::GetTextLineHeightWithSpacing() * 8;
				ImGui::BeginChild("##SceneChild", ImVec2(-1, child_height), true,
					ImGuiWindowFlags_HorizontalScrollbar);


				for (const auto& scene : scenes)
				{
					bool& included = sceneInclusionMap[scene];
					if (ImGui::Checkbox(scene.c_str(), &included))
					{
						if (included) {
							includedScenesNames.push_back(scene);
						}
						else {
							auto it = std::find(includedScenesNames.begin(),
								includedScenesNames.end(),
								scene);
							if (it != includedScenesNames.end())
								includedScenesNames.erase(it);
							if (scene == selectedMainScene) {
								selectedMainScene = "";
							}
						}
					}
				}

				ImGui::EndChild();
			}

			ImGui::Spacing();
			ImGui::Separator();

			{
				ImGui::Text("Window Settings");
				ImGui::Checkbox("Fullscreen", &fullscreen);
				ImGui::SameLine();
				ImGui::Checkbox("VSync", &vsync);
			}

			ImGui::Separator();
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 260);
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				if (icon_texture != 0 || !icon_path.empty()) ResourceManager::GetInstance().ReleaseTexture(icon_path);
				icon_path = "";
				showBuildPanel = false;
				icon_texture = 0;
				memset(app_name, 0, sizeof(app_name));
				sceneInclusionMap.clear();
				includedScenesNames.clear();
				selectedMainScene = "";
				fullscreen = false;
				vsync = false;
			}
			ImGui::SameLine();
			ImGui::BeginDisabled(sceneInclusionMap.empty());
			if (ImGui::Button("Build", ImVec2(120, 0)))
			{
				nlohmann::json build_config;

				build_config["app_name"] = app_name;
				build_config["icon_path"] = icon_path;
				int i = 0;
				for (auto& scene : includedScenesNames)
				{
					build_config["scenes"]["included"][i] = "Assets" + scene;
					i++;
				}
				build_config["scenes"]["main"] = "Assets" + selectedMainScene;

				build_config["window"]["fullscreen"] = fullscreen;
				build_config["window"]["vsync"] = vsync;

				try
				{
					std::filesystem::create_directories("Config/Build_Resources");

					std::ofstream output("Config/Build_Resources/ManLite.init");
					output << build_config.dump(4);
					output.close();

					LOG(LogType::LOG_OK, "Build configuration saved successfully!");
				}
				catch (const std::exception& e)
				{
					LOG(LogType::LOG_ERROR, "Error saving build config: %s", e.what());
				}


				std::string buildFolder = "Build";
#pragma region BUILD_EXE_MODE

				if (std::filesystem::exists(buildFolder))
				{
					std::filesystem::remove_all(buildFolder);
					LOG(LogType::LOG_INFO, "Deleted existing Build folder");
				}

				try
				{
					std::filesystem::create_directories(buildFolder);

					std::string sourceExe = "Config/Build_Resources/ManLiteBuilder.exe";
					std::string destExe = buildFolder + "/" + std::string(app_name) + ".exe";

					if (std::filesystem::exists(sourceExe))
					{
						std::filesystem::copy(sourceExe, destExe,
							std::filesystem::copy_options::overwrite_existing);
					}

					if (!icon_path.empty())
					{
						std::string cmd = "ResourceHacker.exe -open \"" + destExe +
							"\" -save \"" + destExe +
							"\" -action addoverwrite -res \"" +
							icon_path + "\" -mask ICONGROUP,1,";
						system(cmd.c_str());
					}

					std::filesystem::copy("Config/Build_Resources/ManLite.init",
						buildFolder + "/ManLite.init",
						std::filesystem::copy_options::overwrite_existing);

					auto copyAssets = [](const std::filesystem::path& src, const std::filesystem::path& dst)
						{
							for (const auto& entry : std::filesystem::recursive_directory_iterator(src))
							{
								const auto& path = entry.path();
								auto relativePath = std::filesystem::relative(path, src);
								std::string ext = path.extension().string();

								if (ext == ".mlscene" || ext == ".cs") continue;

								if (entry.is_directory())
								{
									std::filesystem::create_directories(dst / relativePath);
								}
								else
								{
									std::filesystem::copy(path, dst / relativePath,
										std::filesystem::copy_options::overwrite_existing);
								}
							}
						};
					copyAssets("Assets", buildFolder + "/Assets");

					for (const auto& sceneName : includedScenesNames)
					{
						try
						{
							std::filesystem::path destPath("Build/Assets" + sceneName);
							std::filesystem::path sourcePath("Assets" + sceneName);

							std::filesystem::copy(sourcePath, destPath,
								std::filesystem::copy_options::overwrite_existing);

							LOG(LogType::LOG_INFO, "Scene copied: %s", sceneName.c_str());
						}
						catch (const std::exception& e)
						{
							LOG(LogType::LOG_ERROR, "Failed to copy scene %s: %s", sceneName.c_str(), e.what());
						}
					}

					std::string exePath = []()
						{
#ifdef _WIN32
						char path[MAX_PATH];
						GetModuleFileNameA(NULL, path, MAX_PATH);
						return std::filesystem::path(path).parent_path().string();
#else
						return std::filesystem::current_path().string();
#endif
						}();

					for (const auto& entry : std::filesystem::directory_iterator(exePath))
					{
						if (entry.path().extension() == ".dll")
						{
							std::filesystem::copy(entry.path(),
								buildFolder + "/" + entry.path().filename().string(),
								std::filesystem::copy_options::overwrite_existing);
						}
					}

					if (std::filesystem::exists(exePath + "/mono"))
					{
						std::filesystem::copy(exePath + "/mono",
							buildFolder + "/mono",
							std::filesystem::copy_options::recursive |
							std::filesystem::copy_options::overwrite_existing);
					}

					LOG(LogType::LOG_OK, "Build completed successfully!");
				}
				catch (const std::exception& e)
				{
					LOG(LogType::LOG_ERROR, "Build failed: %s", e.what());
				}

#pragma endregion BUILD_EXE_MODE

				if (icon_texture != 0 || !icon_path.empty())
					ResourceManager::GetInstance().ReleaseTexture(icon_path);

				icon_path = "";
				showBuildPanel = false;
				icon_texture = 0;
				memset(app_name, 0, sizeof(app_name));
				sceneInclusionMap.clear();
				includedScenesNames.clear();
				selectedMainScene = "";
				fullscreen = false;
				vsync = false;
			}
			ImGui::EndDisabled();

			ImGui::PopStyleColor();
			ImGui::PopStyleVar(2);
			ImGui::EndPopup();
		}
	}
}

void Gui::AutoSaveScenePanel()
{
	if (show_auto_save_panel)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 viewport_pos = viewport->Pos;
		ImVec2 viewport_size = viewport->Size;

		const ImVec2 PADDING(20, 200);
		const ImVec2 PANEL_SIZE(400, 40);
		ImVec2 window_pos(
			viewport_pos.x + viewport_size.x - PADDING.x - PANEL_SIZE.x,
			viewport_pos.y + viewport_size.y - PADDING.y - PANEL_SIZE.y
		);

		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(PANEL_SIZE, ImGuiCond_Always);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoCollapse;

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.12f, 0.12f, 0.95f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.65f, 0.15f, 0.55f, 1.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 10));

		if (ImGui::Begin("AutoSave", nullptr, flags))
		{
			ImGui::Columns(2, "##autosave_columns", false);
			ImGui::SetColumnWidth(0, 120.0f);

			ImGui::AlignTextToFramePadding();
			ImGui::TextColored(ImVec4(0.85f, 0.35f, 0.75f, 1.0f), "AUTO SAVE");

			ImGui::NextColumn();

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - 30) * 0.5f - 15);
			ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.9f, 1.0f), "%d", static_cast<int>(countdown_timer) + 1);

			ImGui::Columns(1);

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
			ImGui::Text("Saving scene in %d seconds...", static_cast<int>(countdown_timer) + 1);

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
			float progress = 1.0f - (countdown_timer / 5.0f);
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.75f, 0.25f, 0.65f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));

			char progress_text[32];
			sprintf(progress_text, "%.0f%%", progress * 100);
			ImGui::ProgressBar(progress, ImVec2(-1, 16), progress_text);

			ImGui::PopStyleColor(2);
		}
		ImGui::End();

		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor(2);
	}
}

void Gui::AutoSaveSceneBg(float dt)
{
	if (engine->GetEngineState() == EngineState::STOP)
	{
		auto_save_timer += dt;

		//auto save every 5 minutes
		if (auto_save_timer >= (60.0f * 5) && !show_auto_save_panel)
		{
			show_auto_save_panel = true;
			countdown_timer = 5.0f;
			auto_save_timer = 0.0f;
		}

		if (show_auto_save_panel)
		{
			countdown_timer -= dt;

			if (countdown_timer <= 0.0f)
			{
				show_auto_save_panel = false;

				std::string filePath = engine->scene_manager_em->GetCurrentScene().GetScenePath();
				if (filePath.empty())
					filePath = "Assets\\Scenes\\" + engine->scene_manager_em->GetCurrentScene().GetSceneName() + ".mlscene";

				if (!filePath.empty())
				{
					std::filesystem::path fullPath(filePath);
					std::string sceneName = fullPath.stem().string();
					std::string directory = fullPath.parent_path().string();

					if (!directory.empty() && directory.back() != std::filesystem::path::preferred_separator)
						directory += std::filesystem::path::preferred_separator;

					engine->scene_manager_em->SaveScene(directory, sceneName);
				}
			}
		}
	}
}

void Gui::RestartAutoSaveTimer()
{
	auto_save_timer = 0.0f;
	countdown_timer = 0.0f;
	show_auto_save_panel = false;
}

void Gui::HandleShortcut()
{
	if (ImGui::GetIO().WantTextInput) return;

	if (engine->input_em->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT &&
		engine->input_em->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT &&
		engine->input_em->GetKey(SDL_SCANCODE_N) == KEY_DOWN)
	{
		if (engine->scene_manager_em->GetCurrentScene().GetSelectedGOs().empty())
		{
			engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(engine->scene_manager_em->GetCurrentScene().GetSceneRoot());
		}
		else
		{
			for (const auto& go : engine->scene_manager_em->GetCurrentScene().GetSelectedGOs())
				engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(*go.lock());
		}
	}
	else if (engine->input_em->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT &&
		engine->input_em->GetKey(SDL_SCANCODE_N) == KEY_DOWN)
	{
		save_scene_panel->RequestFocus();
		save_scene_panel->save_panel_action = SavePanelAction::NEW_SCENE;
	}
	else if (engine->input_em->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT &&
		engine->input_em->GetKey(SDL_SCANCODE_O) == KEY_DOWN)
	{
		save_scene_panel->RequestFocus();
		save_scene_panel->save_panel_action = SavePanelAction::OPEN_SCENE;
	}
	else if (engine->input_em->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT &&
		engine->input_em->GetKey(SDL_SCANCODE_S) == KEY_DOWN)
	{
		std::string filePath = engine->scene_manager_em->GetCurrentScene().GetScenePath();
		if (filePath.empty() || filePath == "")
			filePath = "Assets\\Scenes\\" + engine->scene_manager_em->GetCurrentScene().GetSceneName() + ".mlscene";
		if (!filePath.empty() && filePath != "")
		{
			std::filesystem::path fullPath(filePath);
			std::string sceneName = fullPath.stem().string();
			std::string directory = fullPath.parent_path().string();

			if (!directory.empty() && directory.back() != std::filesystem::path::preferred_separator)
				directory += std::filesystem::path::preferred_separator;

			engine->scene_manager_em->SaveScene(directory, sceneName);
			RestartAutoSaveTimer();
		}
	}
	else if (engine->input_em->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT &&
		engine->input_em->GetKey(SDL_SCANCODE_D) == KEY_DOWN)
	{
		auto& scene = engine->scene_manager_em->GetCurrentScene();
		const auto& selected = scene.GetSelectedGOs();

		std::vector<std::shared_ptr<GameObject>> new_selected;
		for (const auto& weakGo : selected)
		{
			if (auto go = weakGo.lock())
			{
				new_selected.push_back(scene.DuplicateGO(*go));
			}
		}
		scene.SelectGameObject(nullptr, false, true);
		for (const auto& go : new_selected)
		{
			scene.SelectGameObject(go, true);
		}
	}
	else if (engine->input_em->GetKey(SDL_SCANCODE_BACKSPACE) == KEY_DOWN ||
		engine->input_em->GetKey(SDL_SCANCODE_DELETE) == KEY_DOWN)
	{
		std::vector<std::weak_ptr<GameObject>> selected_gos = engine->scene_manager_em->GetCurrentScene().GetSelectedGOs();
		for (const auto& go : selected_gos)
		{
			if (auto map = go.lock()->GetComponent<TileMap>())
			{
				if (tile_map_panel->GetMap() && map->GetID() == tile_map_panel->GetMap()->GetID())
					tile_map_panel->SetMap(nullptr);
			}
			engine->scene_manager_em->GetCurrentScene().SafeDeleteGO(go.lock());
		}
		engine->scene_manager_em->GetCurrentScene().UnselectAll();
	}
}

void Gui::HandleInput()
{
	//for (auto& event : engine->input_em->GetSDLEvents())
	//	ImGui_ImplSDL2_ProcessEvent(&event);

	for (auto& event : engine->input_em->GetSDLEvents())
	{
		if (event.type == SDL_WINDOWEVENT)
		{
			if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST ||
				event.window.event == SDL_WINDOWEVENT_HIDDEN ||
				event.window.event == SDL_WINDOWEVENT_MINIMIZED)
			{
				if (engine->input_em->IsAppFocused())
				{
					continue;
				}
			}
		}
		ImGui_ImplSDL2_ProcessEvent(&event);
	}
}

void Gui::ProcessEvent()
{
}