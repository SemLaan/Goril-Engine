#include "logger.h"

#include "platform.h"

namespace GR
{

	static std::string logLevels[MAX_LOG_LEVELS] =
	{
		"[FATAL]: ",
		"[ERROR]: ",
		"[WARN]:  ",
		"[INFO]:  ",
		"[DEBUG]: ",
		"[TRACE]: ",
	};

	b8 InitializeLogger()
	{
		GRINFO("Initializing logging subsystem...");
		// TODO: create file to write logs to

		return true;
	}

	void ShutdownLogger()
	{
		GRINFO("Shutting down logging subsystem...");
		// TODO: write logs to file
	}

	void Log(log_level level, std::string message)
	{
		PlatformLogString(level, (logLevels[level] + message + "\n").c_str());
	}
}