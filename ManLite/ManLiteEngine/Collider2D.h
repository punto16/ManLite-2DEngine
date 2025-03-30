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

    bool Update(float dt) override;
    void Draw() override;

    // Shape management
    ShapeType GetShapeType() const { return m_shapeType; }
    float GetWidth() const { return m_width; }
    float GetHeight() const { return m_height; }
    float GetRadius() const { return m_radius; }

    void SetShapeType(ShapeType newType);
    void SetSize(float width, float height);
    void SetRadius(float radius);
    bool IsDynamic() const { return m_isDynamic; }
    void SetDynamic(bool isDynamic);

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

    float GetFriction() const { return m_friction; }
    //friction when colliding other bodies (on ice, on grass, etc)
    //values from 0.0 (ice) to 1.0 (sand)
    void SetFriction(float friction);

    float GetLinearDamping() const { return m_linearDamping; }
    //friction that always exist (swiming in water, air resistance, etc)
    //values from 0.0 (space) to 5.0 (dense water)
    void SetLinearDamping(float damping);

    float GetMass() const { return m_mass; }
    void SetMass(float mass);

    bool GetUseGravity() const { return m_useGravity; }
    void SetUseGravity(bool useGravity);

    void SetEnabled(bool enable);

private:
    void UpdateBodyActivation();
    void RecreateFixture();

    b2Body* m_body = nullptr;
    ShapeType m_shapeType;
    bool m_isDynamic;
    bool m_isSensor = false;
    bool m_lockRotation = false;
    //friction that always exist (swiming in water, air resistance, etc)
    //values from 0.0 (space) to 5.0 (dense water)
    float m_linearDamping = 0.0f;
    //friction when colliding other bodies (on ice, on grass, etc)
    //values from 0.0 (ice) to 1.0 (sand)
    float m_friction = 0.3f;
    float m_width, m_height, m_radius;
    // Mass in kg (0 = auto-calculate)
    float m_mass = 0.0f;
    bool m_useGravity = true;
    ML_Color m_color = { 0,255,0,255 };//green default
};