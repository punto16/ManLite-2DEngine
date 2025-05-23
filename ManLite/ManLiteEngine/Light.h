#ifndef __LIGHT_H__
#define __LIGHT_H__
#pragma once

#include "Component.h"
#include "Defs.h"

enum class LightType
{
    AREA_LIGHT          = 0,
    POINT_LIGHT         = 1,
    RAY_LIGHT           = 2
};

class Light : public Component
{
public:
    Light(std::weak_ptr<GameObject> container_go, std::string name = "Light", bool enable = true);
    Light(const Light& component_to_copy, std::shared_ptr<GameObject> container_go);
    ~Light();

    void Draw() override;

    //serialization
    nlohmann::json SaveComponent() override;
    void LoadComponent(const nlohmann::json& componentJSON) override;

    //getters // setters
    void SetColor(ML_Color c) { color = c; }
    ML_Color GetColor() const { return color; }

    LightType GetType() const { return light_type; }
    void SetType(LightType type) { light_type = type; }

    float GetIntensity() { return intensity; }
    void SetIntensity(float i) { intensity = i; }

    float GetRadius() { return radius; }
    void SetRadius(float r) { radius = r; }
    float GetEndRadius() { return endRadius; }
    void SetEndRadius(float r) { endRadius = r; }
    vec2f GetEndPosition() { return endPosition; }
    void SetEndPosition(vec2f p) { endPosition = p; }
    bool IsFinalPosStatic() { return static_end_pos; }
    void SetFinalPosStatic(bool b) { static_end_pos = b; }

    // Propiedades específicas para RayLight
    void SetRayProperties(const vec2f& endPos, float startRad, float endRad)
    {
        endPosition = endPos;
        radius = startRad;
        endRadius = endRad;
    }

private:
    LightType light_type = LightType::POINT_LIGHT;
    float intensity = 0.6f;
    float radius = 1.0f; // Para PointLight
    bool static_end_pos = true;
    vec2f endPosition; // Solo para RayLight
    float endRadius = 0.0f; // Solo para RayLight
    ML_Color color = { 255, 255, 255, 255 }; // Blanco por defecto
};

#endif // !__LIGHT_H__