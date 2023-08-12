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
		// TODO: create file to write logs to

		// Testing logging functions
		GRFATAL("Testing log functions: {}", "succesfull");
		GRERROR("Testing log functions: {}", "succesfull");
		GRWARN("Testing log functions: {}", "succesfull");
		GRINFO("Testing log functions: {}", "succesfull");
		GRDEBUG("Testing log functions: {}", "succesfull");
		GRTRACE("Testing log functions: {}", "succesfull");

		return true;
	}

	void ShutdownLogger()
	{
		// TODO: write logs to file
	}

	void Log(log_level level, std::string message)
	{
		PlatformLogMessage(level, (logLevels[level] + message + "\n").c_str());
	}
}