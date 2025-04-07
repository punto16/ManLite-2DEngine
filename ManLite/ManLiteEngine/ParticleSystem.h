#ifndef __PARTICLE_SYSTEM_H__
#define __PARTICLE_SYSTEM_H__
#pragma once

#include "Component.h"

#include "vector"
#include "memory"

class Emmiter;

class ParticleSystem : public Component {
public:
    ParticleSystem(std::weak_ptr<GameObject> container_go, std::string name = "ParticleSystem", bool enable = true);
    ParticleSystem(const ParticleSystem& component_to_copy, std::shared_ptr<GameObject> container_go);
    ~ParticleSystem();

    bool Init();
    bool Update(float dt);

    void Draw();

    bool Pause();
    bool Unpause();

    //serialization
    nlohmann::json SaveComponent() override;
    void LoadComponent(const nlohmann::json& componentJSON) override;

    //getters // setters
    std::vector<std::shared_ptr<Emmiter>>& GetEmmiters() { return emmiters; }

private:
    std::vector<std::shared_ptr<Emmiter>> emmiters;
};

#endif // !__PARTICLE_SYSTEM_H__