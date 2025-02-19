#include "WindowEM.h"

#include "EngineCore.h"

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

	delete window;
	window = nullptr;

	return ret;
}
