#include "ParticleSystem.h"

#include "GameObject.h"

ParticleSystem::ParticleSystem(std::weak_ptr<GameObject> container_go, std::string name, bool enable) :
	Component(container_go, ComponentType::ParticleSystem, name, enable)
{
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

    return ret;
}

bool ParticleSystem::Update(float dt)
{
    bool ret = true;

    return ret;
}

void ParticleSystem::Draw()
{
}

bool ParticleSystem::Pause()
{
    bool ret = true;

    return ret;
}

bool ParticleSystem::Unpause()
{
    bool ret = true;

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
