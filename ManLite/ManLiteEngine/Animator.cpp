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
}

Animator::~Animator()
{
}

bool Animator::Update(float deltaTime)
{
    if (sprite == nullptr)
    {
        sprite = container_go.lock()->GetComponent<Sprite2D>();
    }
    if (currentAnimation != nullptr && sprite != nullptr) {
        currentAnimation->Update(deltaTime);
        ML_Rect r = currentAnimation->GetCurrentFrame();
        sprite->SetTextureSection(r.x, r.y, r.w, r.h);
    }
    return true;
}
