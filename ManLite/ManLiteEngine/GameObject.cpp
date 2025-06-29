#include "GameObject.h"

#include "Layer.h"
#include "Prefab.h"

#include "EngineCore.h"
#include "SceneManagerEM.h"
#include "Component.h"
#include "Transform.h"
#include "Sprite2D.h"
#include "Camera.h"
#include "Animator.h"
#include "AudioSource.h"
#include "Collider2D.h"
#include "Canvas.h"
#include "ParticleSystem.h"
#include "TileMap.h"
#include "Script.h"
#include "Light.h"
#include "Layer.h"

#include "Log.h"
#include "Defs.h"

#include <random>
#include <ctime>
#include <unordered_set>

GameObject::GameObject(std::weak_ptr<GameObject> parent, std::string name, bool enable) :
    gameobject_id(GenerateGameObjectID()),
    parent_gameobject(parent),
    gameobject_name(""),
    gameobject_tag("Default"),
    enabled(enable),
    visible(true)
{
    this->gameobject_name = GenerateUniqueName(name, this);

    LOG(LogType::LOG_INFO, "GameObject <%s - id: %u> created", gameobject_name.c_str(), gameobject_id);
    //NEVER call AddComponent/AddChild/CloneComponents/ClonHierarchy or similar in constructor
    //WE CANT DO shared_from_this in a constructor!!
}

