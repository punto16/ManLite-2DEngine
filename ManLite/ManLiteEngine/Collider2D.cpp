#include "Collider2D.h"

#include "PhysicsEM.h"
#include "GameObject.h"
#include "Transform.h"
#include "EngineCore.h"
#include "RendererEM.h"
#include "Script.h"
#include "ScriptingEM.h"
#include "SceneManagerEM.h"

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
    m_restitution(0.0f),
    m_useGravity(true)
{
    RecreateBody();

    WaitUntilSafe();
    m_body->SetEnabled(false);
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
    m_restitution(component_to_copy.m_restitution),
    m_useGravity(component_to_copy.m_useGravity)
{
    RecreateBody();

    if (m_isDynamic && component_to_copy.m_body)
    {
        WaitUntilSafe();
        m_body->SetLinearVelocity(component_to_copy.m_body->GetLinearVelocity());
        m_body->SetAngularVelocity(component_to_copy.m_body->GetAngularVelocity());
    }

    WaitUntilSafe();
    m_body->SetEnabled(false);
}

Collider2D::~Collider2D()
{
    if (m_body)
    {
        WaitUntilSafe();
        PhysicsEM::GetWorld()->DestroyBody(m_body);
        m_body = nullptr;
    }
}

bool Collider2D::Init()
{
    bool ret = true;

    if (!m_body) RecreateBody();
    auto go = container_go.lock();
    if (!go) return true;
    Transform* t = go->GetComponent<Transform>();
    if (!t) return true;


    vec2f worldPos = t->GetWorldPosition();
    b2Vec2 pos(
        (worldPos.x),
        (worldPos.y)
    );
    float angle = DEGTORAD * t->GetWorldAngle();

    m_body->SetTransform(pos, angle);

    m_body->SetLinearVelocity(b2Vec2_zero);
    m_body->SetAngularVelocity(0.0f);

    m_body->SetEnabled(true);

    return ret;
}

bool Collider2D::CleanUp()
{
    if (m_body)
    {
        WaitUntilSafe();
        PhysicsEM::GetWorld()->DestroyBody(m_body);
        m_body = nullptr;
    }
    return true;
}

bool Collider2D::Update(float dt)
{
    if (!m_body) RecreateBody();
    b2Vec2 position = m_body->GetPosition();
    Transform* t = container_go.lock()->GetComponent<Transform>();
    t->SetWorldPosition({
    (position.x),
    (position.y)
        });

    if (!m_lockRotation)
        t->SetWorldAngle(m_body->GetAngle() * RADTODEG);

    return true;
}

void Collider2D::Draw()
{
    if (!engine->GetEditorOrBuild()) return;
    if (!m_body) RecreateBody();

    Transform* t = container_go.lock()->GetComponent<Transform>();
    if (!t) return;
    vec2f o_scale = t->GetWorldScale();
    bool a_lock = t->IsAspectRatioLocked();

    t->SetAspectRatioLock(false);
    t->SetWorldScale({1.0f, 1.0f});

    mat3f modelMat = t->GetWorldMatrix();

    t->SetWorldScale(o_scale);
    t->SetAspectRatioLock(a_lock);

    ML_Color color = m_color;
    color.a = 150;

    if (m_shapeType == ShapeType::RECTANGLE) {
        vec2f scale(m_width, m_height);
        mat3f colliderMat = mat3f::CreateTransformMatrix(
            vec2f(0, 0),
            0.0f,
            scale
        );

        mat3f finalMat = modelMat * colliderMat;

        engine->renderer_em->SubmitDebugCollider(finalMat, color, false, engine->scene_manager_em->GetCurrentScene().GetGOOrderInLayer(container_go.lock()));
    }
    else
    {
        mat3f colliderMat = mat3f::CreateTransformMatrix(
            vec2f(0, 0),
            0.0f,
            vec2f(1, 1)
        );

        mat3f finalMat = modelMat * colliderMat;
        engine->renderer_em->SubmitDebugCollider(finalMat, color, true, engine->scene_manager_em->GetCurrentScene().GetGOOrderInLayer(container_go.lock()), 0.0f, m_radius);
    }
}

bool Collider2D::Pause()
{
    bool ret = true;

    if (!m_body) RecreateBody();

    m_body->SetEnabled(false);

    return ret;
}

bool Collider2D::Unpause()
{
    bool ret = true;

    if (!m_body) RecreateBody();

    m_body->SetEnabled(true);

    return ret;
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

vec2f Collider2D::GetVelocity()
{
    if (!m_body) RecreateBody();
    return { m_body->GetLinearVelocity().x, m_body->GetLinearVelocity().y };
}

void Collider2D::OnTriggerCollision(GameObject* other)
{
    auto container = container_go.lock();
    if (!container) return;
    if (!other) return;

    auto scripts = container->GetComponents<Script>();
    for (auto& script : scripts)
    {
        MonoObject* scriptInstance = script->GetMonoObject();
        if (!scriptInstance) continue;

        void* params[] = {
            other
        };

        engine->scripting_em->CallScriptFunction(script, scriptInstance, "OnTriggerCollisionPtr", params, 1);
    }
}

void Collider2D::OnTriggerSensor(GameObject* other)
{
    auto container = container_go.lock();
    if (!container) return;
    if (!other) return;

    auto scripts = container->GetComponents<Script>();
    for (auto& script : scripts)
    {
        MonoObject* scriptInstance = script->GetMonoObject();
        if (!scriptInstance) continue;

        void* params[] = {
            other
        };

        engine->scripting_em->CallScriptFunction(script, scriptInstance, "OnTriggerSensorPtr", params, 1);
    }
}

void Collider2D::OnExitCollision(GameObject* other)
{
    auto container = container_go.lock();
    if (!container) return;
    if (!other) return;

    auto scripts = container->GetComponents<Script>();
    for (auto& script : scripts)
    {
        MonoObject* scriptInstance = script->GetMonoObject();
        if (!scriptInstance) continue;

        void* params[] = {
            other
        };

        engine->scripting_em->CallScriptFunction(script, scriptInstance, "OnExitCollisionPtr", params, 1);
    }
}

void Collider2D::OnExitSensor(GameObject* other)
{
    auto container = container_go.lock();
    if (!container) return;
    if (!other) return;

    auto scripts = container->GetComponents<Script>();
    for (auto& script : scripts)
    {
        MonoObject* scriptInstance = script->GetMonoObject();
        if (!scriptInstance) continue;

        void* params[] = {
            other
        };

        engine->scripting_em->CallScriptFunction(script, scriptInstance, "OnExitSensorPtr", params, 1);
    }
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
    componentJSON["Restitution"] = m_restitution;
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
    if (componentJSON.contains("Restitution")) m_restitution = componentJSON["Restitution"];
    if (componentJSON.contains("UseGravity")) m_useGravity = componentJSON["UseGravity"];

    if (componentJSON.contains("Color")) {
        m_color.r = componentJSON["Color"]["R"];
        m_color.g = componentJSON["Color"]["G"];
        m_color.b = componentJSON["Color"]["B"];
        m_color.a = componentJSON["Color"]["A"];
    }

    if (m_body)
    {
        WaitUntilSafe();
        if (PhysicsEM::GetWorld())
            PhysicsEM::GetWorld()->DestroyBody(m_body);
        m_body = nullptr;
    }
    //
    RecreateBody();
    WaitUntilSafe();
    m_body->SetEnabled(false);

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
            area = (m_width) * (m_height);
        }
        else {
            float radius = (m_radius);
            area = b2_pi * radius * radius;
        }

        float newDensity = area > 0.0f ? m_mass / area : 0.0f;
        m_body->GetFixtureList()->SetDensity(newDensity);
        m_body->ResetMassData();
    }
}

