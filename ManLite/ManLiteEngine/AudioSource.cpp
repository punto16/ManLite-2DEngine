#include "AudioSource.h"
#include "GameObject.h"
#include "Log.h"

#include "Transform.h"
#include "EngineCore.h"
#include "SceneManagerEM.h"
#include "ResourceManager.h"
#include "RendererEM.h"

#include "SDL2/SDL_mixer.h"

AudioSource::AudioSource(std::weak_ptr<GameObject> container_go, std::string name, bool enable)
    : Component(container_go, ComponentType::AudioSource, name, enable)
{
}

AudioSource::AudioSource(const AudioSource& component_to_copy, std::shared_ptr<GameObject> container_go)
    : Component(component_to_copy, container_go)
{
    for (auto& pair : component_to_copy.sounds)
    {
        AddSound(pair.first, pair.second.filePath, pair.second.volume, pair.second.loop, pair.second.play_on_awake, pair.second.spatial, pair.second.spatial_distance);
        if (pair.second.listener.lock().get()) SetListener(pair.first, pair.second.listener.lock());
    }

    for (auto& pair : component_to_copy.musics)
        AddMusic(pair.first, pair.second.filePath, pair.second.volume, pair.second.loop, pair.second.play_on_awake);
}

AudioSource::~AudioSource()
{
    StopAll();
    RemoveAllSounds();
    RemoveAllMusic();
}

bool AudioSource::Init()
{
    for (auto& pair : sounds)
        if (pair.second.play_on_awake)
            PlaySound(pair.first);

    for (auto& pair : musics)
        if (pair.second.play_on_awake)
            PlayMusic(pair.first);

    return true;
}

bool AudioSource::Update(float dt)
{
    for (auto& pair : sounds)
    {
        SoundRef& sound = pair.second;
        if (sound.spatial && sound.channel != -1)
        {
            if (Mix_Playing(sound.channel))
            {
                if (pair.second.listener_id != 0 && engine->scene_manager_em->CurrentSceneAvailable())
                {
                    pair.second.listener = engine->scene_manager_em->GetCurrentScene().FindGameObjectByID(pair.second.listener_id);
                }
                UpdateSoundPosition(sound);
            }
            else
            {
                sound.channel = -1;
            }
        }
    }
    return true;
}

void AudioSource::Draw()
{
    //gizmo
    if (!engine->GetEditorOrBuild() || !engine->renderer_em->rend_colliders) return;
    if (auto gizmo_img = ResourceManager::GetInstance().GetTexture("Config\\Icons\\audio_gizmo.png"))
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

bool AudioSource::Pause()
{
    bool ret = true;

    PauseAll();

    return ret;
}

bool AudioSource::Unpause()
{
    bool ret = true;

    UnpauseAll();

    return ret;
}

void AudioSource::FinishLoad()
{
    for (auto& pair : sounds)
        pair.second.listener = engine->scene_manager_em->GetCurrentScene().FindGameObjectByID(pair.second.listener_id);
}

void AudioSource::AddSound(const std::string& name, const std::string& filePath, int volume, bool loop, bool play_on_awake, bool spatial, int spatial_distance)
{
    if (HasSound(name)) return;
    if (Mix_Chunk* chunk = ResourceManager::GetInstance().LoadSound(filePath)) {
        sounds[name] = { chunk, filePath, -1, volume, loop, play_on_awake, spatial, spatial_distance };
    }
}

void AudioSource::RemoveSound(const std::string& name)
{
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        ResourceManager::GetInstance().ReleaseSound(it->second.filePath);
        sounds.erase(it);
    }
}

void AudioSource::RemoveAllSounds()
{
    for (auto& pair : sounds) {
        ResourceManager::GetInstance().ReleaseSound(pair.second.filePath);
    }
    sounds.clear();
}

bool AudioSource::HasSound(const std::string& name)
{
    return sounds.find(name) != sounds.end();
}

void AudioSource::AddMusic(const std::string& name, const std::string& filePath, int volume, bool loop, bool play_on_awake)
{
    if (HasMusic(name)) return;
    if (Mix_Music* music = ResourceManager::GetInstance().LoadMusic(filePath)) {
        musics[name] = { music, filePath, volume, loop, play_on_awake };
    }
}

void AudioSource::RemoveMusic(const std::string& name)
{
    auto it = musics.find(name);
    if (it != musics.end()) {
        ResourceManager::GetInstance().ReleaseMusic(it->second.filePath);
        musics.erase(it);
    }
}

void AudioSource::RemoveAllMusic()
{
    for (auto& pair : musics) {
        ResourceManager::GetInstance().ReleaseMusic(pair.second.filePath);
    }
    musics.clear();
}

bool AudioSource::HasMusic(const std::string& name)
{
    return musics.find(name) != musics.end();
}

