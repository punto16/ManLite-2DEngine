#include "AudioSource.h"
#include "GameObject.h"
#include "Log.h"

#include "SDL2/SDL_mixer.h"

AudioSource::AudioSource(std::weak_ptr<GameObject> container_go, std::string name, bool enable)
    : Component(container_go, ComponentType::AudioSource, name, enable)
{
}

AudioSource::AudioSource(const AudioSource& component_to_copy, std::shared_ptr<GameObject> container_go)
    : Component(component_to_copy, container_go)
{
    for (auto& pair : component_to_copy.sounds)
        AddSound(pair.first, pair.second.filePath, pair.second.volume, pair.second.loop, pair.second.play_on_awake);

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
    return true;
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

void AudioSource::AddSound(const std::string& name, const std::string& filePath, int volume, bool loop, bool play_on_awake)
{
    if (HasSound(name)) return;
    if (Mix_Chunk* chunk = ResourceManager::GetInstance().LoadSound(filePath)) {
        sounds[name] = { chunk, filePath, volume, loop, play_on_awake };
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
    if (it != sounds.end()) {
        Mix_VolumeChunk(it->second.chunk, it->second.volume * 1.28);
        Mix_PlayChannel(-1, it->second.chunk, it->second.loop ? -1 : 0);
    }
}

void AudioSource::PlayMusic(const std::string& name)
{
    auto it = musics.find(name);
    if (it != musics.end()) {
        current_music = it->second.music;
        Mix_VolumeMusic(it->second.volume * 1.28);
        Mix_PlayMusic(it->second.music, it->second.loop ? -1 : 0);
    }
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
        for (int i = 0; i < numChannels; ++i) {
            if (Mix_GetChunk(i) == targetChunk && Mix_Playing(i)) {
                Mix_HaltChannel(i);
            }
        }
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
    Mix_Pause(-1);
    Mix_PauseMusic();
}

void AudioSource::PauseSound(const std::string& name)
{
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        Mix_Chunk* targetChunk = it->second.chunk;
        int numChannels = Mix_AllocateChannels(-1);
        for (int i = 0; i < numChannels; ++i) {
            if (Mix_GetChunk(i) == targetChunk && Mix_Playing(i)) {
                Mix_Pause(i);
            }
        }
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
    Mix_Resume(-1);
    Mix_ResumeMusic();
}

void AudioSource::UnpauseSound(const std::string& name)
{
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        Mix_Chunk* targetChunk = it->second.chunk;
        int numChannels = Mix_AllocateChannels(-1);
        for (int i = 0; i < numChannels; ++i) {
            if (Mix_GetChunk(i) == targetChunk && Mix_Paused(i)) {
                Mix_Resume(i);
            }
        }
    }
}

void AudioSource::UnpauseMusic(const std::string& name)
{
    auto it = musics.find(name);
    if (it != musics.end() && current_music == it->second.music && Mix_PausedMusic()) {
        Mix_ResumeMusic();
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

        int numChannels = Mix_AllocateChannels(-1);
        for (int i = 0; i < numChannels; ++i) {
            if (Mix_GetChunk(i) == targetChunk && Mix_Playing(i)) {
                Mix_Volume(i, targetVolume);
            }
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
                soundJSON["play_on_awake"]);
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