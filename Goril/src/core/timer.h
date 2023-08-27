#pragma once
#include <chrono>
#include "defines.h"

namespace GR
{

	struct Timer
	{
		std::chrono::steady_clock::time_point startTime;
	};

	Timer CreateAndStartTimer();

	void ResetTimer(Timer* timer);

	f64 SecondsSinceStart(Timer timer);
	f64 MilisecondsSinceStart(Timer timer);
}