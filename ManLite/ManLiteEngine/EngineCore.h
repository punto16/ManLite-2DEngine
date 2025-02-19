#ifndef __ENGINE_CORE_H__
#define __ENGINE_CORE_H__
#pragma once

#include <list>

//EM means Engine Module

class EngineModule;
class WindowEM;

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

private:


public:
	//engine modules
	WindowEM* window_em = nullptr;

private:

	std::list<EngineModule*> engine_modules;

};

extern EngineCore* engine;

#endif // !__ENGINE_CORE_H__