#pragma once
#include <chrono>
#include "defines.h"

namespace GR
{

	struct Timer
	{
		std::chrono::steady_clock::time_point startTime;
	};

	extern Timer g_timer;
	extern f64 g_deltaTime;

	Timer CreateAndStartTimer();

	void TimerReset(Timer* timer);

	f64 TimerSecondsSinceStart(Timer timer);
	f64 TimerMilisecondsSinceStart(Timer timer);
}