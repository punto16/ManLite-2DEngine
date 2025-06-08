#include "ParticleSystem.h"

#include "GameObject.h"
#include "Emitter.h"
#include "EngineCore.h"
#include "ResourceManager.h"
#include "RendererEM.h"
#include "Transform.h"

#include "filesystem"
#include "fstream"

ParticleSystem::ParticleSystem(std::weak_ptr<GameObject> container_go, std::string name, bool enable) :
    Component(container_go, ComponentType::ParticleSystem, name, enable),
    path(""),
    paused(false),
    stop(true)
{
}

ParticleSystem::ParticleSystem(const ParticleSystem& component_to_copy, std::shared_ptr<GameObject> container_go) :
	Component(component_to_copy, container_go),
    path(component_to_copy.path),
    paused(component_to_copy.paused),
    stop(component_to_copy.stop)
{
    for (const auto& emitter : component_to_copy.emitters)
    {
        AddDuplicatedEmitter(emitter.get());
    }
}

ParticleSystem::~ParticleSystem()
{
    emitters.clear();
}

bool ParticleSystem::Init()
{
    bool ret = true;

    stop = false;
    paused = false;

    for (const auto& emitter : emitters)
        if (!emitter->Init()) return false;

    return ret;
}

bool ParticleSystem::Update(float dt)
{
    bool ret = true;

    if (paused) return ret;

    for (const auto& emitter : emitters)
        if (!emitter->Update(dt)) return false;

    return ret;
}

void ParticleSystem::Draw()
{
    for (const auto& emitter : emitters)
        emitter->Draw();

    //gizmo
    if (!engine->GetEditorOrBuild() || !engine->renderer_em->rend_colliders) return;
    if (auto gizmo_img = ResourceManager::GetInstance().GetTexture("Config\\Icons\\particles_gizmo.png"))
    {
        if (auto t = container_go.lock()->GetComponent<Transform>())
        {
            vec2f scale = t->GetScale();
            bool a_lock = t->IsAspectRatioLocked();
            float angle = t->GetAngle();

            t->SetAspectRatioLock(false);
            t->SetWorldScale({ 0.6f, 0.6f });
            t->SetWorldAngle(0.0f);

            mat3f worldMat = t->GetWorldMatrix();

            t->SetAngle(angle);
            t->SetScale(scale);
            t->SetAspectRatioLock(a_lock);


            engine->renderer_em->SubmitSprite(
                gizmo_img,
                worldMat,
                0, 0, 1, 1,
                true,
                0,
                0
            );
        }
    }
}

bool ParticleSystem::Pause()
{
    bool ret = true;

    for (const auto& emitter : emitters)
        if (!emitter->Pause()) return false;

    paused = true;

    return ret;
}

bool ParticleSystem::Unpause()
{
    bool ret = true;

    for (const auto& emitter : emitters)
        if (!emitter->Unpause()) return false;

    paused = false;

    return ret;
}

void ParticleSystem::Stop()
{
    stop = true;
    paused = false;
}

std::shared_ptr<Emitter> ParticleSystem::AddEmptyEmitter()
{
    std::shared_ptr<Emitter> e = std::make_shared<Emitter>(container_go.lock()->weak_from_this());
    emitters.emplace_back(e);
    return e;
}

std::shared_ptr<Emitter> ParticleSystem::AddDuplicatedEmitter(Emitter* ref)
{
    std::shared_ptr<Emitter> e = std::make_shared<Emitter>(*ref, container_go.lock()->shared_from_this());
    emitters.emplace_back(e);
    return e;
}

nlohmann::json ParticleSystem::SaveComponent()
{
    nlohmann::json componentJSON;
    //component generic
    componentJSON["ContainerGOID"] = this->container_go.lock()->GetID();
    componentJSON["ComponentID"] = component_id;
    componentJSON["ComponentName"] = name;
    componentJSON["ComponentType"] = (int)type;
    componentJSON["Enabled"] = enabled;

    //component spcecific
    componentJSON["ParticlePath"] = path;

    //call to save all particles in path
    SaveParticleSystemToFile(path);

    return componentJSON;
}

void ParticleSystem::LoadComponent(const nlohmann::json& componentJSON)
{
    if (componentJSON.contains("ComponentID")) component_id = componentJSON["ComponentID"];
    if (componentJSON.contains("ComponentName")) name = componentJSON["ComponentName"];
    if (componentJSON.contains("ComponentType")) type = (ComponentType)componentJSON["ComponentType"];
    if (componentJSON.contains("Enabled")) enabled = componentJSON["Enabled"];

    if (componentJSON.contains("ParticlePath")) path = componentJSON["ParticlePath"];

    //load all particle system data here from its path
    LoadParticleSystemToFile(path);
}

void ParticleSystem::SaveParticleSystemToFile(std::string save_path)
{
    path = save_path;
    nlohmann::json emittersArray;
    for (auto& emitter : emitters) {
        emittersArray.push_back(emitter->SaveComponent());
    }

    std::ofstream file(path);
    if (file.is_open()) {
        file << emittersArray.dump(4);
        file.close();
    }
}

void ParticleSystem::LoadParticleSystemToFile(std::string load_path)
{
    path = load_path;
    std::ifstream file(path);
    if (file.is_open()) {
        nlohmann::json emittersArray;
        file >> emittersArray;

        emitters.clear();

        for (auto& emitterJSON : emittersArray) {
            std::shared_ptr<Emitter> e = AddEmptyEmitter();
            e->LoadComponent(emitterJSON);
        }
        file.close();
    }
}
