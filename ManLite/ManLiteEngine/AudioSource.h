#ifndef __AUDIO_SOURCE_H__
#define __AUDIO_SOURCE_H__

#include "Component.h"
#include "ResourceManager.h"
#include <SDL2/SDL_mixer.h>
#include <unordered_map>

struct SoundRef {
    Mix_Chunk* chunk;
    std::string filePath;
    int volume = 100;
    bool loop = false;
    bool play_on_awake = false;
};

struct MusicRef {
    Mix_Music* music;
    std::string filePath;
    int volume = 100;
    bool loop = false;
    bool play_on_awake = false;
};

class AudioSource : public Component {
public:
    AudioSource(std::weak_ptr<GameObject> container_go, std::string name = "AudioSource", bool enable = true);
    AudioSource(const AudioSource& component_to_copy, std::shared_ptr<GameObject> container_go);
    ~AudioSource();

    bool Update(float dt) override;

    // sound
    void AddSound(const std::string& name, const std::string& filePath, int volume = 100, bool loop = false, bool play_on_awake = false);
    void RemoveSound(const std::string& name);
    void RemoveAllSounds();
    bool HasSound(const std::string& name);

    // music
    void AddMusic(const std::string& name, const std::string& filePath, int volume = 100, bool loop = false, bool play_on_awake = false);
    void RemoveMusic(const std::string& name);
    void RemoveAllMusic();
    bool HasMusic(const std::string& name);

    // general
    void PlaySound(const std::string& name);
    void PlayMusic(const std::string& name);
    void StopAll();
    void StopSound(const std::string& name);
    void StopMusic(const std::string& name);
    void SetSoundVolume(const std::string& name, int volume);
    void SetMusicVolume(const std::string& name, int volume);

    // serialization
    nlohmann::json SaveComponent() override;
    void LoadComponent(const nlohmann::json& componentJSON) override;

    // Getters
    std::unordered_map<std::string, SoundRef>& GetSounds() { return sounds; }
    std::unordered_map<std::string, MusicRef>& GetMusics() { return musics; }

private:
    std::unordered_map<std::string, SoundRef> sounds;
    std::unordered_map<std::string, MusicRef> musics;

    Mix_Music* current_music = nullptr;

};
#endif // !__AUDIO_SOURCE_H__