#include "WindowEM.h"

#include "EngineCore.h"
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

	SDL_WindowFlags windowFlags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

	window = SDL_CreateWindow(
		"ManLite 2D Engine",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width,
		height,
		windowFlags
	);

	if (!window)
	{
		SDL_Quit();
		return false;
	}
	else
	{
		screenSurface = SDL_GetWindowSurface(window);
	}

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
	w = this->width;
	h = this->height;
}
