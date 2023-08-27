#include "timer.h"

namespace GR
{

	Timer g_timer = {};
	f64 g_previousFrameTime = {};
	f64 g_deltaTime = {};

	Timer CreateAndStartTimer()
	{
		Timer timer{};
		timer.startTime = std::chrono::high_resolution_clock::now();
		return timer;
	}

	void TimerReset(Timer* timer)
	{
		timer->startTime = std::chrono::high_resolution_clock::now();
	}

	f64 TimerSecondsSinceStart(Timer timer)
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - timer.startTime).count() / (f64)1000000;
	}

	f64 TimerMilisecondsSinceStart(Timer timer)
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - timer.startTime).count() / (f64)1000;
	}
}