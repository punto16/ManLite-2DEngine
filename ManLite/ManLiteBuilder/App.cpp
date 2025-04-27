#include "App.h"

#include "../ManLiteEngine/EngineCore.h"
#include "../ManLiteEngine/SceneManagerEM.h"
#include "../ManLiteEngine/Defs.h"
#include "../ManLiteEngine/WindowEM.h"

EngineCore* engine = NULL;

App::App(int argc, char* args[]) : argc(argc), args(args)
{
	//create modules
	engine = new EngineCore();
	engine->SetEditorOrBuild(false); //important! tell the engine we are in BUILD! mode

	//timer
	time_since_start = 0.0f;
	start_timer = new Timer();
}

App::~App()
{
}

bool App::Awake()
{
	bool ret = true;

	targetFrameDuration = (std::chrono::duration<double>)1 / frameRate;
	title = TITLE;
	organization = ORGANIZATION;

	engine->Awake();

	return ret;
}

bool App::Start()
{
	dt = 0;
	start_timer->Start();

	engine->Start();

	//read here from file whats the first scene and games name

	engine->window_em->SetTitle(game_name);

	engine->scene_manager_em->LoadSceneFromJson(first_scene);
	if (engine->scene_manager_em->CurrentSceneAvailable())
	{
		engine->scene_manager_em->StartSession();
	}

	return true;
}

bool App::Update()
{
	bool ret = true;
	PrepareUpdate();

	if (ret) ret = PreUpdate();

	if (ret) ret = DoUpdate();

	if (ret) ret = PostUpdate();

	FinishUpdate();

	time_since_start = start_timer->ReadSec();

	return ret;
}

void App::PrepareUpdate()
{
	frameStart = std::chrono::steady_clock::now();
}

bool App::PreUpdate()
{
	bool ret = true;

	if (!engine->PreUpdate()) return false;

	return true;
}

bool App::DoUpdate()
{
	bool ret = true;

	if (!engine->Update(dt)) return false;

	return true;
}

bool App::PostUpdate()
{
	bool ret = true;

	if (!engine->PostUpdate()) return false;

	return true;
}

void App::FinishUpdate()
{
	frameEnd = std::chrono::steady_clock::now();
	auto frameDuration = std::chrono::duration_cast<std::chrono::duration<double>>(frameEnd - frameStart);

	dt = frameDuration.count();

	if (frameDuration < targetFrameDuration)
	{
		std::chrono::duration<double> sleepTime = targetFrameDuration - frameDuration;
		std::this_thread::sleep_for(sleepTime);

		dt = targetFrameDuration.count();
	}

	dtCount += dt;
	frameCount++;

	if (dtCount >= 1)
	{
		fps = frameCount;
		frameCount = 0;
		dtCount = 0;
	}
	//test only
	engine->window_em->SetTitle(game_name + " | FPS: " + std::to_string(fps));
}

bool App::CleanUp()
{
	bool ret = true;
	RELEASE(start_timer);

	engine->CleanUp();

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

int App::GetFrameRate() const
{
	return frameRate;
}

void App::SetFrameRate(int frameRate)
{
	this->frameRate = frameRate;
	targetFrameDuration = (std::chrono::duration<double>)1 / this->frameRate;
}

double App::GetDT() const
{
	return dt;
}

void App::SetDT(double dt)
{
	this->dt = dt;
}