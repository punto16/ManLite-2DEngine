#include "Animator.h"

#include "GameObject.h"
#include "Sprite2D.h"

Animator::Animator(std::weak_ptr<GameObject> container_go, std::string name, bool enable) :
    Component(container_go, ComponentType::Animator, name, enable),
    currentFrame(0.0f)
{
}

Animator::Animator(const Animator& component_to_copy, std::shared_ptr<GameObject> container_go) :
    Component(component_to_copy, container_go)
{
    for (auto& pair : component_to_copy.animations)
        this->AddAnimation(pair.first, pair.second.filePath);

    Play(component_to_copy.currentAnimationName);
}

Animator::~Animator()
{
    for (auto& pair : animations)
        ResourceManager::GetInstance().ReleaseAnimation(pair.second.filePath);
}

bool Animator::Update(float deltaTime)
{
    if (sprite == nullptr)
    {
        sprite = container_go.lock()->GetComponent<Sprite2D>();
    }
    if (currentAnimation != nullptr && sprite != nullptr)
    {
        currentAnimation->Update(deltaTime, currentFrame);
        ML_Rect r = currentAnimation->GetCurrentFrame(currentFrame);
        sprite->SetTextureSection(r.x, r.y, r.w, r.h);
    }

    return true;
}

void Animator::AddAnimation(const std::string& name, const std::string& filePath)
{
    Animation* anim = ResourceManager::GetInstance().LoadAnimation(filePath);
    if (anim) animations[name] = { anim, filePath };
}

void Animator::RemoveAnimation(const std::string& name)
{
    if (IsPlaying(name)) Stop();
    auto it = animations.find(name);
    if (it != animations.end()) {
        ResourceManager::GetInstance().ReleaseAnimation(it->second.filePath);
        animations.erase(it);
    }
}

bool Animator::HasAnimation(const std::string& name)
{
    return animations.find(name) != animations.end();
}

void Animator::Play(const std::string& name)
{
    Animation* anim = nullptr;

    auto it = animations.find(name);
    if (it != animations.end()) anim = it->second.animation;

    if (!anim) anim = ResourceManager::GetInstance().GetAnimation(name);
    if (!anim) 
    {
        std::string anim_name = std::filesystem::path(name).stem().string();
        auto it2 = animations.find(anim_name);
        if (it2 != animations.end())
            anim = ResourceManager::GetInstance().GetAnimation(it2->second.filePath);
    }
    if (anim)
    {
        currentAnimation = anim;
        currentAnimationName = name;
        currentAnimation->Reset(currentFrame);
    }
}

nlohmann::json Animator::SaveComponent()
{
    nlohmann::json componentJSON;
    //component generic
    componentJSON["ContainerGOID"] = this->container_go.lock()->GetID();
    componentJSON["ComponentID"] = component_id;
    componentJSON["ComponentName"] = name;
    componentJSON["ComponentType"] = (int)type;
    componentJSON["Enabled"] = enabled;

    //component spcecific
    nlohmann::json animationsJSON;
    for (auto& pair : animations) {
        nlohmann::json animJSON;
        animJSON["name"] = pair.first;
        animJSON["path"] = pair.second.filePath;

        animationsJSON.push_back(animJSON);
    }
    componentJSON["Animations"] = animationsJSON;

    if (currentAnimation) componentJSON["CurrentAnimation"] = currentAnimationName;

    return componentJSON;
}

void Animator::LoadComponent(const nlohmann::json& componentJSON)
{
    if (componentJSON.contains("ComponentID")) component_id = componentJSON["ComponentID"];
    if (componentJSON.contains("ComponentName")) name = componentJSON["ComponentName"];
    if (componentJSON.contains("ComponentType")) type = (ComponentType)componentJSON["ComponentType"];
    if (componentJSON.contains("Enabled")) enabled = componentJSON["Enabled"];
    
    if (componentJSON.contains("Animations")) {
        for (auto& animJSON : componentJSON["Animations"]) {
            std::string animName = animJSON["name"];
            std::string animPath = animJSON["path"];
    
            AddAnimation(animName, animPath);
        }
    }
    
    if (componentJSON.contains("CurrentAnimation"))
    {
        std::string currentAnimName = componentJSON["CurrentAnimation"];
        Play(currentAnimName);
    }
}
