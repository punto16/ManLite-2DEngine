#include "GameObject.h"

#include "Layer.h"

#include "Component.h"
#include "Transform.h"

#include <random>
#include <ctime>
#include <unordered_set>

GameObject::GameObject(std::weak_ptr<GameObject> parent, std::string name, bool enable) :
    gameobject_id(GenerateGameObjectID()),
    parent_gameobject(parent),
    gameobject_name(""),
    enabled(enable)
{
    this->gameobject_name = GenerateUniqueName(name, this);

    LOG(LogType::LOG_INFO, "GameObject <%s - id: %s> created", gameobject_name.c_str(), std::to_string(gameobject_id).c_str());
    //NEVER call AddComponent or similar in constructor
    //WE CANT DO shared_from_this in a constructor!!
}

GameObject::GameObject(std::weak_ptr<GameObject> go_to_copy) :
    gameobject_id(GenerateGameObjectID()),
    parent_gameobject(go_to_copy.lock()->GetParentGO()),
    gameobject_name(""),
    enabled(go_to_copy.lock()->IsEnabled())
{
    this->gameobject_name = GenerateUniqueName(go_to_copy.lock()->GetName(), this);
    CloneChildrenHierarchy(go_to_copy.lock());
    CloneComponents(go_to_copy.lock());

    LOG(LogType::LOG_INFO, "GameObject <%s - id: %s> created from <%s>", gameobject_name.c_str(), std::to_string(gameobject_id).c_str(), go_to_copy.lock()->GetName().c_str());
}

GameObject::~GameObject()
{
}

bool GameObject::Awake()
{
    bool ret = true;

    //all game objects must have transform component
    AddComponent<Transform>();

    return ret;
}

bool GameObject::Update(double dt)
{
    bool ret = true;
    
    //update components
    for (const auto& item : components_gameobject) item->Update(dt);

    //then, update children game objects
    for (const auto& item : children_gameobject) item->Update(dt);

    return ret;
}

void GameObject::Draw()
{
    //draw components
    for (const auto& item : components_gameobject) item->Draw();

    //layer system!! do not iterate children.draw()
}

void GameObject::Delete()
{
    components_gameobject.clear();

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

bool GameObject::Reparent(std::shared_ptr<GameObject> new_parent, bool skip_descendant_search)
{
    auto self = shared_from_this();
    bool cycle = false;
    if (new_parent.get() == self.get()) return false;
    if (!skip_descendant_search && IsDescendant(new_parent)) return false;
    if (new_parent->GetParentGO().lock() == self) cycle = true;

    auto old_parent = parent_gameobject.lock();
    if (old_parent)
        old_parent->RemoveChild(self);

    if (new_parent)
        new_parent->AddChild(self);

    parent_gameobject = new_parent;

    if (cycle) new_parent->Reparent(old_parent, true);

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

void GameObject::CloneComponents(const std::shared_ptr<GameObject>& original)
{
    for (const auto& item : original->GetComponents())
    {
        switch (item->GetType())
        {
        case ComponentType::Transform:
        {
            //AddCopiedComponent<Transform>((Transform*)item.get());
            break;
        }
        case ComponentType::Camera:
        {
            break;
        }
        case ComponentType::Sprite:
        {
            break;
        }
        case ComponentType::Script:
        {
            break;
        }
        case ComponentType::Collider2D:
        {
            break;
        }
        case ComponentType::Canvas:
        {
            break;
        }
        case ComponentType::AudioSource:
        {
            break;
        }
        case ComponentType::ParticleSystem:
        {
            break;
        }
        case ComponentType::Unkown:
        {
            break;
        }
        default:
        {
            break;
        }
        }
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

std::string GameObject::GenerateUniqueName(const std::string& baseName, const GameObject* go)
{
    if (go == nullptr) return baseName;
    auto parent = go->GetParentGO().lock();
    if (!parent || parent.get() == go) return baseName;

    std::unordered_set<std::string> existingNames;
    std::string newName = baseName;
    std::vector<std::shared_ptr<GameObject>> children;
    children = go->GetParentGO().lock()->GetChildren();

    for (const auto& child : children)
        existingNames.insert(child->GetName());

    if (existingNames.count(newName) > 0)
    {
        int count = 1;
        while (existingNames.count(newName) > 0)
        {
            size_t pos = newName.find_last_of('(');
            if (pos != std::string::npos && newName.back() == ')' && newName[pos] == '(' && pos > 0)
            {
                int num = std::stoi(newName.substr(pos + 1, newName.size() - pos - 2));
                newName = newName.substr(0, pos - 1) + " (" + std::to_string(++num) + ")";
            }
            else
            {
                newName = baseName + " (" + std::to_string(count++) + ")";
            }
        }
    }

    return newName;
}