GameObject::GameObject(std::weak_ptr<GameObject> go_to_copy) :
    gameobject_id(GenerateGameObjectID()),
    parent_gameobject(go_to_copy.lock()->GetParentGO()),
    gameobject_name(""),
    gameobject_tag(go_to_copy.lock()->GetTag()),
    enabled(go_to_copy.lock()->IsEnabled()),
    visible(go_to_copy.lock()->IsVisible()),
    prefab_path(go_to_copy.lock()->prefab_path),
    prefab_modified(go_to_copy.lock()->prefab_modified),
    prefab_original_data(go_to_copy.lock()->prefab_original_data)
{
    this->gameobject_name = GenerateUniqueName(go_to_copy.lock()->GetName(), this);

    LOG(LogType::LOG_INFO, "GameObject <%s - id: %u> created from <%s - id: %u>", gameobject_name.c_str(), gameobject_id, go_to_copy.lock()->GetName().c_str(), go_to_copy.lock()->GetID());
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

bool GameObject::Init()
{
    bool ret = true;

    //init components
    for (const auto& item : components_gameobject) if (item->IsEnabled()) item->Init();

    //then, init children game objects
    for (const auto& item : children_gameobject) if (item->IsEnabled()) item->Init();

    return ret;
}

bool GameObject::Update(double dt)
{
    bool ret = true;

    auto children_copy = children_gameobject;

    //update components
    for (const auto& item : components_gameobject) if (item->IsEnabled()) item->Update(dt);

    //then, update children game objects
    for (const auto& item : children_copy) if (item->IsEnabled()) item->Update(dt);

    return ret;
}

bool GameObject::CleanUp()
{
    bool ret = true;

    //cleanup components
    for (const auto& item : components_gameobject) if (item->IsEnabled()) item->CleanUp();

    //then, cleanup children game objects
    for (const auto& item : children_gameobject) if (item->IsEnabled()) item->CleanUp();

    return ret;
}

bool GameObject::Pause()
{
    bool ret = true;

    //pause components
    for (const auto& item : components_gameobject) if (item->IsEnabled()) item->Pause();

    //then, pause children game objects
    for (const auto& item : children_gameobject) if (item->IsEnabled()) item->Pause();

    return ret;
}

bool GameObject::Unpause()
{
    bool ret = true;

    //unpause components
    for (const auto& item : components_gameobject) if (item->IsEnabled()) item->Unpause();

    //then, unpause children game objects
    for (const auto& item : children_gameobject) if (item->IsEnabled()) item->Unpause();

    return ret;
}

void GameObject::Draw()
{
    std::ranges::reverse(components_gameobject);

    //draw components
    for (const auto& item : components_gameobject) if (item->IsEnabled()) item->Draw();

    std::ranges::reverse(components_gameobject);

    //layer system!! do not iterate children.draw()
}

void GameObject::PrefabChecker()
{
    if (IsPrefabInstance() && !prefab_modified)
    {
        std::string t_prefab_path = prefab_path;
        prefab_path = "";
        nlohmann::json currentState = SaveGameObject();
        prefab_path = t_prefab_path;
        nlohmann::json cleanCurrent = currentState;
        Prefab::RemoveIDs(cleanCurrent);

        nlohmann::json cleanOriginal = prefab_original_data;
        Prefab::RemoveIDs(cleanOriginal);

        if (cleanCurrent.dump() != cleanOriginal.dump())
        {
            LOG(LogType::LOG_INFO, "Prefab is now different from the file:\n\nCurrent: %s\n\n\n", cleanCurrent.dump().c_str());
            LOG(LogType::LOG_INFO, "Original: %s", cleanOriginal.dump().c_str());
            prefab_modified = true;
        }
    }

    for (const auto& item : children_gameobject) if (item->IsEnabled()) item->PrefabChecker();
}

void GameObject::FinishLoad()
{
    //finish load components // scripts are finish-loaded separately
    for (const auto& item : components_gameobject) if (item->GetType() != ComponentType::Script) item->FinishLoad();

    //then, finish load children game objects
    for (const auto& item : children_gameobject)item->FinishLoad();
}

void GameObject::Delete()
{
    std::string obj_to_delete_name = gameobject_name;
    uint32_t id = gameobject_id;

    components_gameobject.clear();

    auto children_copy = children_gameobject;
    children_gameobject.clear();

    if (this->parent_layer.lock() != nullptr)
        if (this->parent_layer.lock()->HasChild(shared_from_this()))
            this->parent_layer.lock()->RemoveChild(shared_from_this());

    for (auto& child : children_copy) child->Delete();

    if (auto parent_sp = parent_gameobject.lock())
        if (parent_sp->HasChild(gameobject_id))
            parent_sp->RemoveChild(shared_from_this());

    LOG(LogType::LOG_INFO, "GameObject <%s - id: %u> has been deleted", obj_to_delete_name.c_str(), id);
}

bool GameObject::Reparent(std::shared_ptr<GameObject> new_parent, bool skip_descendant_search)
{
    auto self = shared_from_this();
    bool cycle = false;
    if (new_parent.get() == self.get() || new_parent == self->GetParentGO().lock()) return false;
    if (!skip_descendant_search && new_parent->IsDescendant(self)) return false;
    if (new_parent->GetParentGO().lock() == self) cycle = true;

    auto old_parent = parent_gameobject.lock();
    if (old_parent) old_parent->RemoveChild(self);
    if (new_parent) new_parent->AddChild(self);

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

bool GameObject::MoveInVector(int new_position)
{
    auto parent_sp = parent_gameobject.lock();
    if (!parent_sp) return false;

    auto& children = parent_sp->children_gameobject;
    auto self = shared_from_this();

    auto it = std::find(children.begin(), children.end(), self);
    if (it == children.end()) return false;

    const int original_index = std::distance(children.begin(), it);
    children.erase(it);

    if (new_position > original_index) new_position--;

    new_position = std::clamp(new_position, 0, static_cast<int>(children.size()));
    children.insert(children.begin() + new_position, self);

    return true;
}

void GameObject::CloneChildrenHierarchy(const std::shared_ptr<GameObject>& original)
{
    for (const auto& original_child : original->children_gameobject)
    {
        //base copy
        auto child_copy = std::make_shared<GameObject>(original_child);
        child_copy->SetName(original_child->GetName(), false);
        AddChild(child_copy);
        //layer management
        //if (original_child->GetParentLayer().lock().get())
        //{
        //    original_child->GetParentLayer().lock()->AddChild(child_copy);
        //    child_copy->SetParentLayer(original_child->GetParentLayer().lock());
        //}
        //component management
        child_copy->CloneComponents(original_child);
        //children management
        child_copy->CloneChildrenHierarchy(original_child);
    }
}

void GameObject::CloneComponents(const std::shared_ptr<GameObject>& original, bool same_id)
{
    for (const auto& item : original->GetComponents())
    {
        switch (item->GetType())
        {
        case ComponentType::Transform:
        {
            AddCopiedComponent<Transform>(*dynamic_cast<const Transform*>(item.get()));
            if (same_id) GetComponent<Transform>()->SetID(item->GetID());
            break;
        }
        case ComponentType::Camera:
        {
            AddCopiedComponent<Camera>(*dynamic_cast<const Camera*>(item.get()));
            if (same_id) GetComponent<Camera>()->SetID(item->GetID());
            break;
        }
        case ComponentType::Sprite:
        {
            AddCopiedComponent<Sprite2D>(*dynamic_cast<const Sprite2D*>(item.get()));
            if (same_id) GetComponent<Sprite2D>()->SetID(item->GetID());
            break;
        }
        case ComponentType::Animator:
        {
            AddCopiedComponent<Animator>(*dynamic_cast<const Animator*>(item.get()));
            if (same_id) GetComponent<Animator>()->SetID(item->GetID());
            break;
        }
        case ComponentType::Script:
        {
            AddCopiedComponent<Script>(*dynamic_cast<const Script*>(item.get()));
            if (same_id) GetComponent<Script>()->SetID(item->GetID());
            break;
        }
        case ComponentType::Collider2D:
        {
            AddCopiedComponent<Collider2D>(*dynamic_cast<const Collider2D*>(item.get()));
            if (same_id) GetComponent<Collider2D>()->SetID(item->GetID());
            break;
        }
        case ComponentType::Canvas:
        {
            AddCopiedComponent<Canvas>(*dynamic_cast<const Canvas*>(item.get()));
            if (same_id) GetComponent<Canvas>()->SetID(item->GetID());
            break;
        }
        case ComponentType::AudioSource:
        {
            AddCopiedComponent<AudioSource>(*dynamic_cast<const AudioSource*>(item.get()));
            if (same_id) GetComponent<AudioSource>()->SetID(item->GetID());
            break;
        }
        case ComponentType::ParticleSystem:
        {
            AddCopiedComponent<ParticleSystem>(*dynamic_cast<const ParticleSystem*>(item.get()));
            if (same_id) GetComponent<ParticleSystem>()->SetID(item->GetID());
            break;
        }
        case ComponentType::TileMap:
        {
            AddCopiedComponent<TileMap>(*dynamic_cast<const TileMap*>(item.get()));
            if (same_id) GetComponent<TileMap>()->SetID(item->GetID());
            break;
        }
        case ComponentType::Light:
        {
            AddCopiedComponent<Light>(*dynamic_cast<const Light*>(item.get()));
            if (same_id) GetComponent<Light>()->SetID(item->GetID());
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
    if (auto old_parent = child->parent_gameobject.lock())
    {
        old_parent->RemoveChild(child);
    }
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
        if (child.get() != go) existingNames.insert(child->GetName());

    if (existingNames.count(newName) > 0)
    {
        int count = 1;
        while (existingNames.count(newName) > 0)
        {
            size_t pos = newName.find_last_of('(');
            if (pos != std::string::npos && newName.back() == ')' && newName[pos] == '(' && pos > 0)
            {
                int num = std::stoi(newName.substr(pos + 1, newName.size() - pos - 2));
                newName = newName.substr(0, pos - 1) + "_(" + std::to_string(++num) + ")";
            }
            else
            {
                newName = baseName + "_(" + std::to_string(count++) + ")";
            }
        }
    }

    return newName;
}

nlohmann::json GameObject::SaveGameObject()
{
    nlohmann::json goJSON;

    if (this->parent_gameobject.lock())
    {
        goJSON["ParentID"] = this->parent_gameobject.lock()->GetID();
    }
    if (this->parent_layer.lock())
    {
        goJSON["ParentLayerID"] = this->parent_layer.lock()->GetLayerID();
    }

    goJSON["ID"] = this->gameobject_id;

    if (IsPrefabInstance() && !prefab_modified)
    {
        goJSON["PrefabPath"] = prefab_path;
        return goJSON;
    }
    else if (IsPrefabInstance() && prefab_modified)
    {
        goJSON["OriginalPrefabPath"] = prefab_path;
    }

    goJSON["Name"] = this->gameobject_name;
    goJSON["Tag"] = this->gameobject_tag;
    goJSON["Enabled"] = this->enabled;
    goJSON["Visible"] = this->visible;

    if (!this->components_gameobject.empty())
    {
        nlohmann::json componentsJSON;
        for (const auto& component : this->components_gameobject)
        {
            componentsJSON.push_back(component->SaveComponent());
        }
        goJSON["Components"] = componentsJSON;
    }

    if (!this->children_gameobject.empty())
    {
        nlohmann::json childrenJSON;
        for (const auto& child : this->children_gameobject)
        {
            childrenJSON.push_back(child->SaveGameObject());
        }
        goJSON["GameObjects"] = childrenJSON;
    }
    
    return goJSON;
}

void GameObject::LoadGameObject(const nlohmann::json& goJSON)
{
    if (goJSON.contains("ID")) this->gameobject_id = goJSON["ID"];
    if (goJSON.contains("Name")) this->gameobject_name = goJSON["Name"];
    if (goJSON.contains("Tag")) this->gameobject_tag = goJSON["Tag"];
    if (goJSON.contains("Enabled")) this->enabled = goJSON["Enabled"];
    if (goJSON.contains("Visible")) this->visible = goJSON["Visible"];

    if (goJSON.contains("PrefabPath") && !IsPrefabInstance())
    {
        std::string path = goJSON["PrefabPath"];

        auto tempPrefab = Prefab::Instantiate(path, nullptr, engine->GetEngineState() == EngineState::PLAY);

        if (tempPrefab)
        {
            this->gameobject_name = tempPrefab->gameobject_name;
            this->gameobject_tag = tempPrefab->gameobject_tag;
            this->enabled = tempPrefab->enabled;
            this->visible = tempPrefab->visible;

            RemoveComponent(ComponentType::Transform);
            CloneComponents(tempPrefab);
            CloneChildrenHierarchy(tempPrefab);

            prefab_original_data = tempPrefab->prefab_original_data;
            prefab_path = path;
        }
    }
    else if (goJSON.contains("OriginalPrefabPath") && !IsPrefabInstance())
    {
        std::string path = goJSON["OriginalPrefabPath"];
        auto tempPrefab = Prefab::Instantiate(path, nullptr, engine->GetEngineState() == EngineState::PLAY);

        prefab_original_data = tempPrefab->prefab_original_data;
        prefab_path = path;
        prefab_modified = true;
    }

    //components
    if (goJSON.contains("Components"))
    {
        if (!components_gameobject.empty())
            for (const auto& item : components_gameobject) if (item->IsEnabled()) item->CleanUp();
        components_gameobject.clear();
        const nlohmann::json& componentsJSON = goJSON["Components"];

        for (const auto& componentJSON : componentsJSON)
        {
            if (!componentJSON.contains("ComponentType")) break;
            if (componentJSON["ComponentType"] == (int)ComponentType::Transform)
            {
                this->AddComponent<Transform>();
                this->GetComponent<Transform>()->LoadComponent(componentJSON);
            }
            else if (componentJSON["ComponentType"] == (int)ComponentType::Camera)
            {
                this->AddComponent<Camera>();
                this->GetComponent<Camera>()->LoadComponent(componentJSON);
            }
            else if (componentJSON["ComponentType"] == (int)ComponentType::Sprite)
            {
                this->AddComponent<Sprite2D>("");
                this->GetComponent<Sprite2D>()->LoadComponent(componentJSON);
            }
            else if (componentJSON["ComponentType"] == (int)ComponentType::Animator)
            {
                this->AddComponent<Animator>();
                this->GetComponent<Animator>()->LoadComponent(componentJSON);
            }
            else if (componentJSON["ComponentType"] == (int)ComponentType::Script)
            {
                this->AddComponent<Script>();
                this->GetComponents<Script>()[this->GetComponents<Script>().size() - 1]->LoadComponent(componentJSON);
            }
            else if (componentJSON["ComponentType"] == (int)ComponentType::Collider2D)
            {
                this->AddComponent<Collider2D>();
                this->GetComponent<Collider2D>()->LoadComponent(componentJSON);
            }
            else if (componentJSON["ComponentType"] == (int)ComponentType::Canvas)
            {
                this->AddComponent<Canvas>();
                this->GetComponent<Canvas>()->LoadComponent(componentJSON);
            }
            else if (componentJSON["ComponentType"] == (int)ComponentType::AudioSource)
            {
                this->AddComponent<AudioSource>();
                this->GetComponent<AudioSource>()->LoadComponent(componentJSON);
            }
            else if (componentJSON["ComponentType"] == (int)ComponentType::ParticleSystem)
            {
                this->AddComponent<ParticleSystem>();
                this->GetComponent<ParticleSystem>()->LoadComponent(componentJSON);
            }
            else if (componentJSON["ComponentType"] == (int)ComponentType::TileMap)
            {
                this->AddComponent<TileMap>();
                this->GetComponent<TileMap>()->LoadComponent(componentJSON);
            }
            else if (componentJSON["ComponentType"] == (int)ComponentType::Light)
            {
                this->AddComponent<Light>();
                this->GetComponent<Light>()->LoadComponent(componentJSON);
            }
        }
    }

    //children
    if (goJSON.contains("GameObjects"))
    {
        if (!children_gameobject.empty())
            for (const auto& item : children_gameobject) if (item->IsEnabled()) item->CleanUp();
        children_gameobject.clear();
        const nlohmann::json& childrenJSON = goJSON["GameObjects"];
        for (const auto& childJSON : childrenJSON)
        {
            std::shared_ptr<GameObject> child_go = std::make_shared<GameObject>(shared_from_this(), "EmptyGameObject", true);
            AddChild(child_go);
            child_go->LoadGameObject(childJSON);
        }
    }
}

void GameObject::CheckForEmptyLayers(Scene* scene)
{
    for (const auto& child : children_gameobject)
    {
        auto child_layer = child->parent_layer.lock();
        if (child_layer && child_layer->HasChild(child->gameobject_id))
        {
            child->CheckForEmptyLayers(scene);
            continue;
        }

        auto parent_layer_ptr = parent_layer.lock();

        if (!parent_layer_ptr)
        {
            if (!scene->GetSceneLayers().empty())
            {
                parent_layer_ptr = scene->GetSceneLayers()[0];
            }
            else
            {
                child->CheckForEmptyLayers(scene);
                continue;
            }
        }

        child->SetParentLayer(parent_layer_ptr);

        auto& layer_children = parent_layer_ptr->GetChildren();
        auto parent_pos = std::find(layer_children.begin(), layer_children.end(), shared_from_this());

        if (parent_pos != layer_children.end())
        {
            layer_children.insert(parent_pos + 1, child);
        }
        else
        {
            layer_children.push_back(child);
        }

        child->CheckForEmptyLayers(scene);
    }
}

void GameObject::SetParentLayer(std::shared_ptr<Layer> layer)
{
    if (auto old_layer = parent_layer.lock())
    {
        if (old_layer == layer) return;
        old_layer->RemoveChild(shared_from_this());
    }
    parent_layer = layer;
}

void GameObject::SetPrefabPath(std::string& path)
{
    std::replace(path.begin(), path.end(), '/', '\\');
    prefab_path = path;
}
