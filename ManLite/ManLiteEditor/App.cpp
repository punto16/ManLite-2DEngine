#include "App.h"

#include "GUI.h"

#include "../ManLiteEngine/EngineCore.h"

EngineCore* engine = NULL;

App::App(int argc, char* args[]) : argc(argc), args(args)
{
	//create modules
	engine = new EngineCore();
	gui = new Gui(this);

	//add modules

	//render module last to swap buffer
	AddModule(gui, true);

	//state = GameState::NONE;
	//time_since_start = 0.0F;
	//game_time = 0.0F;
	//scale_time = 1.0F;
	//start_timer = new Timer();
	//game_timer = new Timer();
}

App::~App()
{
	modules.clear();
}

void App::AddModule(Module* module, bool activate)
{
	if (activate) module->Init();

	modules.push_back(module);
}

bool App::Awake()
{
	bool ret = false;

	//targetFrameDuration = (std::chrono::duration<double>)1 / frameRate;
	title = "ManLite 2D Engine";
	organization = "punto16";


	engine->Awake();
	for (auto& item : modules)
	{
		if (!item->active) continue;
		item->Awake();
	}

	return ret;
}

bool App::Start()
{
	//dt = 0;
	//start_timer->Start();

	engine->Start();

	for (auto& item : modules)
	{
		if (!item->active) continue;
		if (!item->Start()) return false;
	}
	return true;
}

bool App::Update()
{
	bool ret = true;
	//PrepareUpdate();

	if (ret) ret = PreUpdate();

	if (ret) ret = Update();

	if (ret) ret = PostUpdate();

	//FinishUpdate();

	//time_since_start = start_timer->ReadSec();

	//if (state == GameState::PLAY || state == GameState::PLAY_ONCE) {
	//	game_time = game_timer->ReadSec(scale_time);
	//}

	return ret;
}

//void App::PrepareUpdate()
//{
//	frameStart = std::chrono::steady_clock::now();
//}

bool App::PreUpdate()
{
	bool ret = true;

	if (!engine->PreUpdate()) return false;

	for (auto& item : modules)
	{
		if (!item->active) continue;
		if (!item->PreUpdate()) return false;		
	}
	return true;
}

bool App::DoUpdate()
{
	bool ret = true;

	if (!engine->Update(/*dt*/0.0166f)) return false;

	for (auto& item : modules)
	{
		if (!item->active) continue;
		if (!item->Update(/*dt*/0.0166f)) return false;
	}
	return true;
}

bool App::PostUpdate()
{
	bool ret = true;

	if (!engine->PostUpdate()) return false;

	for (auto& item : modules)
	{
		if (!item->active) continue;
		if (!item->PostUpdate()) return false;
	}
	return true;
}

//bool App::FinishUpdate()
//{
//	frameEnd = std::chrono::steady_clock::now();
//	auto frameDuration = std::chrono::duration_cast<std::chrono::duration<double>>(frameEnd - frameStart);
//
//	dt = frameDuration.count();
//
//	if (frameDuration < targetFrameDuration)
//	{
//		std::chrono::duration<double> sleepTime = targetFrameDuration - frameDuration;
//		std::this_thread::sleep_for(sleepTime);
//
//		dt = targetFrameDuration.count();
//	}
//
//	dtCount += dt;
//	frameCount++;
//
//	if (dtCount >= 1)
//	{
//		fps = frameCount;
//		frameCount = 0;
//		dtCount = 0;
//	}
//
//	app->gui->panelSettings->AddFpsValue(fps);
//}

bool App::CleanUp()
{
	bool ret = true;

	//if (start_timer != nullptr)
	//	delete start_timer;
	//if (game_timer != nullptr)
	//	delete game_timer;

	engine->CleanUp();
	for (auto item = modules.rbegin(); item != modules.rend(); ++item)
	{
		Module* module = *item;
		module->CleanUp();
		delete module;
	}
	modules.clear();

	return ret;
}

int App::GetArgc() const
{
	return argc;
}

const char* App::GetArgv(int index) const
{
	if (index < argc)
		return args[index];
	else
		return NULL;
}

//int App::GetFrameRate() const
//{
//	return frameRate;
//}

//void App::SetFrameRate(int frameRate)
//{
//	this->frameRate = frameRate == 0 ? engine->window->GetDisplayRefreshRate() : frameRate;
//	targetFrameDuration = (std::chrono::duration<double>)1 / this->frameRate;
//}

//double App::GetDT() const
//{
//	return dt;
//}

//void App::SetDT(double dt)
//{
//	this->dt = dt;
//}

//void App::Play()
//{
//	static std::string actual_scene_name;
//	switch (state)
//	{
//	case GameState::NONE:
//	{
//		state = GameState::PLAY;
//
//		game_time = 0.0F;
//		game_timer->Start();
//
//		LOG(LogType::LOG_INFO, "GameState changed to PLAY");
//		break;
//	}
//	case GameState::PLAY:
//	{
//		state = GameState::NONE;
//		game_time = 0.0F;
//
//		LOG(LogType::LOG_INFO, "GameState changed to NONE");
//		break;
//	}
//	case GameState::PAUSE:
//	{
//		state = GameState::PLAY;
//		game_timer->Resume();
//
//		LOG(LogType::LOG_INFO, "GameState changed to PLAY");
//		break;
//	}
//	case GameState::PLAY_ONCE:
//	{
//
//		break;
//	}
//	default:
//		break;
//	}
//}

//void App::Pause()
//{
//	if (state == GameState::PAUSE) {
//		Play();
//	}
//	else if (state == GameState::PLAY || state == GameState::PLAY_ONCE) {
//		state = GameState::PAUSE;
//		game_timer->Pause();
//
//		LOG(LogType::LOG_INFO, "GameState changed to PAUSE");
//	}
//}

//void App::PlayOnce()
//{
//	if (state == GameState::PAUSE) {
//		game_timer->Resume();
//		state = GameState::PLAY_ONCE;
//
//		LOG(LogType::LOG_INFO, "GameState changed to PLAY_ONCE");
//	}
//	else if (state == GameState::PLAY) {
//		state = GameState::PLAY_ONCE;
//		LOG(LogType::LOG_INFO, "GameState changed to PLAY_ONCE");
//	}
//}

//bool App::IsPlaying()
//{
//	if (state == GameState::PLAY || state == GameState::PLAY_ONCE)
//		return true;
//	else
//		return false;
//}

//bool App::IsInGameState()
//{
//	return state != GameState::NONE;
//	LOG(LogType::LOG_INFO, "GameState changed to NONE");
//}

//void App::Stop()
//{
//	game_time = 0.0F;
//	state = GameState::NONE;
//	LOG(LogType::LOG_INFO, "GameState changed to NONE");
//}