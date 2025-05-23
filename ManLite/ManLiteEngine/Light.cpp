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
    Component(component_to_copy, container_go),
    light_type(component_to_copy.light_type),
    intensity(component_to_copy.intensity),
    radius(component_to_copy.radius),
    static_end_pos(component_to_copy.static_end_pos),
    endPosition(component_to_copy.endPosition),
    endRadius(component_to_copy.endRadius),
    color(component_to_copy.color)
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
            info.endPosition = static_end_pos ? this->endPosition : this->endPosition + t->GetWorldPosition();
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
    componentJSON["LightType"] = light_type;
    componentJSON["LightIntensity"] = intensity;
    componentJSON["LightRadius"] = radius;
    componentJSON["LightStaticEndPosition"] = static_end_pos;
    componentJSON["LightEndPosition"] = { endPosition.x, endPosition.y };
    componentJSON["LightEndRadius"] = endRadius;
    componentJSON["LightColor"] = { color.r, color.g, color.b, color.a };

    return componentJSON;
}

void Light::LoadComponent(const nlohmann::json& componentJSON)
{
    if (componentJSON.contains("ComponentID")) component_id = componentJSON["ComponentID"];
    if (componentJSON.contains("ComponentName")) name = componentJSON["ComponentName"];
    if (componentJSON.contains("ComponentType")) type = (ComponentType)componentJSON["ComponentType"];
    if (componentJSON.contains("Enabled")) enabled = componentJSON["Enabled"];

    if (componentJSON.contains("LightType")) light_type = componentJSON["LightType"];
    if (componentJSON.contains("LightIntensity")) intensity = componentJSON["LightIntensity"];
    if (componentJSON.contains("LightRadius")) radius = componentJSON["LightRadius"];
    if (componentJSON.contains("LightStaticEndPosition")) static_end_pos = componentJSON["LightStaticEndPosition"];
    if (componentJSON.contains("LightEndPosition")) endPosition = { componentJSON["LightEndPosition"][0], componentJSON["LightEndPosition"][1] };
    if (componentJSON.contains("LightEndRadius")) endRadius = componentJSON["LightEndRadius"];
    if (componentJSON.contains("LightColor")) color = ML_Color((int)componentJSON["LightColor"][0], (int)componentJSON["LightColor"][1], (int)componentJSON["LightColor"][2], (int)componentJSON["LightColor"][3]);
}
