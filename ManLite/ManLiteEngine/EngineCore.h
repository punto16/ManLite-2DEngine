#ifndef __ENGINE_CORE_H__
#define __ENGINE_CORE_H__
#pragma once

#include "Log.h"

#include <list>
#include <vector>

//EM means Engine Module
class EngineModule;
class WindowEM;
class InputEM;
class RendererEM;
class SceneManagerEM;
class AudioEM;


enum EngineState
{
	PLAY,
	PAUSE,
	STOP,

	//
	COUNT
};

class EngineCore
{
public:
	EngineCore();
	~EngineCore();

	void AddEngineModule(EngineModule* engine_module, bool activate);

	void Awake();
	void Start();
	
	bool PreUpdate();
	bool Update(double dt);
	bool PostUpdate();

	void CleanUp();

	//Logs
	std::vector<LogInfo> GetLogs();
	void AddLog(LogType type, const char* entry);
	void CleanLogs();

	//getters // setters
	double GetDT() const { return this->dt; }

	void SetEngineState(EngineState new_state);
	EngineState GetEngineState() { return engine_state; }

private:


public:
	//engine modules
	WindowEM* window_em = nullptr;
	InputEM* input_em = nullptr;
	RendererEM* renderer_em = nullptr;
	SceneManagerEM* scene_manager_em = nullptr;
	AudioEM* audio_em = nullptr;

private:

	EngineState engine_state = EngineState::STOP;

	double dt;

	std::list<EngineModule*> engine_modules;

	std::vector<LogInfo> logs;
};

extern EngineCore* engine;

#endif // !__ENGINE_CORE_H__