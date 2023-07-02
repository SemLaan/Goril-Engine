#pragma once
#include <chrono>

namespace Goril
{

	class Timer
	{
	public:
		Timer();

		float SecondsSinceStart();
		float MilisecondsSinceStart();
	private:
		std::chrono::steady_clock::time_point m_startTime;
	};
}