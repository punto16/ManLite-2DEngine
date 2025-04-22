#ifndef __SCRIPT_H__
#define __SCRIPT_H__
#pragma once

#include "Component.h"
#include "Defs.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"

#include "string"
#include "variant"
#include "unordered_map"

enum class ScriptFieldType {
    None = 0,
    Float,
    Int,
    Bool,
    String,
    GameObjectPtr
};

struct ScriptField {
    ScriptFieldType type;
    std::variant<float, int, bool, std::string, GameObject*> value;
};

class Script : public Component {
public:
    Script(std::weak_ptr<GameObject> container_go, std::string name = "Null Script", bool enable = false);
    Script(const Script& component_to_copy, std::shared_ptr<GameObject> container_go);
    ~Script();

    bool Init() override;
    bool Update(float dt) override;

    void SetEnabled(bool enable);
    //script does NOT allow a name change
    void SetName(std::string name) {}

    //serialization
    nlohmann::json SaveComponent() override;
    void LoadComponent(const nlohmann::json& componentJSON) override;

    //getters // setters
    MonoObject* GetMonoObject() { return mono_object; }
    std::unordered_map<std::string, ScriptField>& GetScriptFields() { return scriptFields; }

    void RetrieveScriptFields();
    void ApplyFieldValues();
    ScriptFieldType GetScriptFieldType(MonoType* monoType);
    void GetCurrentFieldValue(MonoClassField* field, ScriptField& sf);



private:

    MonoObject* mono_object = nullptr;

    //variables
    std::unordered_map<std::string, ScriptField> scriptFields;
};

#endif // !__SCRIPT_H__