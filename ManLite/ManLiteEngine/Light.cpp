#include "Light.h"

#include "GameObject.h"
#include "Transform.h"
#include "RendererEM.h"
#include "EngineCore.h"

Light::Light(std::weak_ptr<GameObject> container_go, std::string name, bool enable) :
    Component(container_go, ComponentType::Light, name, enable)
{
}

Light::Light(const Light& component_to_copy, std::shared_ptr<GameObject> container_go) :
    Component(component_to_copy, container_go)
{
}

Light::~Light()
{
}

void Light::Draw()
{
    if (auto go = container_go.lock().get())
    {
        if (auto t = go->GetComponent<Transform>())
        {
            LightRenderData info;
            info.color = { (float)color.r / 255, (float)color.g / 255, (float)color.b / 255 };
            info.endPosition = this->endPosition;
            info.endRadius = this->endRadius;
            info.intensity = this->intensity;
            info.position = t->GetWorldPosition();
            info.radius = this->radius;
            info.startRadius = this->radius;
            info.type = (int)light_type;

            engine->renderer_em->SubmitLight(info);
        }
    }
}

nlohmann::json Light::SaveComponent()
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

void Light::LoadComponent(const nlohmann::json& componentJSON)
{
    if (componentJSON.contains("ComponentID")) component_id = componentJSON["ComponentID"];
    if (componentJSON.contains("ComponentName")) name = componentJSON["ComponentName"];
    if (componentJSON.contains("ComponentType")) type = (ComponentType)componentJSON["ComponentType"];
    if (componentJSON.contains("Enabled")) enabled = componentJSON["Enabled"];


}
