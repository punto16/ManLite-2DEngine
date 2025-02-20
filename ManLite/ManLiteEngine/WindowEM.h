#ifndef __WINDOW_EM_H__
#define __WINDOW_EM_H__
#pragma once

#include "EngineModule.h"
#include <SDL2/SDL.h>
#include <string>

class WindowEM : public EngineModule
{
public:
	WindowEM(EngineCore* parent);
	virtual ~WindowEM();

	bool Awake();
	bool Start();

	bool CleanUp();


	void SetTitle(std::string title);
	void SetIcon(std::string path);
	void GetWindowSize(unsigned int& w, unsigned int& h) const;
	SDL_Window* GetSDLWindow() const { return this->window; }
	SDL_Surface* GetSDLSurface() const { return this->screenSurface; }


private:
	SDL_Window* window;
	SDL_Surface* screenSurface;

	unsigned int width = 1700, height = 900;
};

#endif // !__EWINDOW_EM_H__