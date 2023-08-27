#include "timer.h"

namespace GR
{

	Timer CreateAndStartTimer()
	{
		Timer timer{};
		timer.startTime = std::chrono::high_resolution_clock::now();
		return timer;
	}

	void ResetTimer(Timer* timer)
	{
		timer->startTime = std::chrono::high_resolution_clock::now();
	}

	f64 SecondsSinceStart(Timer timer)
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - timer.startTime).count() / (f64)1000000;
	}

	f64 MilisecondsSinceStart(Timer timer)
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - timer.startTime).count() / (f64)1000;
	}
}