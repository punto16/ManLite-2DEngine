#ifndef __ENGINE_CORE_H__
#define __ENGINE_CORE_H__
#pragma once

#include "Log.h"
#include "Timer.h"
#include <list>
#include <vector>

//EM means Engine Module
class EngineModule;
class WindowEM;
class InputEM;
class RendererEM;
class SceneManagerEM;
class AudioEM;
class PhysicsEM;
class FontEM;
class ScriptingEM;

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
	// true -> we are in editor
	// false -> we are in build
	void SetEditorOrBuild(bool b) { editor_or_build = b; }
	// true -> we are in editor
	// false -> we are in build
	bool GetEditorOrBuild() { return editor_or_build; }

	float GetTimeScale() { return scale_time; }
	void SetTimeScale(float t) { scale_time = t; }

	float GetGameTime() { return game_time; }

private:


public:
	//engine modules
	WindowEM* window_em = nullptr;
	InputEM* input_em = nullptr;
	RendererEM* renderer_em = nullptr;
	SceneManagerEM* scene_manager_em = nullptr;
	AudioEM* audio_em = nullptr;
	PhysicsEM* physics_em = nullptr;
	FontEM* font_em = nullptr;
	ScriptingEM* scripting_em = nullptr;

private:

	EngineState engine_state = EngineState::STOP;
	// true -> we are in editor
	// false -> we are in build
	bool editor_or_build = true;

	//time management
	double dt;
	Timer* game_timer;
	float game_time;
	float scale_time;

	std::list<EngineModule*> engine_modules;

	std::vector<LogInfo> logs;
};

extern EngineCore* engine;

#endif // !__ENGINE_CORE_H__