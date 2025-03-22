#ifndef __ANIMATOR_H__
#define __ANIMATOR_H__
#pragma once

#include "Component.h"
#include "Animation.h"
#include <unordered_map>

class Sprite2D;

class Animator : public Component
{
public:

    Animator(std::weak_ptr<GameObject> container_go, std::string name = "Animator", bool enable = true);
    Animator(const Animator& component_to_copy, std::shared_ptr<GameObject> container_go);
    ~Animator();

    bool Update(float deltaTime) override;

    void AddAnimation(const std::string& name, const Animation& animation) {
        animations[name] = animation;
    }

    void Play(const std::string& name) {
        auto it = animations.find(name);
        if (it != animations.end()) {
            currentAnimation = &it->second;
            currentAnimation->Reset();
        }
    }

    bool IsPlaying(const std::string& name) const {
        return currentAnimation == &animations.at(name);
    }

private:
    Sprite2D* sprite = nullptr;
    std::unordered_map<std::string, Animation> animations;
    Animation* currentAnimation = nullptr;
};
#endif // !__ANIMATOR_H__