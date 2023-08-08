#include "timer.h"

namespace GR
{

	Timer::Timer()
	{
		m_startTime = std::chrono::high_resolution_clock::now();
	}

	f64 Timer::SecondsSinceStart()
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_startTime).count() / (f64)1000000;
	}

	f64 Timer::MilisecondsSinceStart()
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_startTime).count() / (f64)1000;
	}
}