void AudioSource::PlaySound(const std::string& name)
{
    auto it = sounds.find(name);
    if (it != sounds.end())
    {
        if (it->second.channel != -1 &&
            (Mix_Playing(it->second.channel) || Mix_Paused(it->second.channel)))
        {
            return;
        }

        Mix_VolumeChunk(it->second.chunk, it->second.volume * 1.28);
        it->second.channel = Mix_PlayChannel(-1, it->second.chunk, it->second.loop ? -1 : 0);

        if (it->second.channel != -1 && it->second.spatial)
        {
            if (it->second.listener_id != 0 && !it->second.listener.lock().get() && engine->scene_manager_em->CurrentSceneAvailable())
            {
                it->second.listener = engine->scene_manager_em->GetCurrentScene().FindGameObjectByID(it->second.listener_id);
            }
            UpdateSoundPosition(it->second);
        }
    }
}

void AudioSource::PlayMusic(const std::string& name)
{
    auto it = musics.find(name);
    if (it != musics.end())
    {
        if (current_music == it->second.music &&
            (Mix_PlayingMusic() || Mix_PausedMusic()))
        {
            return;
        }

        current_music = it->second.music;
        Mix_VolumeMusic(it->second.volume * 1.28);
        Mix_PlayMusic(it->second.music, it->second.loop ? -1 : 0);
    }
}

void AudioSource::RePlaySound(const std::string& name)
{
    StopSound(name);
    PlaySound(name);
}

void AudioSource::RePlayMusic(const std::string& name)
{
    StopMusic(name);
    PlayMusic(name);
}

void AudioSource::StopAll()
{
    Mix_HaltChannel(-1);
    Mix_HaltMusic();
}

void AudioSource::StopSound(const std::string& name)
{
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        Mix_Chunk* targetChunk = it->second.chunk;
        int numChannels = Mix_AllocateChannels(-1);
        if (Mix_Playing(it->second.channel))
        {
            Mix_HaltChannel(it->second.channel);
        }
        it->second.channel = -1;
    }
}

void AudioSource::StopMusic(const std::string& name)
{
    auto it = musics.find(name);
    if (it != musics.end() && Mix_PlayingMusic() && current_music == it->second.music)
    {
        Mix_HaltMusic();
        current_music = nullptr;
    }
}

void AudioSource::PauseAll()
{
    for (auto& pair : sounds)
    {
        if (pair.second.channel != -1 && Mix_Playing(pair.second.channel))
        {
            Mix_Pause(pair.second.channel);
        }
    }

    if (current_music != nullptr && Mix_PlayingMusic())
    {
        Mix_PauseMusic();
    }
}

void AudioSource::PauseSound(const std::string& name)
{
    auto it = sounds.find(name);
    if (it != sounds.end() && it->second.channel != -1 && Mix_Playing(it->second.channel)) {
        Mix_Pause(it->second.channel);
    }
}

void AudioSource::PauseMusic(const std::string& name)
{
    auto it = musics.find(name);
    if (it != musics.end() && current_music == it->second.music && Mix_PlayingMusic()) {
        Mix_PauseMusic();
    }
}

void AudioSource::UnpauseAll()
{
    for (auto& pair : sounds)
    {
        if (pair.second.channel != -1 && Mix_Paused(pair.second.channel))
        {
            Mix_Resume(pair.second.channel);
        }
    }

    if (current_music != nullptr && Mix_PausedMusic())
    {
        Mix_ResumeMusic();
    }
}

void AudioSource::UnpauseSound(const std::string& name)
{
    auto it = sounds.find(name);
    if (it != sounds.end() && it->second.channel != -1 && Mix_Paused(it->second.channel)) {
        Mix_Resume(it->second.channel);
    }
}

void AudioSource::UnpauseMusic(const std::string& name)
{
    auto it = musics.find(name);
    if (it != musics.end() && current_music == it->second.music && Mix_PausedMusic()) {
        Mix_ResumeMusic();
    }
}

int AudioSource::GetSoundVolume(const std::string& name)
{
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        return it->second.volume;
    }
}

int AudioSource::GetMusicVolume(const std::string& name)
{
    auto it = musics.find(name);
    if (it != musics.end()) {
        return it->second.volume;
    }
}

void AudioSource::SetSoundVolume(const std::string& name, int volume)
{
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        it->second.volume = std::clamp(volume, 0, 100);
        Mix_Chunk* targetChunk = it->second.chunk;
        int targetVolume = it->second.volume * 1.28;

        Mix_VolumeChunk(targetChunk, targetVolume);

        if (Mix_Playing(it->second.channel))
        {
            Mix_Volume(it->second.channel, targetVolume);
        }
    }
}

void AudioSource::SetMusicVolume(const std::string& name, int volume)
{
    auto it = musics.find(name);
    if (it != musics.end()) {
        it->second.volume = std::clamp(volume, 0, 100);
        Mix_VolumeMusic(it->second.volume * 1.28);
    }
}

bool AudioSource::SetListener(const std::string& name, std::shared_ptr<GameObject> listener)
{
    if (!listener.get()) return false;
    auto it = sounds.find(name);
    if (it != sounds.end())
    {
        it->second.listener_id = listener->GetID();
        it->second.listener = listener;
        return true;
    }
    return false;
}

