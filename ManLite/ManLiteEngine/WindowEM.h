#ifndef __WINDOW_EM_H__
#define __WINDOW_EM_H__
#pragma once

#include "EngineModule.h"
#include <SDL2/SDL.h>

class WindowEM : public EngineModule
{
public:
	WindowEM(EngineCore* parent);
	virtual ~WindowEM();

	bool Awake();
	bool Start();

	bool CleanUp();

private:
	SDL_Window* window;


	unsigned int width = 800, height = 600;
};

#endif // !__EWINDOW_EM_H__