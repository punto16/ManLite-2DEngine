#ifndef __PARTICLE_SYSTEM_H__
#define __PARTICLE_SYSTEM_H__
#pragma once

#include "Component.h"

#include "vector"
#include "memory"

class Emitter;

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

    void Stop();

    std::shared_ptr<Emitter> AddEmptyEmitter();
    std::shared_ptr<Emitter> AddDuplicatedEmitter(Emitter* ref);

    //serialization
    nlohmann::json SaveComponent() override;
    void LoadComponent(const nlohmann::json& componentJSON) override;

    void SaveParticleSystemToFile(std::string save_path);
    void LoadParticleSystemToFile(std::string load_path);

    //getters // setters
    std::vector<std::shared_ptr<Emitter>>& GetEmitters() { return emitters; }

    std::string GetPath() { return path; }
    void SetPath(std::string p) { path = p; }

    bool IsStop() { return stop; }

private:
    std::vector<std::shared_ptr<Emitter>> emitters;

    std::string path;

    bool paused = false;
    bool stop = false;
};

#endif // !__PARTICLE_SYSTEM_H__