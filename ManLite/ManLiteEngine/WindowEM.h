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
	SDL_GLContext GetGLContext() const { return this->gl_context; }

private:
	SDL_Window* window;

	SDL_GLContext gl_context;

	unsigned int width, height;
};

#endif // !__WINDOW_EM_H__