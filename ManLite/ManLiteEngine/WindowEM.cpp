#include "WindowEM.h"

#include "EngineCore.h"
#include "Defs.h"

#include <SDL2/SDL_image.h>

WindowEM::WindowEM(EngineCore* parent) : EngineModule(parent)
{

}

WindowEM::~WindowEM()
{
}

bool WindowEM::Awake()
{
	bool ret = true;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_WindowFlags windowFlags = (SDL_WindowFlags)(
		SDL_WINDOW_OPENGL |
		SDL_WINDOW_ALLOW_HIGHDPI
		);

	if (engine->GetEditorOrBuild())
	{
		windowFlags = (SDL_WindowFlags)(windowFlags | SDL_WINDOW_RESIZABLE);
	}

	if (fs)
	{
		if (SDL_SetWindowFullscreen(window, fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0)
		{
			return false;
		}
	}

	window = SDL_CreateWindow(
		"ManLite 2D Engine",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		DEFAULT_CAM_WIDTH,
		DEFAULT_CAM_HEIGHT,
		windowFlags
	);

	if (!window)
	{
		LOG(LogType::LOG_ERROR, "WindowEM: Initialize Window failure");
		SDL_Quit();
		return false;
	}
	else
	{
		gl_context = SDL_GL_CreateContext(window);
		SDL_GL_MakeCurrent(window, gl_context);
		SDL_GL_SetSwapInterval(vsync);
	}

	LOG(LogType::LOG_OK, "WindowEM: Initialization Success");

	return ret;
}

bool WindowEM::Start()
{
	bool ret = true;

	return ret;
}

bool WindowEM::CleanUp()
{
	bool ret = true;

	SDL_DestroyWindow(window);
	SDL_Quit();

	window = nullptr;

	return ret;
}

void WindowEM::SetTitle(std::string title)
{
	SDL_SetWindowTitle(window, title.c_str());
}

void WindowEM::SetIcon(std::string path)
{
	SDL_SetWindowIcon(window, IMG_Load(path.c_str()));
}

void WindowEM::GetWindowSize(unsigned int& w, unsigned int& h) const
{
	int w_ = 0, h_ = 0;
	SDL_GetWindowSize(window, &w_, &h_);
	w = (unsigned int)w_;
	h = (unsigned int)h_;
}

void WindowEM::GetWindowPos(int& x, int& y) const
{
	SDL_GetWindowPosition(window, &x, &y);
}

void WindowEM::SetVsync(bool vsync)
{
	this->vsync = vsync;
	SDL_GL_SetSwapInterval(this->vsync ? 1 : 0);
}

void WindowEM::SetFullScreen(bool fs)
{
	if (SDL_SetWindowFullscreen(window, fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0)
	{
		return;
	}

	this->fs = fs;
}
