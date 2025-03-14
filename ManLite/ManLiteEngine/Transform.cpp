#include "Transform.h"
#include "GameObject.h"
#include "mat3f.h"
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

void Transform::SetAngle(float angle)
{
    angle_rotation = fmod(angle, 360.0f);
    if (angle_rotation < 0) angle_rotation += 360.0f;
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
    if (auto go = container_go.lock()) {
        if (auto parent = go->GetParentGO().lock()) {
            if (auto parent_transform = parent->GetComponent<Transform>()) {
                float parent_angle = parent_transform->GetWorldAngle();
                angle_rotation = world_angle - parent_angle;
                angle_rotation = fmod(angle_rotation, 360.0f);
                if (angle_rotation < 0) angle_rotation += 360.0f;
                return;
            }
        }
    }
    SetAngle(world_angle);
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
                scale.x = world_scale.x / parent_scale.x;
                scale.y = world_scale.y / parent_scale.y;
                return;
            }
        }
    }
    scale = world_scale;
}