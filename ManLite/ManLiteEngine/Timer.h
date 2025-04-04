#pragma once
#ifndef __TIMER_H__
#define __TIMER_H__

#include "Defs.h"

class Timer
{
public:

	// Constructor
	Timer();

	void Start();
	uint Read() const;
	float ReadSec() const;
	float ReadSec(float scale);
	void Pause();
	void Resume();

	static void SecondsToFormat(float totalSeconds, int& h, int& m, int& s, int& ms);

private:
	uint started_at;
	uint time_pause;
	uint offset;
	uint time_passed;
	float curr_time;
	float time_last_update;
	bool is_pause;
};

#endif //__TIMER_H__
