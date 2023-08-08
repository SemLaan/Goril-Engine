#pragma once
#include <chrono>
#include "defines.h"

namespace GR
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