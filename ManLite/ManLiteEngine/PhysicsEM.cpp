#include "PhysicsEM.h"
#include "GameObject.h"
#include "Collider2D.h"
#include "box2d/box2d.h"

ContactListener* PhysicsEM::m_contactListener = nullptr;
b2World* PhysicsEM::m_world = nullptr;
int32 PhysicsEM::m_velocityIterations = 6;
int32 PhysicsEM::m_positionIterations = 2;

PhysicsEM::PhysicsEM(EngineCore* parent) : EngineModule(parent)
{
}

PhysicsEM::~PhysicsEM()
{
}

bool PhysicsEM::Awake()
{
	bool ret = true;

	return ret;
}

bool PhysicsEM::Start()
{
	bool ret = true;

	Init();

	return ret;
}

bool PhysicsEM::Update(float dt)
{
	bool ret = true;
	if (m_world)
	{
		m_world->Step(dt, m_velocityIterations, m_positionIterations);
	}
	return ret;
}

bool PhysicsEM::CleanUp()
{
	bool ret = true;
	
	Shutdown();

	return ret;
}

void PhysicsEM::Init(float gravityX, float gravityY)
{
	if (!m_world)
	{
		b2Vec2 gravity(gravityX, gravityY);
		m_world = new b2World(gravity);
		m_contactListener = new ContactListener();
		m_world->SetContactListener(m_contactListener);
	}
}

void PhysicsEM::Shutdown()
{
	delete m_contactListener;
	m_contactListener = nullptr;
	delete m_world;
	m_world = nullptr;
}

void ContactListener::HandleContact(b2Contact* contact, bool begin)
{
    b2Fixture* fixA = contact->GetFixtureA();
    b2Fixture* fixB = contact->GetFixtureB();

    auto* physA = reinterpret_cast<Collider2D*>(fixA->GetUserData().pointer);
    auto* physB = reinterpret_cast<Collider2D*>(fixB->GetUserData().pointer);

    if (physA && physB) {
        GameObject* goA = physA->GetContainerGO().get();
        GameObject* goB = physB->GetContainerGO().get();

        bool sensorContact = fixA->IsSensor() || fixB->IsSensor();

        if (begin) {
            if (sensorContact) {
                physA->OnTriggerSensor(goB);
                physB->OnTriggerSensor(goA);
            }
            else {
                physA->OnTriggerCollision(goB);
                physB->OnTriggerCollision(goA);
            }
        }
        else {
            if (sensorContact) {
                physA->OnExitSensor(goB);
                physB->OnExitSensor(goA);
            }
            else {
                physA->OnExitCollision(goB);
                physB->OnExitCollision(goA);
            }
        }
    }
}
