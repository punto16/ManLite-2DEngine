#include "Collider2D.h"
#include "PhysicsEM.h"
#include "GameObject.h"
#include "Transform.h"

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
    m_linearDamping(0.0f)
{
    Init();
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
    m_linearDamping(component_to_copy.m_linearDamping)
{
    Transform* t = container_go->GetComponent<Transform>();

    b2BodyDef bodyDef;
    bodyDef.type = m_isDynamic ? b2_dynamicBody : b2_staticBody;
    bodyDef.position.Set(
        PIXEL_TO_METERS(t->GetWorldPosition().x),
        PIXEL_TO_METERS(t->GetWorldPosition().y)
    );
    bodyDef.linearDamping = m_linearDamping;
    bodyDef.fixedRotation = m_lockRotation;

    m_body = PhysicsEM::GetWorld()->CreateBody(&bodyDef);

    b2FixtureDef fixtureDef;
    b2Shape* shape = nullptr;

    if (m_shapeType == ShapeType::RECTANGLE) {
        b2PolygonShape* boxShape = new b2PolygonShape();
        boxShape->SetAsBox(
            PIXEL_TO_METERS(m_width / 2),
            PIXEL_TO_METERS(m_height / 2)
        );
        shape = boxShape;
    }
    else {
        b2CircleShape* circleShape = new b2CircleShape();
        circleShape->m_radius = PIXEL_TO_METERS(m_radius);
        shape = circleShape;
    }

    fixtureDef.shape = shape;
    fixtureDef.isSensor = m_isSensor;
    fixtureDef.density = m_isDynamic ? 1.0f : 0.0f;
    fixtureDef.friction = m_friction;

    b2Fixture* fixture = m_body->CreateFixture(&fixtureDef);
    fixture->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);

    delete shape;

    if (m_isDynamic && component_to_copy.m_body) {
        m_body->SetLinearVelocity(component_to_copy.m_body->GetLinearVelocity());
        m_body->SetAngularVelocity(component_to_copy.m_body->GetAngularVelocity());
    }
}
void Collider2D::Init()
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
    m_body = PhysicsEM::GetWorld()->CreateBody(&bodyDef);

    b2FixtureDef fixtureDef;
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

    fixtureDef.shape = shape;
    fixtureDef.isSensor = m_isSensor;
    fixtureDef.density = m_isDynamic ? 1.0f : 0.0f;
    fixtureDef.friction = m_friction;

    b2Fixture* fixture = m_body->CreateFixture(&fixtureDef);
    fixture->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);

    delete shape;
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
    //empty for the moment
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
    Init();
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
