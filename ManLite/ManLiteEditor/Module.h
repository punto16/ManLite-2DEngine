#ifndef __MODULE_H__
#define __MODULE_H__
#pragma once

class App;

class Module
{
public:
	Module() {}
	Module(App* parent) : active(false), app(parent)
	{}

	void Init()
	{
		active = true;
	}

	virtual bool Awake() { return true; }

	virtual bool Start() { return true; }

	virtual bool PreUpdate() { return true; }

	virtual bool Update(double dt) { return true; }

	virtual bool PostUpdate() { return true; }

	virtual bool CleanUp() { return true; }


	void Enable() {
		if (!active)
		{
			active = true;
			Start();
		}
	}

	void Disable() {
		if (active)
		{
			active = false;
			CleanUp();
		}
	}

public:
	bool active;
	App* app;
};

#endif // !__MODULE_H__