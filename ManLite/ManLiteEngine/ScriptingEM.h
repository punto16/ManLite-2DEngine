#ifndef __SCRIPTING_EM_H__
#define __SCRIPTING_EM_H__
#pragma once

#include "EngineModule.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"

#include "string"

class GameObject;

struct MonoData
{
	MonoDomain* monoDomain = nullptr;
	MonoAssembly* coreAssembly = nullptr;
	MonoImage* coreAssemblyImage = nullptr;

	GameObject* currentGOPtr = nullptr;
};

class ScriptingEM : public EngineModule
{
public:
	ScriptingEM(EngineCore* parent);
	virtual ~ScriptingEM();

	bool Awake();
	bool Start();

	bool CleanUp();

	static GameObject* GetGOPtr() { return mono_data.currentGOPtr; }
	static MonoDomain* GetAppDomain() { return mono_data.monoDomain; }

private:
	std::string GetAssemblyPath();
	std::string GetMonoAssembliesPath();

	static MonoData mono_data;
};

#endif // !__SCRIPTING_EM_H__