void Collider2D::SetRestitution(float restitution)
{
    m_restitution = restitution;
    if (m_body) {
        b2Fixture* fixture = m_body->GetFixtureList();
        while (fixture)
        {
            fixture->SetRestitution(m_restitution);
            fixture = fixture->GetNext();
        }
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

void Collider2D::RecreateBody()
{
    b2BodyDef bodyDef;
    Transform* t = container_go.lock()->GetComponent<Transform>();
    bodyDef.type = m_isDynamic ? b2_dynamicBody : b2_staticBody;
    bodyDef.position.Set(
        (t->GetWorldPosition().x),
        (t->GetWorldPosition().y)
    );
    bodyDef.linearDamping = m_linearDamping;
    bodyDef.fixedRotation = m_lockRotation;
    bodyDef.gravityScale = m_useGravity ? 1.0f : 0.0f;


    WaitUntilSafe();
    if (PhysicsEM::GetWorld())
        m_body = PhysicsEM::GetWorld()->CreateBody(&bodyDef);

    RecreateFixture();
}

void Collider2D::RecreateFixture()
{
    if (!m_body || !PhysicsEM::GetWorld()) return;

    while (m_body->GetFixtureList()) m_body->DestroyFixture(m_body->GetFixtureList());

    b2Shape* shape = nullptr;
    if (m_shapeType == ShapeType::RECTANGLE)
    {
        b2PolygonShape* boxShape = new b2PolygonShape();
        boxShape->SetAsBox(
            (m_width / 2),
            (m_height / 2)
        );
        shape = boxShape;
    }
    else
    {
        b2CircleShape* circleShape = new b2CircleShape();
        circleShape->m_radius = (m_radius);
        shape = circleShape;
    }

    b2FixtureDef fixtureDef;
    fixtureDef.shape = shape;
    fixtureDef.isSensor = m_isSensor;
    fixtureDef.friction = m_friction;
    fixtureDef.restitution = m_restitution;

    float density = m_isDynamic ? 1.0f : 0.0f;
    if (m_mass > 0.0f && m_isDynamic)
    {
        float area = (m_shapeType == ShapeType::RECTANGLE) ?
            (m_width) * (m_height) :
            b2_pi * (m_radius) * (m_radius);

        density = (area > 0.0f) ? m_mass / area : 0.0f;
    }
    fixtureDef.density = density;

    WaitUntilSafe();
    b2Fixture* fixture = m_body->CreateFixture(&fixtureDef);
    fixture->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);

    if (m_isDynamic) m_body->ResetMassData();

    delete shape;
}

void Collider2D::WaitUntilSafe()
{
    //if we process things while world is stepping it will crash
    if (std::this_thread::get_id() != engine->main_thread_id ||
        PhysicsEM::IsWorldStepping())
    {
        while (PhysicsEM::IsWorldStepping())
        {
            //wait untill world has finished stepping
        }
    }
}
