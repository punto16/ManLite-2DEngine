#include "GameObject.h"

#include "Layer.h"

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
    CloneChildrenHierarchy(go_to_copy.lock());
}

GameObject::~GameObject()
{
}

bool GameObject::Update(double dt)
{
    bool ret = true;
    
    //update components

    for (const auto& item : children_gameobject) item->Update(dt);

    return ret;
}

void GameObject::Draw()
{
    //draw components
    //layer system!! do not iterate children.draw()
}

void GameObject::Delete()
{
    auto children_copy = children_gameobject;
    children_gameobject.clear();

    for (auto& child : children_copy) child->Delete();

    if (auto parent_sp = parent_gameobject.lock()) {
        if (parent_sp->HasChild(gameobject_id)) {
            parent_sp->RemoveChild(shared_from_this());
        }
    }
    parent_gameobject.reset();
}

bool GameObject::Reparent(std::shared_ptr<GameObject> new_parent)
{
    if (new_parent.get() == this) return false;
    if (IsDescendant(new_parent)) return false;

    if (auto old_parent = parent_gameobject.lock()) {
        old_parent->RemoveChild(shared_from_this());
    }

    if (new_parent) {
        new_parent->AddChild(shared_from_this());
    }

    parent_gameobject = new_parent;

    return true;
}

bool GameObject::IsDescendant(const std::shared_ptr<GameObject>& potential_ancestor) const
{
    if (!potential_ancestor) return false;
    auto current = parent_gameobject.lock();
    while (current) {
        if (current == potential_ancestor) return true;
        current = current->parent_gameobject.lock();
    }
    return false;
}

void GameObject::CloneChildrenHierarchy(const std::shared_ptr<GameObject>& original)
{
    for (const auto& original_child : original->children_gameobject)
    {
        auto child_copy = std::make_shared<GameObject>(original_child);
        child_copy->parent_gameobject = shared_from_this();
        children_gameobject.push_back(child_copy);
    }
}

void GameObject::AddChild(std::shared_ptr<GameObject> child)
{
    child->parent_gameobject = weak_from_this();
    children_gameobject.push_back(child);
}

bool GameObject::RemoveChild(const std::shared_ptr<GameObject>& child)
{
    const size_t initial_size = children_gameobject.size();

    children_gameobject.erase(
        std::remove_if(children_gameobject.begin(), children_gameobject.end(),
            [&child](const std::shared_ptr<GameObject>& c)
            {
                return c.get() == child.get();
            }),
        children_gameobject.end()
    );

    return initial_size != children_gameobject.size();
}

bool GameObject::RemoveChild(uint32_t id)
{
    const size_t initial_size = children_gameobject.size();

    children_gameobject.erase(
        std::remove_if(children_gameobject.begin(), children_gameobject.end(),
            [id](const std::shared_ptr<GameObject>& c)
            {
                return c->gameobject_id == id;
            }),
        children_gameobject.end()
    );

    return initial_size != children_gameobject.size();
}

bool GameObject::HasChild(const std::shared_ptr<GameObject>& child) const
{
    return std::any_of(children_gameobject.begin(), children_gameobject.end(),
        [&child](const std::shared_ptr<GameObject>& c)
        {
            return c.get() == child.get();
        });
}

bool GameObject::HasChild(uint32_t id) const
{
    return std::any_of(children_gameobject.begin(), children_gameobject.end(),
        [id](const std::shared_ptr<GameObject>& c)
        {
            return c->gameobject_id == id;
        });
}

uint32_t GameObject::GenerateGameObjectID() {
    static std::mt19937 engine(std::random_device{}() ^ static_cast<uint32_t>(std::time(nullptr)));
    static std::uniform_int_distribution<uint32_t> distribution(0, std::numeric_limits<uint32_t>::max());
    return distribution(engine);
}