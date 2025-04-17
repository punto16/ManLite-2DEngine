#ifndef __SCRIPTING_EM_H__
#define __SCRIPTING_EM_H__
#pragma once

#include "EngineModule.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"

#include "string"
#include "unordered_map"

class GameObject;

class ScriptingEM : public EngineModule
{
public:
	struct MonoData
	{
		MonoDomain* monoDomain = nullptr;
		MonoDomain* monoRootDomain = nullptr;
		MonoAssembly* coreAssembly = nullptr;
		MonoImage* coreAssemblyImage = nullptr;

		GameObject* currentGOPtr = nullptr;
	};


	ScriptingEM(EngineCore* parent);
	virtual ~ScriptingEM();

	bool Awake();
	bool Start();

	bool CleanUp();

	static GameObject* GetGOPtr() { return mono_data.currentGOPtr; }
	static MonoDomain* GetAppDomain() { return mono_data.monoDomain; }

	//
	MonoObject* InstantiateClass(const std::string& class_name, GameObject* container_go);
	void CallScriptFunction(MonoObject* mono_object, const std::string& function_name, void** params = nullptr, int num_params = 0);
	void ReleaseMonoObject(MonoObject* mono_object);

private:
	std::string GetAssemblyPath();
	std::string GetMonoAssembliesPath();

	static MonoData mono_data;

	//
	std::unordered_map<MonoObject*, uint32_t> mono_gc_handles;


public:
	// Helpers para conversión de tipos
	static void* ToMonoParam(const auto& value);
	static void* ToMonoStringParam(const std::string& str);
	static void* ToMonoGameObjectParam(GameObject* go, const std::string& script_name);
};

#endif // !__SCRIPTING_EM_H__