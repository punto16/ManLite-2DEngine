#ifndef __SCRIPTING_EM_H__
#define __SCRIPTING_EM_H__
#pragma once

#include "EngineModule.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/mono-gc.h"

#include "string"
#include "unordered_map"

class GameObject;
class Script;

class ScriptingEM : public EngineModule
{
public:
	struct MonoData
	{
		MonoDomain* monoDomain = nullptr;
		MonoDomain* monoRootDomain = nullptr;
		//core
		MonoAssembly* coreAssembly = nullptr;
		MonoImage* coreAssemblyImage = nullptr;
		//user
		MonoAssembly* userAssembly = nullptr;
		MonoImage* userAssemblyImage = nullptr;
		//utils
		GameObject* currentGOPtr = nullptr;
	};

	struct InstantiateQueueData
	{
		std::string class_name = "";
		unsigned int container_go_id = 0;
	};


	ScriptingEM(EngineCore* parent);
	virtual ~ScriptingEM();

	bool Awake();
	bool PreUpdate();
	bool Start();

	bool CleanUp();

	static GameObject* GetGOPtr() { return mono_data.currentGOPtr; }
	static MonoDomain* GetAppDomain() { return mono_data.monoDomain; }
	static MonoImage* GetCoreAssemblyImage() { return mono_data.coreAssemblyImage; }

	//
	MonoObject* InstantiateClass(const std::string& class_name, Script* container_script);
	MonoObject* InstantiateClassAsync(const std::string& class_name, Script* container_script);
	void CallScriptFunction(Script* container_script, MonoObject* mono_object, const std::string& function_name, void** params = nullptr, int num_params = 0);
	void ReleaseMonoObject(MonoObject* mono_object);

	//
	bool CompileUserScripts();
	//
	void RecompileScripts();

	//
	bool ScriptsCompiledSuccessfully() { return script_ready; }

private:
	std::string GetAssemblyPath();
	std::string GetUserAssemblyPath();
	std::string GetMonoAssembliesPath();
	std::string GetMCSPath();
	//

	static MonoData mono_data;

	//
	std::unordered_map<MonoObject*, uint32_t> mono_gc_handles;
	std::vector<Script*> active_scripts;

	bool script_ready = true;

	//to load async
	std::vector<InstantiateQueueData> instantiate_queue;
	void ProcessInstantiateQueue();

public:
	bool stop_process_instantiate_queue = false;

	// Helpers para conversión de tipos
	static void* ToMonoParam(const auto& value);
	static void* ToMonoStringParam(const std::string& str);
	static void* ToMonoGameObjectParam(GameObject* go, const std::string& script_name);
};

#endif // !__SCRIPTING_EM_H__