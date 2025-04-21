#include "ScriptingEM.h"

#include "MonoRegisterer.h"
#include "GameObject.h"
#include "Script.h"

#include "Log.h"

ScriptingEM::ScriptingEM(EngineCore* parent) : EngineModule(parent)
{
}

ScriptingEM::~ScriptingEM()
{
}

ScriptingEM::MonoData ScriptingEM::mono_data = { nullptr, nullptr, nullptr, nullptr };

bool ScriptingEM::Awake()
{
	bool ret = true;

    mono_set_assemblies_path(GetMonoAssembliesPath().c_str());

	mono_data.monoRootDomain = mono_jit_init("ManLiteScripting");
	if (!mono_data.monoRootDomain)
	{
		LOG(LogType::LOG_ERROR, "ScriptingEM: Awake error, monoDomain failed to initialize");
		return false;
	}

    char appDomainName[] = "MyAppDomain";
    mono_data.monoDomain = mono_domain_create_appdomain(appDomainName, nullptr);
    mono_domain_set(mono_data.monoDomain, true);

    //register internal calls here
    MonoRegisterer::RegisterFunctions();

	return ret;
}

bool ScriptingEM::Start()
{
	bool ret = true;

    mono_data.coreAssembly = mono_domain_assembly_open(mono_data.monoDomain, GetAssemblyPath().c_str());
    if (!mono_data.coreAssembly)
    {
        LOG(LogType::LOG_ERROR, "ScriptingEM: Start error, core assembly failed to initialize");
        CleanUp();
        return false;
    }

    mono_data.coreAssemblyImage = mono_assembly_get_image(mono_data.coreAssembly);

	return ret;
}

bool ScriptingEM::CleanUp()
{
	bool ret = true;

    if (mono_data.monoRootDomain)
    {
        mono_jit_cleanup(mono_data.monoRootDomain);
        mono_data.monoRootDomain = nullptr;
    }

	return ret;
}

MonoObject* ScriptingEM::InstantiateClass(const std::string& class_name, GameObject* container_go)
{
    if (!mono_data.coreAssemblyImage) return nullptr;

    MonoClass* klass = mono_class_from_name(
        mono_data.coreAssemblyImage,
        "",
        class_name.c_str()
    );

    if (!klass)
    {
        LOG(LogType::LOG_ERROR, "Class %s not found", class_name.c_str());
        return nullptr;
    }

    MonoObject* instance = mono_object_new(mono_data.monoDomain, klass);
    if (!instance)
    {
        LOG(LogType::LOG_ERROR, "Failed to create instance of %s", class_name.c_str());
        return nullptr;
    }

    mono_data.currentGOPtr = container_go;
    mono_runtime_object_init(instance);
    mono_data.currentGOPtr = nullptr;

    uint32_t gc_handle = mono_gchandle_new(instance, false);
    mono_gc_handles[instance] = gc_handle;

    return instance;
}

void ScriptingEM::CallScriptFunction(GameObject* container_go, MonoObject* mono_object, const std::string& function_name, void** params, int num_params)
{
    if (!mono_object) return;

    MonoClass* klass = mono_object_get_class(mono_object);
    MonoMethod* method = mono_class_get_method_from_name(klass, function_name.c_str(), num_params);

    if (!method)
    {
        LOG(LogType::LOG_ERROR, "Method %s not found", function_name.c_str());
        return;
    }

    mono_data.currentGOPtr = container_go;
    MonoObject* exception = nullptr;
    mono_runtime_invoke(
        method,
        mono_object,
        params,
        &exception
    );
    mono_data.currentGOPtr = nullptr;

    if (exception)
    {
        MonoString* exc_str = mono_object_to_string(exception, nullptr);
        char* exc_msg = mono_string_to_utf8(exc_str);
        LOG(LogType::LOG_ERROR, "Script Exception: %s", exc_msg);
        mono_free(exc_msg);
    }
}

void ScriptingEM::ReleaseMonoObject(MonoObject* mono_object)
{
    if (!mono_object) return;

    auto it = mono_gc_handles.find(mono_object);
    if (it != mono_gc_handles.end())
    {
        mono_gchandle_free(it->second);
        mono_gc_handles.erase(it);
        mono_object = nullptr;
    }
}

std::string ScriptingEM::GetAssemblyPath()
{
    std::string resultingAssembly;

    char* vsVersion = nullptr;
    size_t len = 0;
    errno_t err = _dupenv_s(&vsVersion, &len, "VisualStudioVersion");

    if (err == 0 && vsVersion != nullptr)
    {
#ifdef _DEBUG

        resultingAssembly = "..\\x64\\Debug\\ManLiteScripting.dll";

#else

        resultingAssembly = "..\\x64\\Release\\ManLiteScripting.dll";

#endif
    }
    else
        resultingAssembly = "ManLiteScripting.dll";

    free(vsVersion);
    return resultingAssembly;
}

std::string ScriptingEM::GetMonoAssembliesPath()
{
    std::string resultingPath;

    char* vsVersion = nullptr;
    size_t len = 0;
    errno_t err = _dupenv_s(&vsVersion, &len, "VisualStudioVersion");

    if (err == 0 && vsVersion != nullptr)
    {
        resultingPath = "../../mono/lib/4.5";
    }
    else
    {
        resultingPath = "mono/lib/4.5";
    }
    free(vsVersion);
    return resultingPath;
}

void* ScriptingEM::ToMonoParam(const auto& value)
{
    if constexpr (std::is_same_v<decltype(value), bool>)
        return &value;
    else if constexpr (std::is_arithmetic_v<decltype(value)>)
        return const_cast<void*>(static_cast<const void*>(&value));
}

void* ScriptingEM::ToMonoStringParam(const std::string& str)
{
    return mono_string_new(mono_data.monoDomain, str.c_str());
}

void* ScriptingEM::ToMonoGameObjectParam(GameObject* go, const std::string& script_name) {
    if (!go) return nullptr;

    // Buscar el componente Script por nombre
    auto scripts = go->GetComponents<Script>();
    for (auto& script : scripts) {
        if (script->GetName() == script_name) {
            return script->GetMonoObject();
        }
    }

    LOG(LogType::LOG_ERROR,
        "Script '%s' not found in GameObject <%s - id: %u>",
        script_name.c_str(),
        go->GetName(),
        go->GetID()
    );
    return nullptr;
}
