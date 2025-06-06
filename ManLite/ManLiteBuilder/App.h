#pragma once
#include "Module.h"
#include "Timer.h"

#include <string>
#include <list>
#include <chrono>
#include <thread>

class App
{
public:
	App(int argc, char* args[]);
	virtual ~App();

	bool Awake();
	bool Start();
	bool Update();
	bool CleanUp();

	int GetArgc() const;
	const char* GetArgv(int index) const;

	void LoadInit();

	int GetFrameRate() const;
	void SetFrameRate(int frameRate);
	double GetDT() const;
	void SetDT(double dt);

private:
	void PrepareUpdate();
	bool PreUpdate();
	bool DoUpdate();
	bool PostUpdate();
	void FinishUpdate();

public:
	//data
	std::string title;
	std::string organization;

private:

	int argc;
	char** args;


	std::string first_scene = "";
	std::string game_name = "My Test Game";
	std::string icon_path = "";

	//fps control
	std::chrono::duration<double> targetFrameDuration;
	std::chrono::steady_clock::time_point frameStart, frameEnd;

	Timer* start_timer;
	float time_since_start;

	int frameRate = 5000;
	double dt = 0;
	double dtCount = 0;
	int frameCount = 0;
	int fps = 0;
};

extern App* app;