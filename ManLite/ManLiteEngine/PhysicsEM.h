#ifndef __PHYSICS_EM_H__
#define __PHYSICS_EM_H__
#pragma once

#include "EngineModule.h"
#include "Defs.h"
#include <string>
#include "box2d/box2d.h"

class GameObject;

class ContactListener : public b2ContactListener
{
public:
    void BeginContact(b2Contact* contact) override {
        HandleContact(contact, true);
    }

    void EndContact(b2Contact* contact) override {
        HandleContact(contact, false);
    }

private:
	void HandleContact(b2Contact* contact, bool begin);
};

class PhysicsEM : public EngineModule
{
public:
	PhysicsEM(EngineCore* parent);
	virtual ~PhysicsEM();

	bool Awake();
	bool Start();
    bool Update(double dt);
	bool CleanUp();

	//
	static void Init(float gravityX = 0.0f, float gravityY = -9.81f);
	static void Shutdown();

	//getters // setters
	static b2World* GetWorld() { return m_world; }
	static ContactListener* GetContactListener() { return m_contactListener; }

private:

	static b2World* m_world;
	static int32 m_velocityIterations;
	static int32 m_positionIterations;
    static ContactListener* m_contactListener;
};

#endif // !__PHYSICS_EM_H__