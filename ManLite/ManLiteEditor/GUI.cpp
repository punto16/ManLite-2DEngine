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
#include "Collider2D.h"
#include "AudioSource.h"
#include "Animator.h"
#include "ParticleSystem.h"
#include "TileMap.h"

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

	HandleShortcut();

	MainMenuBar();

	//test window
	ImGui::ShowDemoWindow();

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
		save_scene_panel->set_scene = false;
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

		ImGui::Dummy(ImVec2(40.0f, 0.0f));
		if (engine->GetEngineState() == EngineState::STOP && ImGui::Button("Reload Scripts"))
		{
			engine->scripting_em->RecompileScripts();
		}
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
		if (!filePath.empty() && filePath != "")
		{
			std::filesystem::path fullPath(filePath);
			std::string sceneName = fullPath.stem().string();
			std::string directory = fullPath.parent_path().string();

			if (!directory.empty() && directory.back() != std::filesystem::path::preferred_separator)
				directory += std::filesystem::path::preferred_separator;

			engine->scene_manager_em->SaveScene(directory, sceneName);
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
		}
	}
	if (ImGui::BeginMenu("Import"))
	{
		bool tiled_file = ImGui::MenuItem("Tiled File");
		ImGui::SameLine();
		Gui::HelpMarker("Import a tiled file exported as .json\nIMPORTANT: the tiled file must contain only 1 TileSet and Orthogonal");
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

	if (ImGui::MenuItem("Build", 0, false, false))
	{

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
	if (ImGui::MenuItem("Canvas"))
	{
		GameObject* e_go = engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(engine->scene_manager_em->GetCurrentScene().GetSceneRoot()).get();
		e_go->SetName("Canvas");
		e_go->AddComponent<Canvas>();
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

	if (ImGui::MenuItem("Animator"))
	{
		for (const auto& item : engine->scene_manager_em->GetCurrentScene().GetSelectedGOs())
			item.lock()->AddComponent<Animator>();
	}

	if (ImGui::MenuItem("TileMap"))
	{
		for (const auto& item : engine->scene_manager_em->GetCurrentScene().GetSelectedGOs())
		{
			item.lock()->AddComponent<TileMap>();
			item.lock()->GetComponent<TileMap>()->SwapTexture("Assets\\TileMaps\\platformer_tileset.png");
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
		if (!filePath.empty() && filePath != "")
		{
			std::filesystem::path fullPath(filePath);
			std::string sceneName = fullPath.stem().string();
			std::string directory = fullPath.parent_path().string();

			if (!directory.empty() && directory.back() != std::filesystem::path::preferred_separator)
				directory += std::filesystem::path::preferred_separator;

			engine->scene_manager_em->SaveScene(directory, sceneName);
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
				if (map->GetID() == tile_map_panel->GetMap()->GetID())
					tile_map_panel->SetMap(nullptr);
			}
			engine->scene_manager_em->GetCurrentScene().SafeDeleteGO(go.lock());
		}
		engine->scene_manager_em->GetCurrentScene().UnselectAll();
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