#include "timer.h"

namespace Goril
{

	Timer::Timer()
	{
		m_startTime = std::chrono::high_resolution_clock::now();
	}

	float Timer::SecondsSinceStart()
	{
		return (float) std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_startTime).count() / 1000000;
	}

	float Timer::MilisecondsSinceStart()
	{
		return (float)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_startTime).count() / 1000;
	}
}