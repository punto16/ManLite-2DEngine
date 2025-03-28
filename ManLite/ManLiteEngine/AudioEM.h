#ifndef __AUDIO_EM_H__
#define __AUDIO_EM_H__
#pragma once

#include "EngineModule.h"
#include <SDL2/SDL_mixer.h>

class AudioEM : public EngineModule
{
public:
    AudioEM(EngineCore* parent);
    virtual ~AudioEM();

    bool Awake() override;
    bool CleanUp() override;

private:
    // Configuración básica de audio
    int frequency = 44100;      // Frecuencia estándar
    int channels = 2;           // Estéreo
    int chunkSize = 2048;       // Tamaño del buffer
};

#endif // !__AUDIO_EM_H__