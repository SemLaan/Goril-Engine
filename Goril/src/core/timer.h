#pragma once
#include <chrono>
#include "defines.h"



typedef struct Timer
{
	std::chrono::steady_clock::time_point startTime;
} Timer;

extern Timer g_timer;
extern f64 g_deltaTime;

Timer GRAPI CreateAndStartTimer();

void GRAPI TimerReset(Timer* timer);

f64 GRAPI TimerSecondsSinceStart(Timer timer);
f64 GRAPI TimerMilisecondsSinceStart(Timer timer);
