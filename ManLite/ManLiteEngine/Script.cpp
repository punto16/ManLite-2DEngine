#include "Script.h"

#include "EngineCore.h"
#include "GameObject.h"
#include "ScriptingEM.h"

Script::Script(std::weak_ptr<GameObject> container_go, std::string name, bool enable)
    : Component(container_go, ComponentType::Script, name, true)
{
    mono_object = engine->scripting_em->InstantiateClass(name, container_go.lock().get());
}

Script::Script(const Script& component_to_copy, std::shared_ptr<GameObject> container_go)
    : Component(component_to_copy, container_go)
{
    mono_object = engine->scripting_em->InstantiateClass(name, container_go.get());
    this->SetEnabled(component_to_copy.enabled);
}

Script::~Script()
{
    //unregister and delete script
    engine->scripting_em->ReleaseMonoObject(mono_object);
}

bool Script::Init()
{
    bool ret = true;
    
    //call script start here
    engine->scripting_em->CallScriptFunction(mono_object, "Start");

    return ret;
}

bool Script::Update(float dt)
{
    bool ret = true;

    //call script update here
    engine->scripting_em->CallScriptFunction(mono_object, "Update");

    return ret;
}

void Script::SetEnabled(bool enable)
{
    if (!enable)
    {
        this->enabled = false;
    }
    else if (enable)
    {
        this->enabled = true;
        //call script start
        engine->scripting_em->CallScriptFunction(mono_object, "Start");
    }
}

nlohmann::json Script::SaveComponent()
{
    nlohmann::json componentJSON;
    //component generic
    componentJSON["ContainerGOID"] = this->container_go.lock()->GetID();
    componentJSON["ComponentID"] = component_id;
    componentJSON["ComponentName"] = name;
    componentJSON["ComponentType"] = (int)type;
    componentJSON["Enabled"] = enabled;

    //component spcecific

    return componentJSON;
}

void Script::LoadComponent(const nlohmann::json& componentJSON)
{
    if (componentJSON.contains("ComponentID")) component_id = componentJSON["ComponentID"];
    if (componentJSON.contains("ComponentName")) name = componentJSON["ComponentName"];
    if (componentJSON.contains("ComponentType")) type = (ComponentType)componentJSON["ComponentType"];
    if (componentJSON.contains("Enabled")) enabled = componentJSON["Enabled"];

    //register here script
    mono_object = engine->scripting_em->InstantiateClass(name, container_go.lock().get());
    this->SetEnabled(enabled);
}
