#include "ScriptingEM.h"

#include "MonoRegisterer.h"
#include "GameObject.h"
#include "Script.h"
#include "EngineCore.h"
#include "SceneManagerEM.h"

#include "Log.h"

#include "filesystem"
#include "fstream"
#include "Windows.h"

namespace fs = std::filesystem;

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

    //register internal calls
    MonoRegisterer::RegisterFunctions();

	return ret;
}

bool ScriptingEM::PreUpdate()
{
    bool ret = true;

    if (MonoRegisterer::set_scene && MonoRegisterer::new_scene.get() && !MonoRegisterer::is_loading)
    {
        bool engine_mode = engine->GetEditorOrBuild();
        engine->SetEditorOrBuild(false);

        engine->scene_manager_em->CleanUp();
        engine->scene_manager_em->GetCurrentScene() = *MonoRegisterer::new_scene;
        MonoRegisterer::new_scene.reset();
        MonoRegisterer::set_scene = false;
        if (engine->GetEngineState() == EngineState::PLAY)
        {
            ProcessInstantiateQueue();
            engine->scene_manager_em->StartSession();
            stop_process_instantiate_queue = false;
        }
        engine->scene_manager_em->FinishLoad();
        engine->SetEditorOrBuild(engine_mode);
    }

    if (!stop_process_instantiate_queue) ProcessInstantiateQueue();
    
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

    if (engine->GetEditorOrBuild())
    {
        script_ready = CompileUserScripts();
    }
    else
    {
        script_ready = true;
    }

    if (script_ready)
    {
        mono_data.userAssembly = mono_domain_assembly_open(mono_data.monoDomain, GetUserAssemblyPath().c_str());
        if (!mono_data.userAssembly)
        {
            LOG(LogType::LOG_ERROR, "ScriptingEM: Start error, user assembly failed to initialize");
            CleanUp();
            return false;
        }
        mono_data.userAssemblyImage = mono_assembly_get_image(mono_data.userAssembly);
    }

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

void ScriptingEM::CreateScriptFile(std::string script_name, std::string script_directory)
{
    std::string file_path = "Assets\\Scripts\\" + script_name + ".cs";
    if (!script_directory.empty())
        file_path = script_directory + script_name + ".cs";

    if (fs::exists(file_path))
    {
        LOG(LogType::LOG_ERROR, "File %s already exists", file_path.c_str());
        return;
    }

    std::string file_content = R"(
using ManLiteScripting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class CLASS_NAME_PLACEHOLDER : MonoBehaviour
{
    //create variables here

    //will be called once
    public override void Start()
    {
    }

    //it is called every frame
    public override void Update()
    {
    }

    //functions for colliders
    public override void OnTriggerCollision(IGameObject other)
    {
    }
    public override void OnExitCollision(IGameObject other)
    {
    }

    public override void OnTriggerSensor(IGameObject other)
    {
    }
    public override void OnExitSensor(IGameObject other)
    {
    }
})";

    const std::string placeholder = "CLASS_NAME_PLACEHOLDER";
    size_t pos = file_content.find(placeholder);
    if (pos != std::string::npos) {
        file_content.replace(pos, placeholder.length(), script_name);
    }

    std::ofstream out_file(file_path);
    if (out_file.is_open()) {
        out_file << file_content;
        out_file.close();
    }
    else
    {
        LOG(LogType::LOG_ERROR, "Error creating Script File at %s", file_path.c_str());
    }
}

MonoObject* ScriptingEM::InstantiateClass(const std::string& class_name, Script* container_script)
{
    MonoClass* klass = nullptr;

    if (mono_data.userAssemblyImage) {
        klass = mono_class_from_name(
            mono_data.userAssemblyImage,
            "",
            class_name.c_str()
        );
    }

    if (!klass && mono_data.coreAssemblyImage) {
        klass = mono_class_from_name(
            mono_data.coreAssemblyImage,
            "",
            class_name.c_str()
        );
    }

    if (!klass) {
        LOG(LogType::LOG_ERROR, "Class %s not found in any assembly", class_name.c_str());
        return nullptr;
    }

    MonoObject* instance = mono_object_new(mono_data.monoDomain, klass);
    if (!instance)
    {
        LOG(LogType::LOG_ERROR, "Failed to create instance of %s", class_name.c_str());
        return nullptr;
    }

    mono_data.currentGOPtr = container_script->GetContainerGO().get();
    mono_runtime_object_init(instance);
    mono_data.currentGOPtr = nullptr;

    uint32_t gc_handle = mono_gchandle_new(instance, false);
    mono_gc_handles[instance] = gc_handle;
    active_scripts.emplace_back(container_script);

    return instance;
}

