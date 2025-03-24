#ifndef __ANIMATOR_H__
#define __ANIMATOR_H__
#pragma once

#include "Component.h"
#include "Animation.h"
#include <unordered_map>

class Sprite2D;

struct AnimationRef
{
    Animation* animation;
    std::string filePath;
};

class Animator : public Component
{
public:

    Animator(std::weak_ptr<GameObject> container_go, std::string name = "Animator", bool enable = true);
    Animator(const Animator& component_to_copy, std::shared_ptr<GameObject> container_go);
    ~Animator();

    bool Update(float deltaTime) override;

    void AddAnimation(const std::string& name, const std::string& filePath);
    bool HasAnimation(const std::string& name);

    void Play(const std::string& name);
    void Stop() { currentAnimation = nullptr; currentAnimationName.clear(); }

    bool IsPlaying(const std::string& name) const {
        return currentAnimation == animations.at(name).animation;
    }

    //serialization
    nlohmann::json SaveComponent() override;
    void LoadComponent(const nlohmann::json& componentJSON) override;

    //getters // setters
    std::unordered_map<std::string, AnimationRef>& GetAnimations() { return animations; }
    Animation* GetCurrentAnimation() { return currentAnimation; }
    std::string GetCurrentAnimationName() { return currentAnimationName; }

private:
    Sprite2D* sprite = nullptr;
    std::unordered_map<std::string, AnimationRef> animations;
    Animation* currentAnimation = nullptr;
    std::string currentAnimationName;
};
#endif // !__ANIMATOR_H__