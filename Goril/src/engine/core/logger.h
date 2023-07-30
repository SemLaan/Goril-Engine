#pragma once
#include "gorilmem.h"
#include <spdlog/spdlog.h>

namespace Goril
{

	class Logger
	{
	public:
		void Init();
		void Shutdown();

	private:
		Ref<spdlog::logger> m_logger;

		// Singleton code
	public:
		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;
		Logger& operator=(Logger&&) = delete;
		Logger(Logger&&) = delete;

		static Logger*& Get();

	private:
		Logger() = default;
	};
}