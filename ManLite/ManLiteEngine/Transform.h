#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__
#pragma once

#include "Component.h"
#include "Defs.h"
#include "mat3f.h"

#include <string>
#include <memory>
#include <cmath>

class GameObject;

class Transform : public Component
{
public:
    Transform(std::weak_ptr<GameObject> container_go, std::string name = "Transform", bool enable = true);
    Transform(const Transform& component_to_copy, std::shared_ptr<GameObject> container_go);
    ~Transform();

    bool Update(float dt) override;

    //serialization
    nlohmann::json SaveComponent() override;
    void LoadComponent(const nlohmann::json& componentJSON) override;

    //getters // setters
    vec2f GetPosition() const { return position; }
    void SetPosition(vec2f pos, bool ignore_collider = false);
    float GetAngle() const { return angle_rotation; }
    void SetAngle(float angle, bool ignore_collider = false);
    vec2f GetScale() const { return scale; }
    void SetScale(vec2f new_scale);
    bool IsAspectRatioLocked() const { return lock_aspect_ratio; }
    void SetAspectRatioLock(bool lock)
    {
        if (lock && !lock_aspect_ratio)
            aspect_ratio = (scale.y != 0.0f) ? scale.x / scale.y : 1.0f;
        lock_aspect_ratio = lock;
    }

    vec2f GetWorldPosition();
    glm::vec3 GetWorldPositionGLM() {
        vec2f pos = GetWorldPosition();
        return glm::vec3(pos.x, pos.y, 0.0f);
    }
    void SetWorldPosition(vec2f pos, bool ignore_collider = false);
    float GetWorldAngle();
    void SetWorldAngle(float angle, bool ignore_collider = false);
    vec2f GetWorldScale();
    void SetWorldScale(vec2f scale);

    mat3f GetLocalMatrix() const;
    mat3f GetWorldMatrix() const;

private:
    vec2f position;
    float angle_rotation;
    vec2f scale;

    bool lock_aspect_ratio = true;
    float aspect_ratio = 1.0f;
};

#endif // __TRANSFORM_H__