bool AudioSource::RemoveListener(const std::string& name)
{
    auto it = sounds.find(name);
    if (it != sounds.end())
    {
        it->second.listener_id = 0;
        it->second.listener.reset();
        return true;
    }
    return false;
}

nlohmann::json AudioSource::SaveComponent()
{
    nlohmann::json componentJSON;
    componentJSON["ContainerGOID"] = container_go.lock()->GetID();
    componentJSON["ComponentID"] = component_id;
    componentJSON["ComponentName"] = name;
    componentJSON["ComponentType"] = static_cast<int>(type);
    componentJSON["Enabled"] = enabled;

    nlohmann::json soundsJSON;
    for (auto& pair : sounds) {
        nlohmann::json soundJSON;
        soundJSON["name"] = pair.first;
        soundJSON["path"] = pair.second.filePath;
        soundJSON["volume"] = pair.second.volume;
        soundJSON["loop"] = pair.second.loop;
        soundJSON["play_on_awake"] = pair.second.play_on_awake;
        soundJSON["spatial"] = pair.second.spatial;
        soundJSON["spatial_distance"] = pair.second.spatial_distance;
        if (pair.second.listener.lock().get()) soundJSON["listener_id"] = pair.second.listener.lock()->GetID();
        soundsJSON.push_back(soundJSON);
    }
    componentJSON["Sounds"] = soundsJSON;

    nlohmann::json musicsJSON;
    for (auto& pair : musics) {
        nlohmann::json musicJSON;
        musicJSON["name"] = pair.first;
        musicJSON["path"] = pair.second.filePath;
        musicJSON["volume"] = pair.second.volume;
        musicJSON["loop"] = pair.second.loop;
        musicJSON["play_on_awake"] = pair.second.play_on_awake;
        musicsJSON.push_back(musicJSON);
    }
    componentJSON["Musics"] = musicsJSON;

    return componentJSON;
}

void AudioSource::LoadComponent(const nlohmann::json& componentJSON)
{
    RemoveAllSounds();
    RemoveAllMusic();

    if (componentJSON.contains("ComponentID")) component_id = componentJSON["ComponentID"];
    if (componentJSON.contains("ComponentName")) name = componentJSON["ComponentName"];
    if (componentJSON.contains("ComponentType")) type = (ComponentType)componentJSON["ComponentType"];
    if (componentJSON.contains("Enabled")) enabled = componentJSON["Enabled"];

    if (componentJSON.contains("Sounds")) {
        for (auto& soundJSON : componentJSON["Sounds"]) {
            AddSound(soundJSON["name"],
                soundJSON["path"],
                soundJSON["volume"],
                soundJSON["loop"],
                soundJSON["play_on_awake"],
                soundJSON["spatial"],
                soundJSON["spatial_distance"]
            );
            if (soundJSON.contains("listener_id") && engine->scene_manager_em->CurrentSceneAvailable())
            {
                auto it = sounds.find(soundJSON["name"]);
                if (it != sounds.end())
                {
                    it->second.listener_id = soundJSON["listener_id"];
                }
            }
        }
    }

    if (componentJSON.contains("Musics")) {
        for (auto& musicJSON : componentJSON["Musics"]) {
            AddMusic(musicJSON["name"],
                musicJSON["path"],
                musicJSON["volume"],
                musicJSON["loop"],
                musicJSON["play_on_awake"]);
        }
    }
}

void AudioSource::UpdateSoundPosition(SoundRef& sound)
{
    if (sound.channel == -1) return;
    bool cam_is_listener = !sound.listener.lock().get();
    Transform* listener = nullptr;
    if (!cam_is_listener) listener = sound.listener.lock()->GetComponent<Transform>();
    if (!listener && !cam_is_listener)
    {
        sound.listener.reset();
        cam_is_listener = true;
    }
    auto go = container_go.lock();
    if (!go) return;
    if (!engine->scene_manager_em->CurrentSceneAvailable()) return;
    if (!engine->scene_manager_em->GetCurrentScene().HasCameraSet() && cam_is_listener)
    {
        Mix_SetPosition(sound.channel, 0, 0);
        return;
    }

    auto t = container_go.lock()->GetComponent<Transform>();
    if (!t) return;
    float obj_x = t->GetPosition().x;
    float obj_y = t->GetPosition().y;

    if (cam_is_listener) listener = engine->scene_manager_em->GetCurrentScene().GetCurrentCameraGO().GetComponent<Transform>();
    if (!listener) return;
    float cam_x = listener->GetPosition().x;
    float cam_y = listener->GetPosition().y;

    float dx = obj_x - cam_x;
    float dy = obj_y - cam_y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance > sound.spatial_distance)
    {
        Mix_SetPosition(sound.channel, 0, 255);
        return;
    }

    float angle_degrees = (dx / sound.spatial_distance) * 90.0f;
    if (angle_degrees < 0) angle_degrees += 360.0f;

    Uint8 sdl_distance = static_cast<Uint8>((distance / sound.spatial_distance) * 255);
    Mix_SetPosition(sound.channel, static_cast<Sint16>(angle_degrees), sdl_distance);
}
