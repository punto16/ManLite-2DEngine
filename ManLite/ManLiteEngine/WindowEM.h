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
	void GetWindowPos(int& x, int& y) const;
	SDL_Window* GetSDLWindow() const { return this->window; }
	SDL_GLContext GetGLContext() const { return this->gl_context; }
	bool GetVsync() { return vsync; }
	void SetVsync(bool vsync);
	bool GetFullScreen() { return fs; }
	void SetFullScreen(bool fs);

private:
	SDL_Window* window;

	SDL_GLContext gl_context;

	bool vsync = false;
	bool fs = false;
};

#endif // !__WINDOW_EM_H__