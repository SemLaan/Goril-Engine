#include "logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Goril
{
	Ref<spdlog::logger> Logger::s_logger = nullptr;

	void Logger::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_logger = spdlog::stdout_color_mt("GORIL");
		s_logger->set_level(spdlog::level::trace);
		GRINFO("Logger initialized");
	}

	void Logger::Shutdown()
	{
		GRINFO("Logger shutdown");
	}

	Logger*& Logger::Get()
	{
		static Logger* instance = nullptr;
		if (!instance)
		{
			instance = new Logger();
		}
		return instance;
	}
}