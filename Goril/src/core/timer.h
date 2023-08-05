#pragma once
#include <chrono>
#include "defines.h"

namespace gr
{

	class Timer
	{
	public:
		GRAPI Timer();

		GRAPI f64 SecondsSinceStart();
		GRAPI f64 MilisecondsSinceStart();
	private:
		std::chrono::steady_clock::time_point m_startTime;
	};
}