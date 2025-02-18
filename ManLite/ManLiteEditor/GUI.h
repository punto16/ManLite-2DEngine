#ifndef __GUI_H__
#define __GUI_H__
#pragma once

#include "Module.h"

enum class Aspect
{
	A_FREE,
	A_16x9,
	A_21x9
};

struct EditColor
{
	std::string id;
	ImVec4 color;
	ImVec4 previousColor;
};

class Gui : public Module
{
public:
	Gui(App* app);
	~Gui();

	bool Awake();
	bool Start();
	bool PreUpdate();
	bool Update(double dt);
	bool PostUpdate();
	bool CleanUp();

	//bool IsInitialized(Panel* panel);
	//std::list<Panel*> GetPanels();

	void HandleInput();
	void ProcessEvent();


private:

};

#endif // !__GUI_H__