MonoObject* ScriptingEM::InstantiateClassAsync(const std::string& class_name, Script* container_script)
{
    if (class_name.empty() || class_name == "Null Script") return nullptr;
    instantiate_queue.emplace_back(InstantiateQueueData{ class_name, container_script->GetContainerGO()->GetID()});
    return nullptr;
}

void ScriptingEM::CallScriptFunction(Script* container_script, MonoObject* mono_object, const std::string& function_name, void** params, int num_params)
{
    if (!mono_object) return;

    MonoClass* current_class = mono_object_get_class(mono_object);
    MonoMethod* method = nullptr;

    while (current_class != nullptr && method == nullptr) {
        method = mono_class_get_method_from_name(current_class, function_name.c_str(), num_params);
        if (!method) {
            current_class = mono_class_get_parent(current_class);
        }
    }

    if (!method) {
        LOG(LogType::LOG_ERROR, "Method %s not found in class hierarchy", function_name.c_str());
        return;
    }

    mono_data.currentGOPtr = container_script->GetContainerGO().get();
    MonoObject* exception = nullptr;
    mono_runtime_invoke(
        method,
        mono_object,
        params,
        &exception
    );
    mono_data.currentGOPtr = nullptr;

    if (exception) {
        MonoString* exc_str = mono_object_to_string(exception, nullptr);
        char* exc_msg = mono_string_to_utf8(exc_str);
        LOG(LogType::LOG_ERROR, "Script Exception: %s", exc_msg);
        mono_free(exc_msg);
    }
}

void ScriptingEM::ReleaseMonoObject(MonoObject* mono_object)
{
    if (!mono_object) return;

    for (auto& script : active_scripts)
    {
        if (script->GetMonoObject() == mono_object)
        {
            active_scripts.erase(std::find(active_scripts.begin(), active_scripts.end(), script));
            break;
        }
    }

    auto it = mono_gc_handles.find(mono_object);
    if (it != mono_gc_handles.end())
    {
        mono_gchandle_free(it->second);
        mono_gc_handles.erase(it);
        mono_object = nullptr;
    }
}

