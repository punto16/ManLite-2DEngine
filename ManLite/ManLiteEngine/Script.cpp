#include "Script.h"

#include "EngineCore.h"
#include "GameObject.h"
#include "ScriptingEM.h"
#include "MonoRegisterer.h"
#include "SceneManagerEM.h"

Script::Script(std::weak_ptr<GameObject> container_go, std::string name, bool enable)
    : Component(container_go, ComponentType::Script, name, true)
{
    if (std::this_thread::get_id() != engine->main_thread_id)
        mono_object = engine->scripting_em->InstantiateClassAsync(name, this);
    else
    {
        mono_object = engine->scripting_em->InstantiateClass(name, this);
        if (mono_object)
        {
            did_init = true;
            FinishLoad();
            did_init = false;
        }
    }
}

Script::Script(const Script& component_to_copy, std::shared_ptr<GameObject> container_go)
    : Component(component_to_copy, container_go)
{
    for (auto& item : component_to_copy.scriptFields)
    {
        scriptFields.push_back(item);
    }
    if (std::this_thread::get_id() != engine->main_thread_id)
    {
        mono_object = engine->scripting_em->InstantiateClassAsync(name, this);
    }
    else
    {
        mono_object = engine->scripting_em->InstantiateClass(name, this);
        did_init = true;
        FinishLoad();
        did_init = false;
    }
    enabled = component_to_copy.enabled;
}

Script::~Script()
{
    //unregister and delete script
    if (mono_object) engine->scripting_em->ReleaseMonoObject(mono_object);
    scriptFields.clear();
}

bool Script::Init()
{
    bool ret = true;
    ApplyFieldValues();

    //call script start here
    engine->scripting_em->CallScriptFunction(this, mono_object, "Start");
    if (mono_object) did_init = true;
    return ret;
}

bool Script::Update(float dt)
{
    bool ret = true;

    //call script update here
    engine->scripting_em->CallScriptFunction(this, mono_object, "Update");

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
        //ApplyFieldValues();
        engine->scripting_em->CallScriptFunction(this, mono_object, "Start");
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
    nlohmann::json scriptFieldsJSON;
    for (const auto& [fieldName, field] : scriptFields) {
        nlohmann::json fieldJSON;
        fieldJSON["type"] = static_cast<int>(field.type);

        switch (field.type) {
        case ScriptFieldType::Float:
            fieldJSON["value"] = std::get<float>(field.value);
            break;
        case ScriptFieldType::Int:
            fieldJSON["value"] = std::get<int>(field.value);
            break;
        case ScriptFieldType::Bool:
            fieldJSON["value"] = std::get<bool>(field.value);
            break;
        case ScriptFieldType::String:
            fieldJSON["value"] = std::get<std::string>(field.value);
            break;
        case ScriptFieldType::GameObjectPtr: {
            uint go_id = std::get<uint>(field.value);
            fieldJSON["value"] = go_id;
            break;
        }
        default:
            break;
        }
        scriptFieldsJSON[fieldName] = fieldJSON;
    }

    componentJSON["ScriptFields"] = scriptFieldsJSON;

    return componentJSON;
}

void Script::LoadComponent(const nlohmann::json& componentJSON)
{
    if (componentJSON.contains("ComponentID")) component_id = componentJSON["ComponentID"];
    if (componentJSON.contains("ComponentName")) name = componentJSON["ComponentName"];
    if (componentJSON.contains("ComponentType")) type = (ComponentType)componentJSON["ComponentType"];
    if (componentJSON.contains("Enabled")) enabled = componentJSON["Enabled"];

    //register here script
    mono_object = engine->scripting_em->InstantiateClassAsync(name, this);
    
    RetrieveScriptFields();

    if (componentJSON.contains("ScriptFields")) {
        auto fieldsJSON = componentJSON["ScriptFields"];

        for (auto& [fieldName, fieldData] : fieldsJSON.items()) {
            ScriptField field;
            field.type = static_cast<ScriptFieldType>(fieldData["type"].get<int>());

            switch (field.type) {
            case ScriptFieldType::Float:
                field.value = fieldData["value"].get<float>();
                break;
            case ScriptFieldType::Int:
                field.value = fieldData["value"].get<int>();
                break;
            case ScriptFieldType::Bool:
                field.value = fieldData["value"].get<bool>();
                break;
            case ScriptFieldType::String:
                field.value = fieldData["value"].get<std::string>();
                break;
            case ScriptFieldType::GameObjectPtr: {
                uint32_t goID = fieldData["value"].get<uint32_t>();
                field.value = goID;
                break;
            }
            default:
                break;
            }

            scriptFields.emplace_back(fieldName, field);
        }
    }

    ApplyFieldValues();
}

