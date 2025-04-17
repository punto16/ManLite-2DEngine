#ifndef __SCRIPT_H__
#define __SCRIPT_H__
#pragma once

#include "Component.h"
#include "Defs.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"

class Script : public Component {
public:
    Script(std::weak_ptr<GameObject> container_go, std::string name = "Script", bool enable = false);
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

private:

    MonoObject* mono_object = nullptr;
};

#endif // !__SCRIPT_H__