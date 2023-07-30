#include "logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Goril
{

	void Logger::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		m_logger = spdlog::stdout_color_mt("Goril");
		m_logger->set_level(spdlog::level::trace);
	}

	void Logger::Shutdown()
	{

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