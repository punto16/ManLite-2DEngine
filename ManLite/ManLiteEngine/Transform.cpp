#include "Transform.h"
#include "GameObject.h"
#include "mat3f.h"
#include "Defs.h"

#include <cmath>

Transform::Transform(std::weak_ptr<GameObject> container_go, std::string name, bool enable) :
    Component(container_go, ComponentType::Transform, name, enable),
    position(vec2f(0.0f, 0.0f)),
    angle_rotation(0.0f),
    scale(vec2f(1.0f, 1.0f))
{
}

Transform::Transform(const Transform& component_to_copy, std::shared_ptr<GameObject> container_go) :
    Component(component_to_copy, container_go),
    position(component_to_copy.position),
    angle_rotation(component_to_copy.angle_rotation),
    scale(component_to_copy.scale)
{
}

Transform::~Transform()
{
}

bool Transform::Update(float dt)
{
    return true;
}

json Transform::SaveComponent()
{
    json componentJSON;
    //component generic
    componentJSON["ContainerGOID"] = this->container_go.lock()->GetID();
    componentJSON["ComponentID"] = component_id;
    componentJSON["ComponentName"] = name;
    componentJSON["ComponentType"] = (int)type;
    componentJSON["Enabled"] = enabled;

    //component spcecific
    componentJSON["Position"] = { position.x, position.y };
    componentJSON["Rotation"] = angle_rotation;
    componentJSON["Scale"] = { scale.x, scale.y };
    componentJSON["LockAspect"] = lock_aspect_ratio;
    componentJSON["AspectRatio"] = aspect_ratio;

    return componentJSON;
}

void Transform::LoadComponent(const json& componentJSON)
{
    if (componentJSON.contains("ComponentID")) component_id = componentJSON["ComponentID"];
    if (componentJSON.contains("ComponentName")) name = componentJSON["ComponentName"];
    if (componentJSON.contains("ComponentType")) type = (ComponentType)componentJSON["ComponentType"];
    if (componentJSON.contains("Enabled")) enabled = componentJSON["Enabled"];

    if (componentJSON.contains("Position")) position = vec2f(componentJSON["Position"][0], componentJSON["Position"][1]);
    if (componentJSON.contains("Rotation")) angle_rotation = componentJSON["Rotation"];
    if (componentJSON.contains("Scale")) scale = vec2f(componentJSON["Scale"][0], componentJSON["Scale"][1]);
    if (componentJSON.contains("LockAspect")) lock_aspect_ratio = componentJSON["LockAspect"];
    if (componentJSON.contains("AspectRatio")) aspect_ratio = componentJSON["AspectRatio"];
}

void Transform::SetAngle(float angle)
{
    angle_rotation = fmod(angle, 360.0f);
    if (angle_rotation < 0) angle_rotation += 360.0f;
}

void Transform::SetScale(vec2f new_scale)
{
    if (lock_aspect_ratio)
    {
        if (new_scale.x != scale.x)
        {
            scale.x = new_scale.x;
            scale.y = new_scale.x / aspect_ratio;
        }
        else if (new_scale.y != scale.y)
        {
            scale.y = new_scale.y;
            scale.x = new_scale.y * aspect_ratio;
        }
    }
    else
    {
        scale = new_scale;
    }
}

mat3f Transform::GetLocalMatrix() const
{
    const float radians = angle_rotation * (PI / 180.0f);
    return mat3f::CreateTransformMatrix(position, radians, scale);
}

mat3f Transform::GetWorldMatrix() const
{
    mat3f world_matrix = GetLocalMatrix();

    if (auto go = container_go.lock()) {
        if (auto parent = go->GetParentGO().lock()) {
            if (auto parent_transform = parent->GetComponent<Transform>()) {
                world_matrix = parent_transform->GetWorldMatrix() * world_matrix;
            }
        }
    }

    return world_matrix;
}

vec2f Transform::GetWorldPosition()
{
    return GetWorldMatrix().GetTranslation();
}

void Transform::SetWorldPosition(vec2f world_pos)
{
    if (auto go = container_go.lock()) {
        if (auto parent = go->GetParentGO().lock()) {
            if (auto parent_transform = parent->GetComponent<Transform>()) {
                mat3f parent_inverse = parent_transform->GetWorldMatrix().Inverted();
                position = parent_inverse.TransformPoint(world_pos);
                return;
            }
        }
    }
    position = world_pos;
}

float Transform::GetWorldAngle()
{
    return GetWorldMatrix().GetRotation() * (180.0f / PI);
}

void Transform::SetWorldAngle(float world_angle)
{
    auto parent_transform = container_go.lock()->GetParentGO().lock()->GetComponent<Transform>();
    SetAngle(world_angle - parent_transform->GetWorldAngle());
}

vec2f Transform::GetWorldScale()
{
    return GetWorldMatrix().GetScale();
}

void Transform::SetWorldScale(vec2f world_scale)
{
    if (auto go = container_go.lock()) {
        if (auto parent = go->GetParentGO().lock()) {
            if (auto parent_transform = parent->GetComponent<Transform>()) {
                vec2f parent_scale = parent_transform->GetWorldScale();
                vec2f new_scale(
                    world_scale.x / parent_scale.x,
                    world_scale.y / parent_scale.y
                );
                SetScale(new_scale);
                return;
            }
        }
    }

    SetScale(world_scale);
}