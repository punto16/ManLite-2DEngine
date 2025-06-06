#include "PhysicsEM.h"

#include "GameObject.h"
#include "Collider2D.h"
#include "EngineCore.h"

#include "box2d/box2d.h"

ContactListener* PhysicsEM::m_contactListener = nullptr;
b2World* PhysicsEM::m_world = nullptr;
int32 PhysicsEM::m_velocityIterations = 6;
int32 PhysicsEM::m_positionIterations = 2;
bool PhysicsEM::world_stepping = false;

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

bool PhysicsEM::Update(double dt)
{
	bool ret = true;
	if (m_world && engine->GetEngineState() == EngineState::PLAY)
	{
        world_stepping = true;
		m_world->Step(dt, m_velocityIterations, m_positionIterations);
        world_stepping = false;
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
    if (engine->GetEngineState() != EngineState::PLAY) return;

    b2Fixture* fixA = contact->GetFixtureA();
    b2Fixture* fixB = contact->GetFixtureB();

    auto* physA = reinterpret_cast<Collider2D*>(fixA->GetUserData().pointer);
    auto* physB = reinterpret_cast<Collider2D*>(fixB->GetUserData().pointer);

    if (physA && physB) {
        std::shared_ptr<GameObject> goA = physA->GetContainerGO();
        std::shared_ptr<GameObject> goB = physB->GetContainerGO();

        if (goA == nullptr || goB == nullptr) return;

        bool sensorContact = fixA->IsSensor() || fixB->IsSensor();

        if (begin) {
            if (sensorContact) {
                physA->OnTriggerSensor(goB.get());
                physB->OnTriggerSensor(goA.get());
            }
            else {
                physA->OnTriggerCollision(goB.get());
                physB->OnTriggerCollision(goA.get());
            }
        }
        else {
            if (sensorContact) {
                physA->OnExitSensor(goB.get());
                physB->OnExitSensor(goA.get());
            }
            else {
                physA->OnExitCollision(goB.get());
                physB->OnExitCollision(goA.get());
            }
        }
    }
}
