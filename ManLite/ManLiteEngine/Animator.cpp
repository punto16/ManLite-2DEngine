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
        currentAnimation->Reset();
    }
}
