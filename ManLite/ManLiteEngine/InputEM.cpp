#include "InputEM.h"

#include <string.h>
#include <SDL2/SDL.h>

InputEM::InputEM(EngineCore* parent) : EngineModule(parent)
{
	keyboard = new KeyState[MAX_KEYS];
	memset(keyboard, KEY_IDLE, sizeof(KeyState) * MAX_KEYS);
	memset(mouse_buttons, KEY_IDLE, sizeof(KeyState) * MAX_MOUSE_BUTTONS);
}

InputEM::~InputEM()
{
	delete[] keyboard;
}

bool InputEM::Awake()
{
	bool ret = true;

	SDL_Init(0);
	if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0) return false;

	return ret;
}

bool InputEM::Start()
{
	bool ret = true;
	SDL_StopTextInput();
	return ret;
}

bool InputEM::PreUpdate()
{
	bool ret = true;

	static SDL_Event event;
	const Uint8* keys = SDL_GetKeyboardState(NULL);

	for (size_t i = 0; i < MAX_KEYS; i++)
	{
		if (keys[i] == 1)
		{
			if (keyboard[i] == KEY_IDLE) keyboard[i] = KEY_DOWN;
			else keyboard[i] = KEY_REPEAT;
		}
		else
		{
			if (keyboard[i] == KEY_REPEAT || keyboard[i] == KEY_DOWN) keyboard[i] = KEY_UP;
			else keyboard[i] = KEY_IDLE;
		}
	}

	for (int i = 0; i < MAX_MOUSE_BUTTONS; ++i)
	{
		if (mouse_buttons[i] == KEY_DOWN) mouse_buttons[i] = KEY_REPEAT;
		if (mouse_buttons[i] == KEY_UP) mouse_buttons[i] = KEY_IDLE;
	}

	while (SDL_PollEvent(&event) != SDL_FIRSTEVENT)
	{
		switch (event.type)
		{
		case SDL_QUIT:
		{
			return false;
			break;
		}
		case SDL_WINDOWEVENT:
		{
			switch (event.window.event)
			{
				//case SDL_WINDOWEVENT_LEAVE:
			case SDL_WINDOWEVENT_HIDDEN:
			case SDL_WINDOWEVENT_MINIMIZED:
			case SDL_WINDOWEVENT_FOCUS_LOST:
				windowEvents[WindowEvent_Hide] = true;
				break;

				//case SDL_WINDOWEVENT_ENTER:
			case SDL_WINDOWEVENT_SHOWN:
			case SDL_WINDOWEVENT_FOCUS_GAINED:
			case SDL_WINDOWEVENT_MAXIMIZED:
			case SDL_WINDOWEVENT_RESTORED:
				windowEvents[WindowEvent_Show] = true;
				break;
				break;
			}
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			mouse_buttons[event.button.button - 1] = KEY_DOWN;
			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			mouse_buttons[event.button.button - 1] = KEY_UP;
			break;
		}
		case SDL_MOUSEMOTION:
		{
			this->mouseX = event.motion.x;
			this->mouseY = event.motion.y;
			this->mouse_motionX = event.motion.xrel;
			this->mouse_motionY = event.motion.yrel;
			break;
		}
		default:
			break;
		}
	}

	return ret;
}

bool InputEM::CleanUp()
{
	bool ret = true;

	SDL_QuitSubSystem(SDL_INIT_EVENTS);

	return ret;
}
