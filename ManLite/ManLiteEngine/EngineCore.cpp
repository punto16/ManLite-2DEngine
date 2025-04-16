#include "EngineCore.h"

#include "EngineModule.h"
#include "WindowEM.h"
#include "InputEM.h"
#include "RendererEM.h"
#include "SceneManagerEM.h"
#include "AudioEM.h"
#include "ResourceManager.h"
#include "PhysicsEM.h"
#include "FontEM.h"
#include "ScriptingEM.h"

#include "Defs.h"
#include "Log.h"

EngineCore::EngineCore()
{
	//create engine modules
	window_em = new WindowEM(this);
	input_em = new InputEM(this);
	audio_em = new AudioEM(this);
	font_em = new FontEM(this);
	physics_em = new PhysicsEM(this);
	scripting_em = new ScriptingEM(this);
	scene_manager_em = new SceneManagerEM(this);

	//renderer last
	renderer_em = new RendererEM(this);


	//------add modules------
	AddEngineModule(window_em, true);
	AddEngineModule(input_em, true);
	AddEngineModule(audio_em, true);
	AddEngineModule(font_em, true);
	AddEngineModule(physics_em, true);
	AddEngineModule(scripting_em, true);
	AddEngineModule(scene_manager_em, true);

	//renderer last
	AddEngineModule(renderer_em, true);

	//timer
	scale_time = 1.0f;
	game_time = 0.0f;
	game_timer = new Timer();
}

EngineCore::~EngineCore()
{
	engine_modules.clear();
}

void EngineCore::AddEngineModule(EngineModule* engine_module, bool activate)
{
	if (activate) engine_module->Init();

	engine_modules.push_back(engine_module);
}

void EngineCore::Awake()
{
	for (auto& item : engine_modules)
	{
		if (!item->active) continue;
		item->Awake();
	}
	LOG(LogType::LOG_OK, "EngineCore: Initialization Success");
}

void EngineCore::Start()
{
	for (auto& item : engine_modules)
	{
		if (!item->active) continue;
		item->Start();
	}
}

bool EngineCore::PreUpdate()
{
	bool ret = true;

	ResourceManager::GetInstance().ProcessTextures();

	for (auto& item : engine_modules)
	{
		if (!item->active) continue;
		if (!item->PreUpdate()) return false;
	}

	return ret;
}

bool EngineCore::Update(double dt)
{
	bool ret = true;
	this->dt = dt;

	for (auto& item : engine_modules)
	{
		if (!item->active) continue;
		if (!item->Update(dt)) return false;
	}

	return ret;
}

bool EngineCore::PostUpdate()
{
	bool ret = true;

	for (auto& item : engine_modules)
	{
		if (!item->active) continue;
		if (!item->PostUpdate()) return false;
	}

	if (engine_state == EngineState::PLAY)
	{
		game_time = game_timer->ReadSec(scale_time);
	}

	return ret;
}

void EngineCore::CleanUp()
{
	RELEASE(game_timer)

	for (auto item = engine_modules.rbegin(); item != engine_modules.rend(); ++item)
	{
		EngineModule* module = *item;
		module->CleanUp();
		RELEASE(module)
	}
	engine_modules.clear();
}

std::vector<LogInfo> EngineCore::GetLogs()
{
	return logs;
}

void EngineCore::AddLog(LogType type, const char* entry)
{
	if (logs.size() > MAX_LOGS_CONSOLE)
		logs.erase(logs.begin());

	std::string toAdd = entry;
	LogInfo info = { type, toAdd };

	logs.push_back(info);
}

void EngineCore::CleanLogs()
{
	logs.clear();
}

void EngineCore::SetEngineState(EngineState new_state)
{
	switch (engine_state)
	{
	case PLAY:
	{
		//
		switch (new_state)
		{
		case PLAY: //play to play do nothing
		{
			
			break;
		}
		case PAUSE: //play to pause stops updates
		{
			game_timer->Pause();
			scene_manager_em->GetCurrentScene().Pause();
			LOG(LogType::LOG_INFO, "EngineCore: State from Play to Pause");
			break;
		}
		case STOP: //play to stop takes pre-play scene to front
		{
			game_time = 0.0f;
			scene_manager_em->StopSession();
			LOG(LogType::LOG_INFO, "EngineCore: State from Play to Stop");
			break;
		}
		case COUNT:
			break;
		default:
			break;
		}
		break;
		//
	}
	case PAUSE:
	{
		//
		switch (new_state)
		{
		case PLAY: //pause to play resume updates
		{
			game_timer->Resume();
			scene_manager_em->GetCurrentScene().Unpause();
			LOG(LogType::LOG_INFO, "EngineCore: State from Pause to Play");
			break;
		}
		case PAUSE: //pause to pause do nothing
		{
			
			break;
		}
		case STOP: //pause to stop takes pre-play scene to front
		{
			game_time = 0.0f;
			scene_manager_em->StopSession();
			LOG(LogType::LOG_INFO, "EngineCore: State from Pause to Stop");
			break;
		}
		case COUNT:
			break;
		default:
			break;
		}
		break;
		//
	}
	case STOP:
	{
		//
		switch (new_state)
		{
		case PLAY: //stop to play init things and resume updates
		{
			game_time = 0.0f;
			game_timer->Start();
			scene_manager_em->StartSession();
			LOG(LogType::LOG_INFO, "EngineCore: State from Stop to Play");
			break;
		}
		case PAUSE: //stop to pause do nothing
		{
			
			break;
		}
		case STOP: //stop to stop do nothing
		{
			
			break;
		}
		case COUNT:
			break;
		default:
			break;
		}
		break;
		//
	}
	case COUNT:
		break;
	default:
		break;
	}

	//update to new state
	engine_state = new_state;
}
