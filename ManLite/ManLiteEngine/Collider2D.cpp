#include "Collider2D.h"
#include "PhysicsEM.h"
#include "GameObject.h"
#include "Transform.h"
#include "EngineCore.h"
#include "RendererEM.h"

Collider2D::Collider2D(std::weak_ptr<GameObject> container_go,
    bool isDynamic,
    ShapeType shapeType,
    float width,
    float height,
    float radius)
    : Component(container_go, ComponentType::Collider2D, "Collider2D", true),
    m_isDynamic(isDynamic),
    m_shapeType(shapeType),
    m_width(width),
    m_height(height),
    m_radius(radius),
    m_friction(0.3f),
    m_linearDamping(0.0f),
    m_mass(0.0f),
    m_useGravity(true)
{
    b2BodyDef bodyDef;
    Transform* t = container_go.lock()->GetComponent<Transform>();
    bodyDef.type = m_isDynamic ? b2_dynamicBody : b2_staticBody;
    bodyDef.position.Set(
        PIXEL_TO_METERS(t->GetWorldPosition().x),
        PIXEL_TO_METERS(t->GetWorldPosition().y)
    );
    bodyDef.linearDamping = m_linearDamping;
    bodyDef.fixedRotation = m_lockRotation;
    bodyDef.gravityScale = m_useGravity ? 1.0f : 0.0f;

    m_body = PhysicsEM::GetWorld()->CreateBody(&bodyDef);

    RecreateFixture();
}

Collider2D::Collider2D(const Collider2D& component_to_copy, std::shared_ptr<GameObject> container_go)
    : Component(component_to_copy, container_go),
    m_shapeType(component_to_copy.m_shapeType),
    m_isDynamic(component_to_copy.m_isDynamic),
    m_isSensor(component_to_copy.m_isSensor),
    m_width(component_to_copy.m_width),
    m_height(component_to_copy.m_height),
    m_radius(component_to_copy.m_radius),
    m_color(component_to_copy.m_color),
    m_lockRotation(component_to_copy.m_lockRotation),
    m_friction(component_to_copy.m_friction),
    m_linearDamping(component_to_copy.m_linearDamping),
    m_mass(component_to_copy.m_mass),
    m_useGravity(component_to_copy.m_useGravity)
{
    b2BodyDef bodyDef;
    Transform* t = container_go->GetComponent<Transform>();
    bodyDef.type = m_isDynamic ? b2_dynamicBody : b2_staticBody;
    bodyDef.position.Set(
        PIXEL_TO_METERS(t->GetWorldPosition().x),
        PIXEL_TO_METERS(t->GetWorldPosition().y)
    );
    bodyDef.linearDamping = m_linearDamping;
    bodyDef.fixedRotation = m_lockRotation;
    bodyDef.gravityScale = m_useGravity ? 1.0f : 0.0f;

    m_body = PhysicsEM::GetWorld()->CreateBody(&bodyDef);

    RecreateFixture();

    if (m_isDynamic && component_to_copy.m_body)
    {
        m_body->SetLinearVelocity(component_to_copy.m_body->GetLinearVelocity());
        m_body->SetAngularVelocity(component_to_copy.m_body->GetAngularVelocity());
    }
}

Collider2D::~Collider2D()
{
    if (m_body)
    {
        PhysicsEM::GetWorld()->DestroyBody(m_body);
    }
}

bool Collider2D::Update(float dt)
{
    if (!m_body)
    {
        LOG(LogType::LOG_ERROR, "Collider2D: Update error, m_body is nullptr");
        return true;
    }
    b2Vec2 position = m_body->GetPosition();
    Transform* t = container_go.lock()->GetComponent<Transform>();
    t->SetWorldPosition({
    METERS_TO_PIXELS(position.x),
    METERS_TO_PIXELS(position.y)
        });

    if (!m_lockRotation)
        t->SetWorldAngle(m_body->GetAngle() * RADTODEG);


    return true;
}

void Collider2D::Draw()
{
    if (!m_body)
    {
        LOG(LogType::LOG_ERROR, "Collider2D: Draw error, m_body is nullptr");
        return;
    }

    Transform* t = container_go.lock()->GetComponent<Transform>();
    if (!t) return;

    // Obtener transformación base del GameObject
    mat3f modelMat = t->GetWorldMatrix();
    ML_Color color = m_color;
    color.a = 150; // 60% de opacidad

    if (m_shapeType == ShapeType::RECTANGLE) {
        // Crear matriz de escala para el tamaño del collider
        vec2f scale(m_width, m_height);
        mat3f colliderMat = mat3f::CreateTransformMatrix(
            vec2f(0, 0),
            0.0f,
            scale
        );

        // Combinar con la transformación del GameObject
        mat3f finalMat = modelMat * colliderMat;

        engine->renderer_em->SubmitDebugCollider(finalMat, color, false);
    }
    else {
        // Para círculos, escalar por el diámetro (radio * 2)
        float diameter = m_radius * 2.0f;
        mat3f colliderMat = mat3f::CreateTransformMatrix(
            vec2f(0, 0),
            0.0f,
            vec2f(diameter, diameter)
        );

        mat3f finalMat = modelMat * colliderMat;
        engine->renderer_em->SubmitDebugCollider(finalMat, color, true);
    }
}

