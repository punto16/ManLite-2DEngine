#ifndef __INPUT_EM_H__
#define __INPUT_EM_H__
#pragma once

#include "EngineModule.h"

#include <vector>
#include <SDL2/SDL.h>

#define MAX_KEYS 300
#define MAX_MOUSE_BUTTONS 5

enum EventWindow
{
	WindowEvent_Quit = 0,
	WindowEvent_Hide = 1,
	WindowEvent_Show = 2,
	WindowEvent_Count = 3,
};

enum KeyState
{
	KEY_IDLE = 0,
	KEY_DOWN = 1,
	KEY_REPEAT = 2,
	KEY_UP = 3,
};

class InputEM : public EngineModule
{
public:
	InputEM(EngineCore* parent);
	~InputEM();

	bool Awake();
	bool Start();
	bool PreUpdate();
	bool CleanUp();


	KeyState GetKey(int key) const { return keyboard[key]; }
	KeyState GetMouseButtonDown(int id) const { return mouse_buttons[id - 1]; }
	bool GetWindowEvent(EventWindow ev) { return windowEvents[ev]; }
	void GetMousePosition(int& x, int& y) { x = this->mouseX; y = this->mouseY; }
	void GetMouseMotion(int& x, int& y) { x = this->mouse_motionX; y = this->mouse_motionY; }
	//-1 scroll down | 0 nothing | 1 scroll up
	int GetMouseWheelMotion() const { return mouse_wheelY; }
	void CloseApp();

	std::vector<SDL_Event> GetSDLEvents() { return events; }

private:

	bool close_app;

	bool windowEvents[WindowEvent_Count];
	KeyState* keyboard;
	KeyState mouse_buttons[5];
	int mouseX, mouseY, mouse_motionX, mouse_motionY, mouse_wheelY;

	std::vector<SDL_Event> events;

	//
	float window_ev_fix_time = 5.0f;
	float window_ev_fix_timer = 0.0f;
};

#endif // __INPUT_EM_H__