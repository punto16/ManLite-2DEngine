#include "Animator.h"

#include "GameObject.h"
#include "Sprite2D.h"

Animator::Animator(std::weak_ptr<GameObject> container_go, std::string name, bool enable) :
    Component(container_go, ComponentType::Animator, name, enable)
{
}

Animator::Animator(const Animator& component_to_copy, std::shared_ptr<GameObject> container_go) :
    Component(component_to_copy)
{
    for (auto& pair : component_to_copy.animations)
        this->AddAnimation(pair.first, *pair.second);
}

Animator::~Animator()
{
    for (auto& pair : animations)
        ResourceManager::GetInstance().ReleaseAnimation(pair.first);
}

bool Animator::Update(float deltaTime)
{
    if (sprite == nullptr)
    {
        sprite = container_go.lock()->GetComponent<Sprite2D>();
    }
    if (currentAnimation != nullptr && sprite != nullptr)
    {
        currentAnimation->Update(deltaTime);
        ML_Rect r = currentAnimation->GetCurrentFrame();
        sprite->SetTextureSection(r.x, r.y, r.w, r.h);
    }
    return true;
}

void Animator::AddAnimation(const std::string& name, const Animation& animation)
{
    Animation* anim = ResourceManager::GetInstance().LoadAnimation(name, animation);
    animations[name] = anim;
}

void Animator::Play(const std::string& name)
{
    Animation* anim = ResourceManager::GetInstance().GetAnimation(name);
    if (anim)
    {
        currentAnimation = anim;
        currentAnimationName = name;
        currentAnimation->Reset();
    }
}

json Animator::SaveComponent()
{
    json componentJSON;
    //component generic
    componentJSON["ContainerGOID"] = this->container_go.lock()->GetID();
    componentJSON["ComponentID"] = component_id;
    componentJSON["ComponentName"] = name;
    componentJSON["ComponentType"] = (int)type;
    componentJSON["Enabled"] = enabled;

    //component spcecific
    json animationsJSON;
    for (auto& pair : animations) {
        json animJSON;
        animJSON["name"] = pair.first;
        animJSON["data"] = pair.second->Save();

        animationsJSON.push_back(animJSON);
    }
    componentJSON["CustomAnimations"] = animationsJSON;

    if (currentAnimation) componentJSON["CurrentAnimation"] = currentAnimationName;

    return componentJSON;
}

void Animator::LoadComponent(const json& componentJSON)
{
    if (componentJSON.contains("ComponentID")) component_id = componentJSON["ComponentID"];
    if (componentJSON.contains("ComponentName")) name = componentJSON["ComponentName"];
    if (componentJSON.contains("ComponentType")) type = (ComponentType)componentJSON["ComponentType"];
    if (componentJSON.contains("Enabled")) enabled = componentJSON["Enabled"];
    
    if (componentJSON.contains("CustomAnimations")) {
        for (auto& animJSON : componentJSON["CustomAnimations"]) {
            std::string animName = animJSON["name"];
    
            Animation newAnim;
            newAnim.Load(animJSON["data"]);
    
            AddAnimation(animName, newAnim);
        }
    }
    
    if (componentJSON.contains("CurrentAnimation"))
    {
        std::string currentAnimName = componentJSON["CurrentAnimation"];
        Play(currentAnimName);
    }
}