void Collider2D::SetShapeType(ShapeType newType)
{
    if (m_shapeType != newType)
    {
        m_shapeType = newType;
        RecreateFixture();
    }
}

void Collider2D::SetSize(float width, float height)
{
    if (width <= 0 || height <= 0) return;
    if (m_shapeType == ShapeType::RECTANGLE &&
        (m_width != width || m_height != height))
    {
        m_width = width;
        m_height = height;
        RecreateFixture();
    }
}

void Collider2D::SetRadius(float radius)
{
    if (radius <= 0) return;
    if (m_shapeType == ShapeType::CIRCLE && m_radius != radius)
    {
        m_radius = radius;
        RecreateFixture();
    }
}

void Collider2D::SetDynamic(bool isDynamic)
{
    if (m_isDynamic != isDynamic)
    {
        m_isDynamic = isDynamic;

        if (m_body)
        {
            m_body->SetType(m_isDynamic ? b2_dynamicBody : b2_staticBody);

            RecreateFixture();

            if (!m_isDynamic)
            {
                m_body->SetLinearVelocity(b2Vec2_zero);
                m_body->SetAngularVelocity(0.0f);
            }
        }
    }
}

void Collider2D::SetSensor(bool sensor)
{
    if (m_isSensor == sensor) return;
    m_isSensor = sensor;

    if (m_body && m_body->GetFixtureList()) {
        b2Fixture* fixture = m_body->GetFixtureList();
        fixture->SetSensor(sensor);

        ContactListener* listener = PhysicsEM::GetContactListener();

        if (listener) {
            b2Contact* contact = PhysicsEM::GetWorld()->GetContactList();
            while (contact) {
                b2Fixture* fixA = contact->GetFixtureA();
                b2Fixture* fixB = contact->GetFixtureB();

                if ((fixA == fixture || fixB == fixture) && contact->IsTouching()) {
                    listener->EndContact(contact);
                    if (sensor)
                    {
                        listener->BeginContact(contact);
                    }
                }
                contact = contact->GetNext();
            }
        }
    }
}

void Collider2D::ApplyForce(float x, float y)
{
    m_body->ApplyForceToCenter(b2Vec2(x, y), true);
}

void Collider2D::SetVelocity(float x, float y)
{
    m_body->SetLinearVelocity(b2Vec2(x, y));
}

nlohmann::json Collider2D::SaveComponent()
{
    nlohmann::json componentJSON;
    //component generic
    componentJSON["ContainerGOID"] = this->container_go.lock()->GetID();
    componentJSON["ComponentID"] = component_id;
    componentJSON["ComponentName"] = name;
    componentJSON["ComponentType"] = (int)type;
    componentJSON["Enabled"] = enabled;

    //component spcecific
    componentJSON["ShapeType"] = (m_shapeType == ShapeType::RECTANGLE) ? "RECTANGLE" : "CIRCLE";
    componentJSON["IsDynamic"] = m_isDynamic;
    componentJSON["IsSensor"] = m_isSensor;
    componentJSON["LockRotation"] = m_lockRotation;
    componentJSON["Friction"] = m_friction;
    componentJSON["LinearDamping"] = m_linearDamping;
    componentJSON["Width"] = m_width;
    componentJSON["Height"] = m_height;
    componentJSON["Radius"] = m_radius;
    componentJSON["Mass"] = m_mass;
    componentJSON["UseGravity"] = m_useGravity;

    componentJSON["Color"]["R"] = m_color.r;
    componentJSON["Color"]["G"] = m_color.g;
    componentJSON["Color"]["B"] = m_color.b;
    componentJSON["Color"]["A"] = m_color.a;


    return componentJSON;
}

