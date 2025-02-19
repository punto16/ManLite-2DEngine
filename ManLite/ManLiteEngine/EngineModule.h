#ifndef __ENGINE_MODULE_H__
#define __ENGINE_MODULE_H__
#pragma once

class EngineCore;

class EngineModule
{
public:
	EngineModule() {}
	EngineModule(EngineCore* parent) : active(false), engine(parent)
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
	EngineCore* engine;
};

#endif // !__ENGINE_MODULE_H__