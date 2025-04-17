#include "ScriptingEM.h"

#include "MonoRegisterer.h"

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

    //esta linea crashea el programa :D
	mono_data.monoDomain = mono_jit_init("ManLiteScripting");
	if (!mono_data.monoDomain)
	{
		LOG(LogType::LOG_ERROR, "ScriptingEM: Awake error, monoDomain failed to initialize");
		return false;
	}


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

    if (mono_data.monoDomain)
    {
        mono_jit_cleanup(mono_data.monoDomain);
        mono_data.monoDomain = nullptr;
    }

	return ret;
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
