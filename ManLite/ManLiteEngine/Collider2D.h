#pragma once
#include "Component.h"
#include "box2d/box2d.h"
#include "Defs.h"

enum class ShapeType { RECTANGLE, CIRCLE };

class Collider2D : public Component {
public:
    Collider2D(std::weak_ptr<GameObject> container_go,
        bool isDynamic = true,
        ShapeType shapeType = ShapeType::RECTANGLE,
        float width = 1.0f,
        float height = 1.0f,
        float radius = 0.5f);
    Collider2D(const Collider2D& component_to_copy, std::shared_ptr<GameObject> container_go);
    ~Collider2D();

    void Init();
    bool Update(float dt) override;
    void Draw() override;

    // Shape management
    ShapeType GetShapeType() const { return m_shapeType; }
    float GetWidth() const { return m_width; }
    float GetHeight() const { return m_height; }
    float GetRadius() const { return m_radius; }

    // Sensor management
    void SetSensor(bool sensor);
    bool IsSensor() const { return m_isSensor; }

    // Physics control
    void ApplyForce(float x, float y);
    void SetVelocity(float x, float y);
    b2Body* GetBody() const { return m_body; }

    // Collision events
    virtual void OnTriggerCollision(GameObject* other) {}
    virtual void OnTriggerSensor(GameObject* other) {}
    virtual void OnExitCollision(GameObject* other) {}
    virtual void OnExitSensor(GameObject* other) {}

    //serialization
    nlohmann::json SaveComponent() override;
    void LoadComponent(const nlohmann::json& componentJSON) override;

    // Debug
    void SetColor(const ML_Color& color) { m_color = color; }
    const ML_Color& GetColor() const { return m_color; }

    void SetLockRotation(bool lockRotation);
    bool GetLockRotation() const { return m_lockRotation; }

    void SetEnabled(bool enable);

private:
    void UpdateBodyActivation();

    b2Body* m_body = nullptr;
    ShapeType m_shapeType;
    bool m_isDynamic;
    bool m_isSensor = false;
    bool m_lockRotation = false;
    float m_width, m_height, m_radius;
    ML_Color m_color = { 0,255,0,255 };//green default
};