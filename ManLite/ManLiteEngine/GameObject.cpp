#include "GameObject.h"

#include <random>
#include <ctime>

GameObject::GameObject(std::weak_ptr<GameObject> parent, std::string name, bool enable) :
    gameobject_id(GenerateGameObjectID()),
    parent_gameobject(parent),
    gameobject_name(name)
{
}

GameObject::GameObject(std::weak_ptr<GameObject> go_to_copy) :
    gameobject_id(GenerateGameObjectID()),
    parent_gameobject(go_to_copy.lock().get()->GetParentGO()),
    gameobject_name(go_to_copy.lock().get()->GetName() + "_copy")
{
}

GameObject::~GameObject()
{
}

uint32_t GameObject::GenerateGameObjectID() {
    static std::mt19937 engine(std::random_device{}() ^ static_cast<uint32_t>(std::time(nullptr)));
    static std::uniform_int_distribution<uint32_t> distribution(0, std::numeric_limits<uint32_t>::max());
    return distribution(engine);
}