void Script::RetrieveScriptFields()
{
    if (!mono_object) return;

    MonoClass* klass = mono_object_get_class(mono_object);
    void* iter = nullptr;
    MonoClassField* field = nullptr;

    while ((field = mono_class_get_fields(klass, &iter)))
    {
        uint32_t flags = mono_field_get_flags(field);
        if (!(flags & MONO_FIELD_ATTR_PUBLIC)) continue;

        const char* name = mono_field_get_name(field);
        MonoType* type = mono_field_get_type(field);
        ScriptFieldType sfType = GetScriptFieldType(type);

        if (sfType == ScriptFieldType::None) continue;

        auto it = std::find_if(scriptFields.begin(), scriptFields.end(),
            [&name](const auto& pair) { return pair.first == name; });

        if (it != scriptFields.end())
        {
            GetCurrentFieldValue(field, it->second);
        }
        else
        {
            ScriptField sf;
            sf.type = sfType;
            GetCurrentFieldValue(field, sf);

            sf.default_value = sf.value;
            sf.first_time = false;

            scriptFields.emplace_back(name, sf);
        }
    }
}

void Script::ApplyFieldValues()
{
    if (!mono_object) return;
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
        case ScriptFieldType::GameObjectPtr:
        {
            uint go_id = std::get<uint>(field.value);
            GameObject* go = engine->scene_manager_em->GetCurrentScene().FindGameObjectByID(go_id).get();
            if (!go) break;

            MonoClass* iGameObjectClass = mono_class_from_name(
                engine->scripting_em->GetCoreAssemblyImage(),
                "ManLiteScripting",
                "IGameObject"
            );

            MonoObject* monoIGameObject = mono_object_new(
                engine->scripting_em->GetAppDomain(),
                iGameObjectClass
            );
            mono_runtime_object_init(monoIGameObject);

            MonoClassField* ptrField = mono_class_get_field_from_name(
                iGameObjectClass,
                "_game_object_ptr"
            );

            void* goPtr = reinterpret_cast<void*>(go);
            mono_field_set_value(
                monoIGameObject,
                ptrField,
                &goPtr
            );

            mono_field_set_value(
                mono_object,
                monoField,
                monoIGameObject
            );
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
    case MONO_TYPE_CLASS: 
    {
        MonoClass* fieldClass = mono_type_get_class(monoType);
        MonoClass* iGameObjectClass = mono_class_from_name(
            engine->scripting_em->GetCoreAssemblyImage(),
            "ManLiteScripting",
            "IGameObject"
        );

        if (mono_class_is_assignable_from(fieldClass, iGameObjectClass)) {
            return ScriptFieldType::GameObjectPtr;
        }
        break;
    }
    default: return ScriptFieldType::None;
    }
}

void Script::GetCurrentFieldValue(MonoClassField* field, ScriptField& sf) {
    if (!mono_object) return;
    switch (sf.type)
    {
    case ScriptFieldType::Float:
    {
        float value = 0.0f;
        mono_field_get_value(mono_object, field, &value);
        sf.value = value;
        break;
    }
    case ScriptFieldType::Int:
    {
        int value = 0;
        mono_field_get_value(mono_object, field, &value);
        sf.value = value;
        break;
    }
    case ScriptFieldType::Bool:
    {
        bool value = false;
        mono_field_get_value(mono_object, field, &value);
        sf.value = value;
        break;
    }
    case ScriptFieldType::String:
    {
        MonoString* monoStr;
        mono_field_get_value(mono_object, field, &monoStr);
        if (monoStr) sf.value = std::string(MonoRegisterer::ToCppString(monoStr));
        else sf.value = "";
        break;
    }
    case ScriptFieldType::GameObjectPtr:
    {
        MonoObject* monoIGameObject = nullptr;
        mono_field_get_value(mono_object, field, &monoIGameObject);

        if (monoIGameObject)
        {
            MonoClass* iGameObjectClass = mono_class_from_name(
                engine->scripting_em->GetCoreAssemblyImage(),
                "ManLiteScripting",
                "IGameObject"
            );

            MonoClassField* ptrField = mono_class_get_field_from_name(
                iGameObjectClass,
                "game_object_ptr"
            );

            void* goPtr = nullptr;
            mono_field_get_value(monoIGameObject, ptrField, &goPtr);

            GameObject* go = reinterpret_cast<GameObject*>(goPtr);
            sf.value = (uint)go->GetID();
        }
        else {
            sf.value = (uint)0;
        }
        break;
    }
    default: break;
    }
}

void Script::SetValueDefault(std::string variable)
{
    for (auto& [name, field] : scriptFields)
    {
        if (name == variable) field.value = field.default_value;
    }
}

void Script::FinishLoad()
{
    std::unordered_map<std::string, ScriptField> savedFields;
    for (auto& [name, field] : scriptFields) {
        savedFields[name] = field;
    }

    scriptFields.clear();

    RetrieveScriptFields();

    for (auto& [currentName, currentField] : scriptFields) {
        if (savedFields.count(currentName) && savedFields[currentName].type == currentField.type) {
            currentField.value = savedFields[currentName].value;
        }
    }

    ApplyFieldValues();

    if (!did_init && enabled && engine->GetEngineState() == EngineState::PLAY) Init();
}

void Script::ReloadScript()
{
    if (mono_object) engine->scripting_em->ReleaseMonoObject(mono_object);
    mono_object = engine->scripting_em->InstantiateClass(name, this);
    FinishLoad();
}
