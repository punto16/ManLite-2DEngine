#include "Script.h"

#include "EngineCore.h"
#include "GameObject.h"
#include "ScriptingEM.h"
#include "MonoRegisterer.h"

Script::Script(std::weak_ptr<GameObject> container_go, std::string name, bool enable)
    : Component(container_go, ComponentType::Script, name, true)
{
    mono_object = engine->scripting_em->InstantiateClass(name, container_go.lock().get());
    RetrieveScriptFields();
}

Script::Script(const Script& component_to_copy, std::shared_ptr<GameObject> container_go)
    : Component(component_to_copy, container_go)
{
    mono_object = engine->scripting_em->InstantiateClass(name, container_go.get());
    this->SetEnabled(component_to_copy.enabled);
    RetrieveScriptFields();
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
    engine->scripting_em->CallScriptFunction(container_go.lock().get(), mono_object, "Start");
    return ret;
}

bool Script::Update(float dt)
{
    bool ret = true;

    //call script update here
    engine->scripting_em->CallScriptFunction(container_go.lock().get(), mono_object, "Update");

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
        engine->scripting_em->CallScriptFunction(container_go.lock().get(), mono_object, "Start");
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

void Script::RetrieveScriptFields()
{
    scriptFields.clear();
    MonoClass* klass = mono_object_get_class(mono_object);
    void* iter = nullptr;
    MonoClassField* field = nullptr;

    while ((field = mono_class_get_fields(klass, &iter))) {
        uint32_t flags = mono_field_get_flags(field);
        if (!(flags & MONO_FIELD_ATTR_PUBLIC)) continue;

        const char* name = mono_field_get_name(field);
        MonoType* type = mono_field_get_type(field);
        ScriptFieldType sfType = GetScriptFieldType(type);

        if (sfType == ScriptFieldType::None) continue;

        ScriptField sf;
        sf.type = sfType;
        GetCurrentFieldValue(field, sf); // Obtener valor actual
        scriptFields[name] = sf;
    }
}

void Script::ApplyFieldValues()
{
    MonoClass* klass = mono_object_get_class(mono_object);
    for (auto& [name, field] : scriptFields) {
        MonoClassField* monoField = mono_class_get_field_from_name(klass, name.c_str());
        if (!monoField) continue;

        switch (field.type) {
        case ScriptFieldType::Float: {
            float val = std::get<float>(field.value);
            mono_field_set_value(mono_object, monoField, &val);
            break;
        }
        case ScriptFieldType::Int: {
            int val = std::get<int>(field.value);
            mono_field_set_value(mono_object, monoField, &val);
            break;
        }
        case ScriptFieldType::Bool: {
            bool val = std::get<bool>(field.value);
            mono_field_set_value(mono_object, monoField, &val);
            break;
        }
        case ScriptFieldType::String: {
            std::string strVal = std::get<std::string>(field.value);
            MonoString* monoStr = MonoRegisterer::ToMonoString(strVal);
            mono_field_set_value(mono_object, monoField, monoStr);
            break;
        }
        case ScriptFieldType::GameObjectPtr: {
            GameObject* goPtr = std::get<GameObject*>(field.value);

            // Convertir a IntPtr para C#
            void* boxedPtr = MonoRegisterer::BoxPointer(goPtr);
            mono_field_set_value(mono_object, monoField, boxedPtr);
            break;
        }
        default: break;
        }
    }
}

ScriptFieldType Script::GetScriptFieldType(MonoType* monoType)
{
    int typeEncoding = mono_type_get_type(monoType);
    switch (typeEncoding) {
    case MONO_TYPE_R4: return ScriptFieldType::Float;
    case MONO_TYPE_I4: return ScriptFieldType::Int;
    case MONO_TYPE_BOOLEAN: return ScriptFieldType::Bool;
    case MONO_TYPE_STRING: return ScriptFieldType::String;
    case MONO_TYPE_CLASS: {
        MonoClass* fieldClass = mono_type_get_class(monoType);

        MonoClass* iGameObjectClass = mono_class_from_name(
            engine->scripting_em->GetCoreAssemblyImage(),
            "ManLiteScripting",
            "IGameObject"
        );

        if (fieldClass == iGameObjectClass) {
            return ScriptFieldType::GameObjectPtr;
        }
        break;
    }
    default: return ScriptFieldType::None;
    }
}

void Script::GetCurrentFieldValue(MonoClassField* field, ScriptField& sf) {
    switch (sf.type) {
    case ScriptFieldType::Float:
    {
        float value;
        mono_field_get_value(mono_object, field, &value);
        sf.value = value;
        break;
    }
    case ScriptFieldType::Int:
    {
        int value;
        mono_field_get_value(mono_object, field, &value);
        sf.value = value;
        break;
    }
    case ScriptFieldType::Bool:
    {
        bool value;
        mono_field_get_value(mono_object, field, &value);
        sf.value = value;
        break;
    }
    case ScriptFieldType::String:
    {
        MonoString* monoStr;
        mono_field_get_value(mono_object, field, &monoStr);
        if (monoStr) sf.value = std::string(MonoRegisterer::ToCppString(monoStr));
        break;
    }
    case ScriptFieldType::GameObjectPtr: {
        void* ptr;
        mono_field_get_value(mono_object, field, &ptr);

        GameObject* goPtr = nullptr;
        if (ptr != nullptr) {
            goPtr = reinterpret_cast<GameObject*>(
                MonoRegisterer::UnboxPointer(ptr)
                );
        }
        sf.value = goPtr;
        break;
    }
    default: break;
    }
}