void Collider2D::LoadComponent(const nlohmann::json& componentJSON)
{
    if (componentJSON.contains("ComponentID")) component_id = componentJSON["ComponentID"];
    if (componentJSON.contains("ComponentName")) name = componentJSON["ComponentName"];
    if (componentJSON.contains("ComponentType")) type = (ComponentType)componentJSON["ComponentType"];
    if (componentJSON.contains("Enabled")) enabled = componentJSON["Enabled"];

    if (componentJSON.contains("ShapeType")) {
        std::string shapeStr = componentJSON["ShapeType"];
        m_shapeType = (shapeStr == "RECTANGLE") ? ShapeType::RECTANGLE : ShapeType::CIRCLE;
    }

    if (componentJSON.contains("IsDynamic")) m_isDynamic = componentJSON["IsDynamic"];
    if (componentJSON.contains("IsSensor")) m_isSensor = componentJSON["IsSensor"];
    if (componentJSON.contains("LockRotation")) m_lockRotation = componentJSON["LockRotation"];
    if (componentJSON.contains("Friction")) m_friction = componentJSON["Friction"];
    if (componentJSON.contains("LinearDamping")) m_linearDamping = componentJSON["LinearDamping"];
    if (componentJSON.contains("Width")) m_width = componentJSON["Width"];
    if (componentJSON.contains("Height")) m_height = componentJSON["Height"];
    if (componentJSON.contains("Radius")) m_radius = componentJSON["Radius"];
    if (componentJSON.contains("Mass")) m_mass = componentJSON["Mass"];
    if (componentJSON.contains("UseGravity")) m_useGravity = componentJSON["UseGravity"];

    if (componentJSON.contains("Color")) {
        m_color.r = componentJSON["Color"]["R"];
        m_color.g = componentJSON["Color"]["G"];
        m_color.b = componentJSON["Color"]["B"];
        m_color.a = componentJSON["Color"]["A"];
    }

    if (m_body)
    {
        PhysicsEM::GetWorld()->DestroyBody(m_body);
        m_body = nullptr;
    }
    //
    b2BodyDef bodyDef;
    Transform* t = container_go.lock()->GetComponent<Transform>();
    bodyDef.type = m_isDynamic ? b2_dynamicBody : b2_staticBody;
    bodyDef.position.Set(
        PIXEL_TO_METERS(t->GetWorldPosition().x),
        PIXEL_TO_METERS(t->GetWorldPosition().y)
    );
    bodyDef.linearDamping = m_linearDamping;
    bodyDef.fixedRotation = m_lockRotation;
    bodyDef.gravityScale = m_useGravity ? 1.0f : 0.0f;
    m_body = PhysicsEM::GetWorld()->CreateBody(&bodyDef);
    //
    RecreateFixture();
    SetSensor(m_isSensor);
    SetLockRotation(m_lockRotation);
}

void Collider2D::SetLockRotation(bool lockRotation)
{
    m_lockRotation = lockRotation;
    if (m_body)
    {
        m_body->SetFixedRotation(lockRotation);
    }
}

void Collider2D::SetFriction(float friction)
{
    m_friction = friction;
    if (m_body && m_body->GetFixtureList()) {
        m_body->GetFixtureList()->SetFriction(friction);
    }
}

void Collider2D::SetLinearDamping(float damping)
{
    m_linearDamping = damping;
    if (m_body) {
        m_body->SetLinearDamping(damping);
    }
}

void Collider2D::SetMass(float mass)
{
    m_mass = mass;
    if (m_body && m_isDynamic) {
        float area = 0.0f;
        if (m_shapeType == ShapeType::RECTANGLE) {
            area = PIXEL_TO_METERS(m_width) * PIXEL_TO_METERS(m_height);
        }
        else {
            float radius = PIXEL_TO_METERS(m_radius);
            area = b2_pi * radius * radius;
        }

        float newDensity = area > 0.0f ? m_mass / area : 0.0f;
        m_body->GetFixtureList()->SetDensity(newDensity);
        m_body->ResetMassData();
    }
}

void Collider2D::SetUseGravity(bool useGravity)
{
    m_useGravity = useGravity;
    if (m_body) {
        m_body->SetGravityScale(m_useGravity ? 1.0f : 0.0f);
    }
}

void Collider2D::SetEnabled(bool enable)
{
    if (enabled == enable) return;

    this->enabled = enable;
    UpdateBodyActivation();
}

void Collider2D::UpdateBodyActivation()
{
    if (!m_body) return;

    if (enabled)
    {
        m_body->SetEnabled(true);
        m_body->SetAwake(true);
    }
    else
    {
        m_body->SetEnabled(false);
        m_body->SetLinearVelocity(b2Vec2_zero);
        m_body->SetAngularVelocity(0.0f);
    }
}

void Collider2D::RecreateFixture()
{
    if (!m_body) return;

    while (m_body->GetFixtureList()) m_body->DestroyFixture(m_body->GetFixtureList());

    b2Shape* shape = nullptr;
    if (m_shapeType == ShapeType::RECTANGLE)
    {
        b2PolygonShape* boxShape = new b2PolygonShape();
        boxShape->SetAsBox(
            PIXEL_TO_METERS(m_width / 2),
            PIXEL_TO_METERS(m_height / 2)
        );
        shape = boxShape;
    }
    else
    {
        b2CircleShape* circleShape = new b2CircleShape();
        circleShape->m_radius = PIXEL_TO_METERS(m_radius);
        shape = circleShape;
    }

    b2FixtureDef fixtureDef;
    fixtureDef.shape = shape;
    fixtureDef.isSensor = m_isSensor;
    fixtureDef.friction = m_friction;

    float density = m_isDynamic ? 1.0f : 0.0f;
    if (m_mass > 0.0f && m_isDynamic)
    {
        float area = (m_shapeType == ShapeType::RECTANGLE) ?
            PIXEL_TO_METERS(m_width) * PIXEL_TO_METERS(m_height) :
            b2_pi * PIXEL_TO_METERS(m_radius) * PIXEL_TO_METERS(m_radius);

        density = (area > 0.0f) ? m_mass / area : 0.0f;
    }
    fixtureDef.density = density;

    b2Fixture* fixture = m_body->CreateFixture(&fixtureDef);
    fixture->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);

    if (m_isDynamic) m_body->ResetMassData();

    delete shape;
}