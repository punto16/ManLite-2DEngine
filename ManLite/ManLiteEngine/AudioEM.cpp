#include "AudioEM.h"
#include "EngineCore.h"
#include "Defs.h"
#include "ResourceManager.h"

AudioEM::AudioEM(EngineCore* parent) : EngineModule(parent)
{
}

AudioEM::~AudioEM()
{
}

bool AudioEM::Awake()
{
    int mixerFlags = MIX_INIT_OGG/* | MIX_INIT_MP3*/;
    if (Mix_Init(mixerFlags) != mixerFlags) {
        LOG(LogType::LOG_ERROR, "SDL_mixer initialization error: %s", Mix_GetError());
        return false;
    }

    if (Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, channels, chunkSize) < 0) {
        LOG(LogType::LOG_ERROR, "Mix_OpenAudio error: %s", Mix_GetError());
        return false;
    }

    Mix_Volume(-1, 64);
    Mix_VolumeMusic(64);

    LOG(LogType::LOG_OK, "AudioEM: Initialization Success");
    return true;
}

bool AudioEM::CleanUp()
{
    Mix_CloseAudio();
    Mix_Quit();

    LOG(LogType::LOG_OK, "AudioEM closed correctly");
    return true;
}