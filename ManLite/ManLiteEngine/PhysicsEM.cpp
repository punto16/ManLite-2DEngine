#include "PhysicsEM.h"

#include "GameObject.h"
#include "Collider2D.h"
#include "EngineCore.h"
#include "vector"

#include "box2d/box2d.h"

ContactListener* PhysicsEM::m_contactListener = nullptr;
b2World* PhysicsEM::m_world = nullptr;
int32 PhysicsEM::m_velocityIterations = 6;
int32 PhysicsEM::m_positionIterations = 2;
bool PhysicsEM::world_stepping = false;
std::vector<std::function<void()>> PhysicsEM::deferredActions;

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
        ProcessDeferredActions();
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
    else
    {
        delete m_contactListener;
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

void PhysicsEM::DeferAction(std::function<void()> action)
{
    deferredActions.push_back(action);
}

void PhysicsEM::ProcessDeferredActions()
{
    for (auto& action : deferredActions) {
        action();
    }
    deferredActions.clear();
}

void ContactListener::BeginContact(b2Contact* contact)
{
    HandleContact(contact, true);
}

void ContactListener::EndContact(b2Contact* contact)
{
    HandleContact(contact, false);
}

void ContactListener::HandleContact(b2Contact* contact, bool begin)
{
    if (engine->GetEngineState() != EngineState::PLAY) return;

    if (!contact || !contact->IsEnabled()) return;

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
                if (physA && goB.get())
                    physA->OnTriggerSensor(goB.get());
                if (physB && goA.get())
                    physB->OnTriggerSensor(goA.get());
            }
            else {
                if (physA && goB.get())
                    physA->OnTriggerCollision(goB.get());
                if (physB && goA.get())
                    physB->OnTriggerCollision(goA.get());
            }
        }
        else {
            if (sensorContact) {
                if (physA && goB.get())
                    physA->OnExitSensor(goB.get());
                if (physB && goA.get())
                    physB->OnExitSensor(goA.get());
            }
            else {
                if (physA && goB.get())
                    physA->OnExitCollision(goB.get());
                if (physB && goA.get())
                    physB->OnExitCollision(goA.get());
            }
        }
    }
}
