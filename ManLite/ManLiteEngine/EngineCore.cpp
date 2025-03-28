#include "EngineCore.h"

#include "EngineModule.h"
#include "WindowEM.h"
#include "InputEM.h"
#include "RendererEM.h"
#include "SceneManagerEM.h"
#include "AudioEM.h"
#include "ResourceManager.h"

#include "Defs.h"
#include "Log.h"

EngineCore::EngineCore()
{
	//create engine modules
	window_em = new WindowEM(this);
	input_em = new InputEM(this);
	scene_manager_em = new SceneManagerEM(this);
	audio_em = new AudioEM(this);

	//renderer last
	renderer_em = new RendererEM(this);


	//------add modules------
	AddEngineModule(window_em, true);
	AddEngineModule(input_em, true);
	AddEngineModule(audio_em, true);
	AddEngineModule(scene_manager_em, true);

	//renderer last
	AddEngineModule(renderer_em, true);
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

	return ret;
}

void EngineCore::CleanUp()
{
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
