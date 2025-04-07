#include "ParticleSystem.h"

#include "GameObject.h"
#include "Emmiter.h"

ParticleSystem::ParticleSystem(std::weak_ptr<GameObject> container_go, std::string name, bool enable) :
	Component(container_go, ComponentType::ParticleSystem, name, enable)
{
    emmiters.push_back(std::make_shared<Emmiter>(container_go));
}

ParticleSystem::ParticleSystem(const ParticleSystem& component_to_copy, std::shared_ptr<GameObject> container_go) :
	Component(component_to_copy, container_go)
{
}

ParticleSystem::~ParticleSystem()
{
    emmiters.clear();
}

bool ParticleSystem::Init()
{
    bool ret = true;

    for (const auto& emmiter : emmiters)
        if (!emmiter->Init()) return false;

    return ret;
}

bool ParticleSystem::Update(float dt)
{
    bool ret = true;

    for (const auto& emmiter : emmiters)
        if (!emmiter->Update(dt)) return false;

    return ret;
}

void ParticleSystem::Draw()
{
    for (const auto& emmiter : emmiters)
        emmiter->Draw();
}

bool ParticleSystem::Pause()
{
    bool ret = true;

    for (const auto& emmiter : emmiters)
        if (!emmiter->Pause()) return false;

    return ret;
}

bool ParticleSystem::Unpause()
{
    bool ret = true;

    for (const auto& emmiter : emmiters)
        if (!emmiter->Unpause()) return false;

    return ret;
}

nlohmann::json ParticleSystem::SaveComponent()
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

void ParticleSystem::LoadComponent(const nlohmann::json& componentJSON)
{
    if (componentJSON.contains("ComponentID")) component_id = componentJSON["ComponentID"];
    if (componentJSON.contains("ComponentName")) name = componentJSON["ComponentName"];
    if (componentJSON.contains("ComponentType")) type = (ComponentType)componentJSON["ComponentType"];
    if (componentJSON.contains("Enabled")) enabled = componentJSON["Enabled"];


}