bool ScriptingEM::CompileUserScripts()
{
    std::vector<std::string> csFiles;
    try {
        for (const auto& entry : fs::recursive_directory_iterator("Assets")) {
            if (entry.is_regular_file() && entry.path().extension() == ".cs") {
                csFiles.push_back(entry.path().string());
            }
        }
    }
    catch (...) {
        LOG(LogType::LOG_ERROR, "Error searching .cs files in directory");
    }

    if (csFiles.empty()) return true;

    std::string outputDll = fs::absolute(GetUserAssemblyPath()).string();
    std::string referenceDll = fs::absolute(GetAssemblyPath()).string();
    std::string mcs_path = fs::absolute(GetMCSPath() + "\\mcs.bat").string();

    if (fs::exists(outputDll)) {
        fs::remove(outputDll);
    }

    std::string compileCommand;
    compileCommand += mcs_path + " -target:library -out:\"" + outputDll + "\" ";
    compileCommand += "-r:\"" + referenceDll + "\" ";
    compileCommand += "-r:Microsoft.CSharp.dll ";
    compileCommand += "-r:System.dll ";
    compileCommand += "-r:System.Core.dll ";
    compileCommand += "-r:System.Data.dll ";
    compileCommand += "-r:System.Data.DataSetExtensions.dll ";
    compileCommand += "-r:System.Net.Http.dll ";
    compileCommand += "-r:System.Xml.dll ";
    compileCommand += "-r:System.Xml.Linq.dll ";

    for (const auto& file : csFiles) {
        compileCommand += " \"" + file + "\"";
    }

    compileCommand += " > compilation_log.txt 2>&1";

    LOG(LogType::LOG_INFO, "Compiling scripts... Command:\n%s", compileCommand.c_str());

    //execute command without creating cmd window
    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    BOOL success = CreateProcessA(
        NULL,
        (LPSTR)compileCommand.c_str(),
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (!success) {
        LOG(LogType::LOG_ERROR, "Failed to start compiler process");
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    std::ifstream logFile("compilation_log.txt");
    if (logFile.is_open()) {
        std::string line;
        LOG(LogType::LOG_INFO, "[MCS] -----------------------------------------", line.c_str());
        while (std::getline(logFile, line)) {
            LOG(LogType::LOG_INFO, "[MCS] %s", line.c_str());
        }
        LOG(LogType::LOG_INFO, "[MCS] -----------------------------------------", line.c_str());
        logFile.close();
        if (fs::exists("compilation_log.txt")) {
            fs::remove("compilation_log.txt");
        }
    }

    if (exitCode != 0) {
        LOG(LogType::LOG_ERROR, "Script compilation failed! Error code: %u", exitCode);
        return false;
    }

    LOG(LogType::LOG_OK, "Scripts compiled successfully!");
    return true;
}

void ScriptingEM::RecompileScripts()
{
    // free all before reload
    for (auto& pair : mono_gc_handles) {
        mono_gchandle_free(pair.second);
    }
    mono_gc_handles.clear();

    mono_data.coreAssembly = nullptr;
    mono_data.coreAssemblyImage = nullptr;
    mono_data.userAssembly = nullptr;
    mono_data.userAssemblyImage = nullptr;
    if (mono_data.monoDomain)
    {
        mono_domain_set(mono_data.monoRootDomain, true);
        mono_domain_unload(mono_data.monoDomain);
        mono_data.monoDomain = nullptr;
    }

    // reload all
    char appDomainName[] = "MyAppDomain";
    mono_data.monoDomain = mono_domain_create_appdomain(appDomainName, nullptr);
    mono_domain_set(mono_data.monoDomain, true);

    MonoRegisterer::RegisterFunctions();

    mono_data.coreAssembly = mono_domain_assembly_open(mono_data.monoDomain, GetAssemblyPath().c_str());
    if (!mono_data.coreAssembly) {
        LOG(LogType::LOG_ERROR, "Failed to reload core assembly!");
        return;
    }
    mono_data.coreAssemblyImage = mono_assembly_get_image(mono_data.coreAssembly);

    if (engine->GetEditorOrBuild())
    {
        script_ready = CompileUserScripts();
    }
    else
    {
        script_ready = true;
    }

    mono_data.userAssembly = mono_domain_assembly_open(mono_data.monoDomain, GetUserAssemblyPath().c_str());
    if (!mono_data.userAssembly) {
        LOG(LogType::LOG_ERROR, "Failed to reload user assembly!");
        return;
    }
    mono_data.userAssemblyImage = mono_assembly_get_image(mono_data.userAssembly);

    for (Script* script : active_scripts) {
        script->ReloadScript();
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

std::string ScriptingEM::GetUserAssemblyPath()
{
    std::string resultingAssembly;

    char* vsVersion = nullptr;
    size_t len = 0;
    errno_t err = _dupenv_s(&vsVersion, &len, "VisualStudioVersion");

    if (err == 0 && vsVersion != nullptr)
    {
#ifdef _DEBUG

        resultingAssembly = "..\\x64\\Debug\\ManLiteUserScripts.dll";

#else

        resultingAssembly = "..\\x64\\Release\\ManLiteUserScripts.dll";

#endif
    }
    else
        resultingAssembly = "ManLiteUserScripts.dll";

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
        resultingPath = "..\\..\\mono\\lib\\mono\\4.5";
    }
    else
    {
        resultingPath = "mono\\lib\\mono\\4.5";
    }
    free(vsVersion);
    return resultingPath;
}

std::string ScriptingEM::GetMCSPath()
{
    std::string resultingPath;

    char* vsVersion = nullptr;
    size_t len = 0;
    errno_t err = _dupenv_s(&vsVersion, &len, "VisualStudioVersion");

    if (err == 0 && vsVersion != nullptr)
    {
        resultingPath = "..\\..\\mono\\bin";
    }
    else
    {
        resultingPath = "mono\\bin";
    }
    free(vsVersion);
    return resultingPath;
}

void ScriptingEM::ProcessInstantiateQueue()
{
    for (auto& data : instantiate_queue)
    {
        if (auto go = engine->scene_manager_em->GetCurrentScene().FindGameObjectByID(data.container_go_id))
        {
            auto scripts = go->GetComponents<Script>();
            for (auto& script : scripts)
            {
                if (script->GetName() != data.class_name) continue;

                MonoObject* mono_obj = InstantiateClass(data.class_name, script);
                if (!mono_obj) continue;
                script->SetMonoObject(mono_obj);
                script->FinishLoad();
                break;
            }
        }
    }
    instantiate_queue.clear();
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
